#!/usr/bin/python3
#
# Copyright (c) 2023 LunarG, Inc.
# Copyright (c) 2023 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author: Shahbaz Youssefi <syoussefi@google.com>

"""Used to merge multiple codified VUs in one.

The merger attempts to:

- Refactor constant variable decalarations
- Remap variable names to avoid collision
- Merge the bodies of identical for loops, as long as there are no breaks or
  continues
- Merge the bodies of identical if conditions, as long as there are no breaks
  or continues

The `require(condition)` calls are changed to `require(condition, vu_index, require_index)`, with
`vu_index` an integer indexing the list of original VU ASTs and `require_index` identifying which
`require()` inside the VU this refers to.  This `index` is later used during translation to generate
an error message for the failing VU based on its original specification at the point of the failing
`require()`.
"""

import ast
from collections import namedtuple
import copy

HAS_BRANCH = True
NO_BRANCH = False

IS_CONSTANT = True
IS_VARIABLE = False

IS_ELSE_BLOCK = True

class VuAnalyzer(ast.NodeVisitor):
    """Analyze the VU:

    - Find constant variables per block
    - Find for and if blocks that contain branches
    - Associate require() nodes with the VUID and VU AST
    - Tag each variable with a unique symbol id
    """
    def __init__(self, vuAST, nextUniqueId, vuRequireIndex):
        self.vuAST = vuAST

        self.nextUniqueId = nextUniqueId
        """A unique id for symbols.  Used later to identify symbols without having to use variable
        names and scoping rules."""

        self.vuRequireIndex = vuRequireIndex
        """The index of this VU in the list of VUs being processed.  require() calls are tagged with
        this index.  Later during codegen, the index is used to look up the original VU's AST for
        error message generation."""

        self.constantVariableSet = {}
        """Map from block node (for, if-true, if-else, etc) to list of constant variables"""

        self.branchlessBlockMap = set()
        """Set of for and if nodes that are known to not have branches"""

        self.scopedVariables = []
        """Dictionary of variables in each scope, mapping to their unique id and whether they are
        constant."""

        self.nextRequireIndex = 0
        """The index of the `require()` call inside the VU.  Used to identify the `require()` call
        inside the merged AST when the VU contains multiple `require()`."""

    def analyze(self):
        self.visit(self.vuAST)
        assert(len(self.scopedVariables) == 0)

    def getTaggedVu(self):
        """Return the VU with variable ids tagged in.  Used to generate the VU failure message
        later.  The indices added to require are stripped, as they are not needed."""
        originalVu = copy.deepcopy(self.vuAST)

        for node in ast.walk(originalVu):
            if (isinstance(node, ast.Call) and
                isinstance(node.func, ast.Name) and
                node.func.id == 'require'):
                node.args = node.args[:1]

        return originalVu

    def beginScope(self):
        self.scopedVariables.append({})

    def endScope(self, node, isElse):
        # Add any variables that are still constant to constantVariableSet
        constantVars = [(variable, str(varId)) for variable, (varId, isConstant) in
                        self.scopedVariables[-1].items() if isConstant]
        self.constantVariableSet[(node, isElse)] = constantVars
        self.scopedVariables.pop()

    def visitBody(self, node, statements, loopTarget = None, isElse = not IS_ELSE_BLOCK):
        self.beginScope()

        # Define the loop target inside the scope
        if loopTarget is not None:
            newId = self.nextUniqueId
            self.nextUniqueId += 1
            self.scopedVariables[-1][loopTarget.id] = newId, IS_VARIABLE

            self.visit(loopTarget)

        hasBranch = False
        for statement in statements:
            hasBranch = self.visit(statement) or hasBranch

        self.endScope(node, isElse)

        if not hasBranch:
            self.branchlessBlockMap.add(node)
        return hasBranch

    def visit_Module(self, node):
        self.visitBody(node, node.body)

    def visit_Expr(self, node):
        self.visit(node.value)
        return NO_BRANCH

    def visit_Assign(self, node):
        lhs = node.targets[0].id

        # Check to see if this is a new variable.
        isNew = True
        for scopeIndex in reversed(range(len(self.scopedVariables))):
            if lhs in self.scopedVariables[scopeIndex]:
                # If not, mark it as not constant
                isNew = False
                varId, _ = self.scopedVariables[scopeIndex][lhs]
                self.scopedVariables[scopeIndex][lhs] = varId, IS_VARIABLE
                break

        # Always assume new variables are constant
        if isNew:
            newId = self.nextUniqueId
            self.nextUniqueId += 1
            self.scopedVariables[-1][lhs] = newId, IS_CONSTANT

        self.visit(node.targets[0])
        self.visit(node.value)

        return NO_BRANCH

    def visit_If(self, node):
        self.visit(node.test)
        hasBranch = self.visitBody(node, node.body)
        hasBranch = self.visitBody(node, node.orelse, isElse=IS_ELSE_BLOCK) or hasBranch
        # If the code inside the if block has a branch, it will affect the outer loop, so mark the
        # outer block as also having a branch
        return hasBranch

    def visit_For(self, node):
        self.visit(node.iter)
        hasBranch = self.visitBody(node, node.body, loopTarget=node.target)

        # Any branches inside the for loop only affect itself; the outer block can still be
        # considered branchless.
        return NO_BRANCH

    def visit_While(self, node):
        # Identical logic to for loops
        self.visit(node.test)
        hasBranch = self.visitBody(node, node.body)
        return NO_BRANCH

    def visit_Break(self, node):
        return HAS_BRANCH

    def visit_Continue(self, node):
        return HAS_BRANCH

    def visit_BoolOp(self, node):
        for value in node.values:
            self.visit(value)
        return NO_BRANCH

    def visit_UnaryOp(self, node):
        self.visit(node.operand)
        return NO_BRANCH

    def visit_BinOp(self, node):
        self.visit(node.left)
        self.visit(node.right)
        return NO_BRANCH

    def visit_Compare(self, node):
        self.visit(node.left)
        self.visit(node.comparators[0])
        return NO_BRANCH

    def visit_Call(self, node):
        self.visit(node.func)
        for arg in node.args:
            self.visit(arg)
        # Add the VU index to the list of require args.
        if isinstance(node.func, ast.Name) and node.func.id == 'require':
            node.args.append(ast.Constant(self.vuRequireIndex))
            node.args.append(ast.Constant(self.nextRequireIndex))
            self.nextRequireIndex += 1
        return NO_BRANCH

    def visit_Attribute(self, node):
        self.visit(node.value)
        return NO_BRANCH

    def visit_Subscript(self, node):
        self.visit(node.value)
        self.visit(node.slice)
        return NO_BRANCH

    def visit_IfExp(self, node):
        self.visit(node.body)
        self.visit(node.test)
        self.visit(node.orelse)
        return NO_BRANCH

    def visit_Constant(self, node):
        return NO_BRANCH

    def visit_Name(self, node):
        # If this is a variable, embed its symbol id in the AST
        for scopeIndex in reversed(range(len(self.scopedVariables))):
            if node.id in self.scopedVariables[scopeIndex]:
                varId, _ = self.scopedVariables[scopeIndex][node.id]
                node.id = node.id + '|' + str(varId)
                break

        return NO_BRANCH


