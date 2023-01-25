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

"""Used to generate VVL code from codified VUs."""

import ast
from collections import namedtuple

import spec_tools.util as util
from vuAST import VuAST, VuTypeExtractor, VuFormatter

from vu_merger import mergeVus
from vu_message import generateVuMessage, VuPrintfStyler

# A few functions in VVL have an extra state parameter.
APIS_WITH_ADDITIONAL_STATE_ARG = {'vkCreateGraphicsPipelines',
                                  'vkCreateComputePipelines',
                                  'vkCreateRayTracingPipelinesNV',
                                  'vkCreateRayTracingPipelinesKHR',
                                  'vkAllocateDescriptorSets', }
ADDITIONAL_STATE_ARG = 'validation_state'

Type = namedtuple('Type', ['typeStr', 'pointerLevel', 'arrayLen'], defaults=['', None])

def removeOneArrayLevel(t):
    lastStar = t.pointerLevel.rfind('*')

    assert(lastStar != 1)
    secondLastStar = t.pointerLevel.rfind('*', 0, lastStar)

    if secondLastStar == -1:
        return Type(t.typeStr, t.pointerLevel[:lastStar], None)

    return Type(t.typeStr, t.pointerLevel[:secondLastStar + 1], None)

def getAPIMemberInfo(member):
    nameElem = member.find('name')
    assert(nameElem is not None)

    typeElem = member.find('type')
    assert(typeElem is not None)

    nameTail = nameElem.tail.strip() if nameElem.tail is not None else ''
    typeTail = typeElem.tail.strip()
    typeHead = member.text if member.text is not None else ''

    typeStr = typeHead + typeElem.text + typeTail

    # Default to altlen when available; len will have LaTeX markup
    lenAttr = 'altlen' if 'altlen' in member.attrib else 'len'
    arrayLen = member.attrib.get(lenAttr)
    # Take only the first array level. TODO: test multidimensional arrays
    if arrayLen is not None:
        arrayLen = arrayLen.split(',')[0]
    pointerLevel = typeTail

    if arrayLen is None and nameTail != '' and nameTail[0] == '[':
        arrayLen = member.find('enum').text
        typeStr += '[' + arrayLen + ']'
        pointerLevel += '*'

    return nameElem.text, typeStr, Type(typeElem.text, pointerLevel, arrayLen)

def getAPIParams(typeExtractor, api):
    # Get the api info out of the registry, looked up by VuTypeExtractor.
    info = typeExtractor.lookupAPIInfo(api)
    if info is None:
        print('Error: Invalid API name in VU ' + api)
        sys.exit(1)

    # Get the <member> or <param> of this struct / command
    isStruct = info.elem.tag == 'type' and info.elem.get('category') in ['struct', 'union']
    isCommand = info.elem.tag == 'command'
    if not (isStruct or isCommand):
        print('Error: API name with no member looked up: ' + api)
        sys.exit(1)

    return info.elem.findall('.//{}'.format('member' if isStruct else 'param'))

def getAPIParamInfo(typeExtractor, api, symbol):
    # For the specific case of 'VkPipelineCreateInfo' and 'flags', use any
    # of the real Vk*PipelineCreateInfo types.  They all have flags.
    if api == 'VkPipelineCreateInfo' and symbol == 'flags':
        api = 'VkGraphicsPipelineCreateInfo'

    members = getAPIParams(typeExtractor, api)
    for member in members:
        name = util.getElemName(member)
        if symbol == name:
            return getAPIMemberInfo(member)

    # VU contains an unrecognized symbol
    return '', 'const auto', Type('const auto')