MergeInfo = namedtuple('MergeInfo', ['scope', 'body', 'constantVariableSet',
                                     'branchlessBlockMap'])


def renameVariables(expression, variableIdMap):
    for node in ast.walk(expression):
        if isinstance(node, ast.Name) and '|' in node.id:
            _, varId = node.id.split('|')
            if varId in variableIdMap:
                node.id = variableIdMap[varId]

    return expression

def duplicateStatement(statement, variableIdMap, constantVariableSet,
                       constantVariableExpressionMap):
    # First, check if this is an assignment.  If so, we need to determine if we should keep this
    # variable name or remap it.
    if isinstance(statement, ast.Assign):
        target = statement.targets[0].id
        assert('|' in target)
        targetName, targetId = target.split('|')

        renamedExpression = renameVariables(statement.value, variableIdMap)

        if (targetName, targetId) in constantVariableSet:
            expressionText = ast.dump(renamedExpression)

            if expressionText in constantVariableExpressionMap:
                # If target exists in constantVariableExpressionMap with identical expression, drop
                # this statement
                existingVar = constantVariableExpressionMap[expressionText]
                variableIdMap[targetId] = existingVar
                return None
            else:
                # Otherwise add this to constantVariableExpressionMap, so future duplicates would
                # use it
                constantVariableExpressionMap[expressionText] = target
                # The assignment is to the same target, but its expression may need renaming.
                # That's done at the end of this function

    # Otherwise just rename the variables in the statement.
    return renameVariables(statement, variableIdMap)

def canMerge(vu1, bodyIndex1, vu2, bodyIndex2, variableIdMap):
    node1 = vu1.body[bodyIndex1]
    node2 = vu2.body[bodyIndex2]

    assert(isinstance(node1, ast.If) or isinstance(node1, ast.For))
    assert(isinstance(node2, ast.If) or isinstance(node2, ast.For))

    # Ifs can merge if their expression is identical
    if isinstance(node1, ast.If) and isinstance(node2, ast.If):
        test1 = ast.dump(renameVariables(node1.test, variableIdMap))
        test2 = ast.dump(renameVariables(node2.test, variableIdMap))
        return test1 == test2

    # Fors can merge if:
    # - Both are branchless
    # - Their iterator is identical
    #
    if isinstance(node1, ast.For) and isinstance(node2, ast.For):
        if node1 not in vu1.branchlessBlockMap or node2 not in vu2.branchlessBlockMap:
            return False

        iter1 = ast.dump(renameVariables(node1.iter, variableIdMap))
        iter2 = ast.dump(renameVariables(node2.iter, variableIdMap))
        if iter1 != iter2:
            return False

        # The loop variable of node2 is mapped to the loop variable of node1 in preparation or
        # merge.
        assert('|' in node1.target.id)
        assert('|' in node2.target.id)
        _, target2Id = node2.target.id.split('|')
        variableIdMap[target2Id] = node1.target.id

        return True

    return False


def mergeFor(vus, nextIndex, variableIdMap):
    vuCount = len(vus)

    mergeList = []

    # Get the bodies of all involved for loops
    for vuIndex in range(vuCount):
        vu = vus[vuIndex]
        forStatement = vu.body[nextIndex[vuIndex]]
        assert(isinstance(forStatement, ast.For))

        body = MergeInfo((forStatement, not IS_ELSE_BLOCK), forStatement.body,
                         vu.constantVariableSet,
                         vu.branchlessBlockMap)
        mergeList.append(body)

    # Merge the bodies
    mergedResult = mergeVuBodies(mergeList, variableIdMap)

    for0 = vus[0].body[nextIndex[0]]
    for0.iter = renameVariables(for0.iter, variableIdMap)
    for0.body = mergedResult
    return for0


def mergeIf(vus, nextIndex, variableIdMap):
    vuCount = len(vus)

    trueMergeList = []
    falseMergeList = []

    # Get the bodies of all involved if blocks
    for vuIndex in range(vuCount):
        vu = vus[vuIndex]
        ifStatement = vu.body[nextIndex[vuIndex]]
        assert(isinstance(ifStatement, ast.If))

        trueBody = MergeInfo((ifStatement, not IS_ELSE_BLOCK), ifStatement.body,
                         vu.constantVariableSet,
                         vu.branchlessBlockMap)
        trueMergeList.append(trueBody)

        falseBody = MergeInfo((ifStatement, IS_ELSE_BLOCK), ifStatement.orelse,
                         vu.constantVariableSet,
                         vu.branchlessBlockMap)
        falseMergeList.append(falseBody)

    # Merge the bodies
    trueMergedResult = mergeVuBodies(trueMergeList, variableIdMap)
    falseMergedResult = mergeVuBodies(falseMergeList, variableIdMap)

    if0 = vus[0].body[nextIndex[0]]
    if0.test = renameVariables(if0.test, variableIdMap)
    if0.body = trueMergedResult
    if0.orelse = falseMergedResult
    return if0