# TODO: complete this list
statefulObjectTypeMap = {
    'VkPhysicalDevice': 'PHYSICAL_DEVICE_STATE',
    'VkSurfaceKHR': 'SURFACE_STATE',
    'VkBuffer': 'BUFFER_STATE',
    'VkImage': 'IMAGE_STATE',
    'VkDeviceMemory': 'DEVICE_MEMORY_STATE',
    'VkBufferView': 'BUFFER_VIEW_STATE',
    'VkImageView': 'IMAGE_VIEW_STATE',
    'VkSampler': 'SAMPLER_STATE',
    'VkSamplerYcbcrConversion': 'SAMPLER_YCBCR_CONVERSION_STATE',
    'VkFramebuffer': 'FRAMEBUFFER_STATE',
    'VkRenderPass': 'RENDER_PASS_STATE',
    'VkEvent': 'EVENT_STATE',
    'VkCommandPool': 'COMMAND_POOL_STATE',
    'VkDescriptorPool': 'DESCRIPTOR_POOL_STATE',
    'VkPipelineLayout': 'PIPELINE_LAYOUT_STATE',
    'VkPipeline': 'PIPELINE_STATE',
    'VkQueryPool': 'QUERY_POOL_STATE',
    'VkQuery': 'QUERY_STATE',
}

def lookUpStateObject(objType, expr):
    if objType.pointerLevel != '' or objType.arrayLen is not None:
        return []

    if objType.typeStr in statefulObjectTypeMap:
        return ['Get<', statefulObjectTypeMap[objType.typeStr], '>(', expr, ')']

    return []

def makeStateObjectName(name):
    return name + '_'