def mergeVuBodies(vus, variableIdMap):
    """vus is a list of MergeInfos.  The given bodies are concatenated up to loops and conditions.
    Identical constant variables are coalesced by mapping their unique ids to one of them.

    The ifs are grouped by their condition, and the bodies of each group is recursively merged.

    The for loops are grouped by their iterator and the bodies of each group is recursively merged.
    Loops with branches are not merged.

    While loops are concatenated untouched.

    What comes after the merged blocks are recursively merged as well.
    """

    vuCount = len(vus)
    processed = [False] * vuCount
    nextIndex = [0] * vuCount

    # The result is a new body
    result = []

    # A map of constant variables to their value expression.  Used to determine if another constant
    # variable can be coalesced to an existing one.
    constantVariableExpressionMap = {}

    while not all(processed):
        # First, traverse up to the first for or if on each VU
        for vuIndex in range(vuCount):
            vu = vus[vuIndex]
            while nextIndex[vuIndex] < len(vu.body):
                statement = vu.body[nextIndex[vuIndex]]
                if isinstance(statement, ast.If) or isinstance(statement, ast.For):
                    break
                nextIndex[vuIndex] += 1

                # Duplicate the statement in the result
                duplicate = duplicateStatement(statement, variableIdMap,
                                               vu.constantVariableSet[vu.scope],
                                               constantVariableExpressionMap)
                if duplicate is not None:
                    result.append(duplicate)

            if nextIndex[vuIndex] == len(vu.body):
                processed[vuIndex] = True

        # Now take the first VU with anything left and attempt to merge its if/for with another VU.
        # Once a group is processed, restart the loop.  When no merging is done, this retains the
        # ordering of the VUs; when merging is done only that portion of the merged VU is moved up.
        for vuIndex in range(vuCount):
            if processed[vuIndex]:
                continue

            vu = vus[vuIndex]
            vuNextIndex = nextIndex[vuIndex]
            vuStatement = vu.body[vuNextIndex]
            mergeList = [vu]
            mergeNextIndex = [vuNextIndex]
            nextIndex[vuIndex] += 1

            # If any other VU can be merged in, bundle them together
            for otherIndex in range(vuIndex + 1, vuCount):
                if processed[otherIndex]:
                    continue

                other = vus[otherIndex]
                otherNextIndex = nextIndex[otherIndex]
                if canMerge(vu, vuNextIndex, other, otherNextIndex, variableIdMap):
                    mergeList.append(other)
                    mergeNextIndex.append(otherNextIndex)
                    nextIndex[otherIndex] += 1
                    if nextIndex[otherIndex] == len(other.body):
                        processed[otherIndex] = True

            # Create the shared node and recursively merge the bodies
            if len(mergeList) > 1:
                if isinstance(vuStatement, ast.For):
                    result.append(mergeFor(mergeList, mergeNextIndex, variableIdMap))
                else:
                    assert(isinstance(vuStatement, ast.If))
                    result.append(mergeIf(mergeList, mergeNextIndex, variableIdMap))
            else:
                result.append(renameVariables(vuStatement, variableIdMap))

            # Continue from the first VU.  Otherwise the ifs and fors of VUs can get interleaved.
            # Chances of this leading to more or fewer merged blocks is the same.
            break

    return result

def mergeVus(vus):
    """Merge all VUs of a given API token"""

    variableIdMap = {}
    originalASTs = []
    analyzedVUs = []

    nextUniqueId = 0

    for vu in vus:
        # Analyze each VU, and keep the transformed VUs in the same order.  The require() calls are
        # tagged with this index.
        analyzer = VuAnalyzer(vu, nextUniqueId, len(originalASTs))
        analyzer.analyze()

        # Note: The AST is deep copied to retain the original; mergeVuBodies is instead free to
        # modify the ASTs as necessary.
        originalASTs.append(analyzer.getTaggedVu())
        nextUniqueId = analyzer.nextUniqueId

        # Create the merge info for this VU
        analyzedVUs.append(MergeInfo((analyzer.vuAST, not IS_ELSE_BLOCK), analyzer.vuAST.body,
                                     analyzer.constantVariableSet,
                                     analyzer.branchlessBlockMap))

    # Merge the VUs and return the result as well as aggregated information (original ASTs and id
    # map)
    result = mergeVuBodies(analyzedVUs, variableIdMap)
    return ast.Module(result, []), originalASTs, variableIdMap