class VuCodegen(ast.NodeVisitor):
    def __init__(self, typeExtractor, api, members, vuids, originalVus, variableIdMap):
        self.api = api
        self.members = members
        self.typeExtractor = typeExtractor  # Note: registry can be retrieved from this if necessary

        self.vuids = vuids
        self.originalVus = originalVus
        self.variableIdMap = variableIdMap

        self.indent = 1
        self.variableTypeMap = {}
        self.loopVariableMap = {}

    def generate(self, vuAST):
        self.variableTypeMap = {}
        self.loopVariableMap = {}
        self.generated = []

        result = self.visit(vuAST)
        return result

    def regenerateSubExpression(self, expression):
        """Generate code for a subexpression that is previously visited.

        This is used by VuPrintfStyler to generate printf argument expressions.  That is only
        meaningful for expressions that are in scope (i.e. it cannot be called on expressions that
        have yet to be visited by VuCodegen.

        As such, this function will only need to look things up in the maps, but never add to them.
        """
        return self.visit(expression)

    def beginScope(self):
        # Scope always begins with { and a new line
        result = self.endLine()
        result += self.beginLine()
        result += ['{']
        result += self.endLine()
        self.indent += 1
        return result

    def endScope(self):
        # Scope ends with }
        self.indent -= 1
        result = self.beginLine()
        result += ['}']
        result += self.endLine()
        return result

    def beginParenthesis(self):
        return ['(']

    def endParenthesis(self):
        return [')']

    def makeIndent(self, indent):
        return ' ' * (indent * 4)

    def beginLine(self):
        return [self.makeIndent(self.indent)]

    def endLine(self):
        return ['\n']

    def getMappedName(self, name):
        if '|' in name:
            varName, varId = name.split('|')

            # Make sure merged variables use the merged variable name
            if varId in self.variableIdMap:
                return self.variableIdMap[varId]

        return name

    def makeVarName(self, name):
        # Change var|id to var_id to make it a valid name.  By adding id to the name, this makes
        # sure the name is unique in the generated code.
        assert('|' in name)
        return '_'.join(name.split('|'))

    def hasStateObject(self, objType):
        return objType.typeStr in statefulObjectTypeMap

    def getStateObject(self, expr, objType):
        # If it's a member, state object is already cached
        if expr in self.members:
            return [makeStateObjectName(expr)]

        result = lookUpStateObject(objType, expr)
        assert(len(result) > 0)
        return result

    def visitBody(self, statements):
        # Handle a list of statements, adding indentation appropriately
        body = self.beginScope()

        for statement in statements:
            body += self.beginLine()
            body += self.visit(statement)
            body += self.endLine()

        body += self.endScope()
        return body

    # Map of op classes to their textual representation
    opMap = {
        # Found in BoolOp
        ast.And: '&&',
        ast.Or: '||',
        # Found in BinOp
        ast.Add: '+',
        ast.Sub: '-',
        ast.Mult: '*',
        ast.MatMult: 'UNIMPLEMENTED',
        ast.Div: '/',
        ast.Mod: '%',
        ast.Pow: 'UNIMPLEMENTED',
        ast.LShift: '<<',
        ast.RShift: '>>',
        ast.BitOr: '|',
        ast.BitXor: '^',
        ast.BitAnd: '&',
        ast.FloorDiv: 'UNIMPLEMENTED',
        # Found in UnaryOp
        ast.Invert: '~',
        ast.Not: '!',
        ast.UAdd: '+',
        ast.USub: '-',
        # Found in CompOp
        ast.Eq: '==',
        ast.NotEq: '!=',
        ast.Lt: '<',
        ast.LtE: '<=',
        ast.Gt: '>',
        ast.GtE: '>=',
        ast.Is: 'UNIMPLEMENTED',
        ast.IsNot: 'UNIMPLEMENTED',
        ast.In: 'UNIMPLEMENTED',
        ast.NotIn: 'UNIMPLEMENTED',
    }

    def addBinaryExpression(self, left, op, right):
        leftExpr, leftType = self.visit(left)
        rightExpr, rightType = self.visit(right)

        expr = self.beginParenthesis()
        expr += leftExpr
        expr += [' ', self.opMap[op.__class__], ' ']
        expr += rightExpr
        expr += self.endParenthesis()

        resultType = None
        if op.__class__ in [ast.Eq, ast.NotEq, ast.Lt, ast.LtE, ast.Gt, ast.GtE]:
            resultType = Type('bool')
        elif op.__class__ in [ast.Add, ast.Sub, ast.Mult, ast.MatMult, ast.Div,
                              ast.Mod, ast.Pow, ast.FloorDiv, ast.LShift,
                              ast.RShift, ast.BitOr, ast.BitXor, ast.BitAnd]:
            resultType = leftType

        return expr, resultType

    def visit_Module(self, node):
        return self.visitBody(node.body)

    def visit_Expr(self, node):
        result, _ = self.visit(node.value)

        isRequire = (isinstance(node.value, ast.Call) and
                     isinstance(node.value.func, ast.Name) and
                     node.value.func.id == 'require')

        # require() calls are turned into if () ..., so don't add `;` for them.
        if not isRequire:
            result += [';']
        return result

    def visit_Assign(self, node):
        lhs = self.getMappedName(node.targets[0].id)
        rhs, rhsType = self.visit(node.value)

        assign = []

        if lhs not in self.variableTypeMap:
            if rhsType.pointerLevel != '':
                assign += ['const ']
            assign += [rhsType.typeStr, ' ', rhsType.pointerLevel]
            self.variableTypeMap[lhs] = rhsType

        assign += [self.makeVarName(lhs)]
        assign += ['=']
        assign += rhs
        assign += [';']

        return assign

    def visit_If(self, node):
        result = ['if ']
        result += self.beginParenthesis()
        test, _ = self.visit(node.test)
        result += test
        result += self.endParenthesis()
        result += self.visitBody(node.body)

        return result

    def visit_For(self, node):
        array, arrayType = self.visit(node.iter)
        loopVar = self.getMappedName(node.target.id)
        loopVarType = removeOneArrayLevel(arrayType)

        loopIndex = 'i' + str(self.indent)
        self.variableTypeMap[loopVar] = loopVarType
        self.loopVariableMap[loopVar] = loopIndex

        result = ['for ']
        result += self.beginParenthesis()
        result += ['uint32_t ' + loopIndex + ' = 0; ' + loopIndex + ' < ' + arrayType.arrayLen + '; ++' + loopIndex]
        result += self.endParenthesis()
        result += self.beginScope()

        result += self.beginLine()
        result += ['const ' + loopVarType.typeStr + loopVarType.pointerLevel + ' &' +
                   self.makeVarName(loopVar) + ' = ']
        result += array
        result += ['[' + loopIndex + '];']

        result += self.visitBody(node.body)
        result += self.endScope()

        return result

    def visit_While(self, node):
        result = ['while ']
        result += self.beginParenthesis()
        test, _ = self.visit(node.test)
        result += test
        result += self.endParenthesis()
        result += self.visitBody(node.body)

        return result

    def visit_Break(self, node):
        return ['break;']

    def visit_Continue(self, node):
        return ['continue;']

    def visit_BoolOp(self, node):
        result = self.beginParenthesis()

        opText = self.opMap[node.op.__class__]

        first = True
        for value in node.values:
            if not first:
                result += [opText]
            first = False

            expr, _ = self.visit(value)
            result += expr

        result += self.endParenthesis()

        return result, Type('bool')

    def visit_UnaryOp(self, node):
        opText = self.opMap[node.op.__class__]

        result = [opText]
        result += self.beginParenthesis()
        expr, operandType = self.visit(node.operand)
        result += expr
        result += self.endParenthesis()

        return result, operandType

    def visit_BinOp(self, node):
        return self.addBinaryExpression(node.left, node.op, node.right)

    def visit_Compare(self, node):
        return self.addBinaryExpression(node.left, node.ops[0], node.comparators[0])

    def generate_loop_index(self, node):
        loopTarget = self.getMappedName(node.args[0].id)
        return [self.loopVariableMap[loopTarget]], self.variableTypeMap[loopTarget]

    def generate_require(self, node):
        result = []

        condition = node.args[0]
        vuIndex = node.args[1].value
        requireIndex = node.args[2].value

        result += ['if (!(']
        condExpr, _ = self.visit(condition)
        result += condExpr
        result += ['))']

        result += self.beginScope()
        result += self.beginLine()

        # Generate an error message based on the original VU, extracting relevant objects,
        # adding relevant values and highlighting the failed require().
        formatter = VuFormatter(VuPrintfStyler(self, requireIndex))
        message, objects = generateVuMessage(formatter, self.originalVus[vuIndex])

        # List of objects that are involved in the VU
        result += ['const LogObjectList objlist{', ', '.join(objects), '};']
        result += self.endLine()
        result += self.beginLine()

        # The error message itself
        indent = self.makeIndent(self.indent + 1)
        result += ['skip |= LogFail(objlist, "', self.vuids[vuIndex], '",\n',
                   ',\n'.join(indent + arg for arg in message), ');']
        result += self.endLine()
        result += self.endScope()

        return result

    def visit_Call(self, node):
        funcname = ''
        obj = None
        objType = None
        if isinstance(node.func, ast.Attribute):
            funcname = node.func.attr
            obj, objType = self.visit(node.func.value)
        else:
            funcname = node.func.id

        if funcname == 'require':
            return self.generate_require(node), Type('void')
        if funcname == 'loop_index':
            return self.generate_loop_index(node)

        result = ['Builtin_' + funcname]

        ispnext = funcname == 'has_pnext' or funcname == 'pnext'

        # For has_pnext() and pnext() builtins, get the structure type out of the first argument and
        # use that as template argument.  Then the object would have to be suffixed with `.pNext`,
        # or if there's no object, the pNext function argument should be used.
        args = node.args
        if ispnext:
            args = node.args[1:]
            result += ['<', node.args[0].id, '>']

            if obj is None:
                obj = ['pNext']

        result += self.beginParenthesis()

        # Some built-ins need the state object (like IMAGE_STATE) instead of the handle.
        needsStateObject = funcname.endswith('create_info')

        delimiter = []
        if obj:
            if needsStateObject and self.hasStateObject(objType):
                obj = self.getStateObject(''.join(obj), objType)

            result += obj
            delimiter = [', ']

        for arg in args:
            result += delimiter
            delimiter = [', ']

            argExpr, _ = self.visit(arg)
            result += argExpr

        result += self.endParenthesis()

        resultType = Type('void')
        if funcname in ['has_pnext', 'ext_enabled', 'externally_synchronized', 'has_bit', 'any', 'none', 'valid']:
            resultType = Type('bool')
        elif funcname == 'pnext':
            resultType = Type(node.args[0].id, pointerLevel='*')
        elif funcname == 'create_info':
            resultType = Type(objType.typeStr + 'CreateInfo')
        elif funcname == 'graphics_create_info':
            resultType = Type(objType.typeStr[:2] + 'Graphics' + objType.typeStr[2:] + 'CreateInfo')
        elif funcname == 'compute_create_info':
            resultType = Type(objType.typeStr[:2] + 'Compute' + objType.typeStr[2:] + 'CreateInfo')
        elif funcname == 'raytracing_create_info':
            resultType = Type(objType.typeStr[:2] + 'RayTracing' + objType.typeStr[2:] + 'CreateInfo')

        return result, resultType

    def visit_Attribute(self, node):
        obj, objType = self.visit(node.value)

        result = obj
        if '*' in objType.pointerLevel:
            result += ['->']
        else:
            result += ['.']
        result += [node.attr]

        _, _, memberType = getAPIParamInfo(self.typeExtractor, objType.typeStr, node.attr)
        return result, memberType

    def visit_Subscript(self, node):
        array, arrayType = self.visit(node.value)
        index, _ = self.visit(node.slice)

        result = array
        result += ['[']
        result += index
        result += [']']

        return result, removeOneArrayLevel(arrayType)

    def visit_IfExp(self, node):
        left, leftType = self.visit(node.body)
        cond, _ = self.visit(node.test)
        right, rightType = self.visit(node.orelse)

        result = self.beginParenthesis()

        result += self.beginParenthesis()
        result += cond
        result += self.endParenthesis()

        result += [' ? ']
        result += left
        result += [' : ']

        result += self.beginParenthesis()
        result += right
        result += self.endParenthesis()

        result += self.endParenthesis()

        return result, leftType

    def visit_Constant(self, node):
        if isinstance(node.value, str):
            result = ['"']
            result += [node.value]
            result += ['"']
            return result, Type('const char', '*')

        if isinstance(node.value, bool):
            result = ['true' if node.value else 'false']
            return result, Type('bool')

        # TODO: correct type
        if isinstance(node.value, float):
            return [str(node.value), 'float']
        if node.value < 0:
            return [str(node.value), 'int32_t']
        return [str(node.value), 'uint32_t']

    def visit_Name(self, node):
        name = self.getMappedName(node.id)
        if name in self.variableTypeMap:
            varType = self.variableTypeMap[name]
            return [self.makeVarName(name)], varType

        assert('|' not in node.id)
        _, _, varType = getAPIParamInfo(self.typeExtractor, self.api, node.id)

        return [node.id], varType


def compileVU(vuText):
    vu = VuAST()
    vu.parse('\n'.join(vuText), '<generated>', 0)
    vu.applyMacros({})
    return vu

def generateInit(membersInfo):
    """Generate look up code for any object that has state tracking (VkImage,
    VkImageView, etc), so it doesn't need to be looked up so often."""

    lookUpCode = []

    for name, _, objType in membersInfo:
        # If this object is stateful, get the state object.
        stateObject = lookUpStateObject(objType, name)
        if len(stateObject) > 0:
            lookUpCode += ['    auto ', makeStateObjectName(name), ' = '] + stateObject + [';\n']

    return lookUpCode

def generateValidation(registry, api, vus):
    """Given a list of (vuid, vu), this function compiles the vus, merges them into one AST and
    generates code accordingly for the api token. Returns the generated code."""

    # First, compile the vus.
    vuids = [vuid for vuid, _ in vus]
    compiledVUs = [compileVU(vu).astExpanded for _, vu in vus]

    # Merge the ASTs
    merged, originalVus, variableIdMap = mergeVus(compiledVUs)

    # Create a preamble where state objects are cached for members if applicable
    typeExtractor = VuTypeExtractor(registry, api)
    members = getAPIParams(typeExtractor, api)
    membersInfo = [getAPIMemberInfo(member) for member in members]
    init = generateInit(membersInfo)

    memberNames = [name for name, _, _ in membersInfo]

    generator = VuCodegen(typeExtractor, api, memberNames, vuids, originalVus, variableIdMap)
    return init + generator.generate(merged)
