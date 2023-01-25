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

"""Used to generate a validation error message from a VU AST.

Along with the AST, an index is used to specify which `require()` call has failed.  This generator
formats the AST using VuFormatter from the spec scripts, adding in values to any expression of
interest in the scope of the target `require()`.

Additionally, it extracts the objects involved in the VU
"""

import ast
from collections import namedtuple
import copy
import re

# TODO: remove; for debugging
import xml.etree.ElementTree as etree

stylePattern = re.compile(r'\${vu-[a-z-]*}')
valuePattern = re.compile(r'{{[^}]*}}')
styleAndValuePattern = re.compile(r'\${vu-value}{{[^}]*}}\${}')

REQUIRE_FAIL_MESSAGE = '${vu-fail} <-- Failing condition (Values captured at this instant)${}'

STR_BEGIN = 'R"('
STR_END = ')"'

def entityIsHandle(entity):
    return entity is not None and entity.get('category') == 'handle'

def deduplicate(objects):
    """If the same object is added to the list multiple times, deduplicate them."""
    return list(set(objects))

class VuPrintfStyler:
    """Used by VuFormatter from the spec scripts to generate a printf-style format string.  See
    other stylers in the spec scripts (e.g. VuOutputStyler) for reference."""
    def __init__(self, codegen, requireIndex):
        self.codegen = codegen
        self.requireIndex = requireIndex

        self.space = '  '
        """String used for indentation."""

        self.endOfLine = '\n'
        """String used to indicate end-of-line."""

        self.initialIndent = 2
        """Indent by 4 spaces for readability."""

        self.args = []
        """Arguments for the generated format string."""

        self.objects = []
        """Objects of interest."""

        self.scopeStack = []
        """Stack of indices to args and objects, marking the beginning of the scope.  If the desired
        require() is not in this scope, no variable inside the scope is printed."""

        self.requireVisited = False
        """True if the desired require is already visited.  Used to stop outputting values after
        it."""

        self.currentRequireIndex = 0
        """The index of the next require() to be seen.  Used to compare with requireIndex to
        identify the require() call that's failing."""

    def verifyIdentical(self, originalAST, formatted):
        toVerify = formatted

        # Strip all annotations first
        toVerify = toVerify.replace(REQUIRE_FAIL_MESSAGE, '')
        toVerify = toVerify.replace('${}', '')
        toVerify = re.sub(stylePattern, '', toVerify)
        toVerify = re.sub(valuePattern, '', toVerify)

        # Remove initial indentation
        toVerify = '\n'.join([line[len(self.space) * self.initialIndent:] for line in toVerify.split('\n')])

        # Fix up the original AST to change variable|id to variable, as well as remove the
        # additional tracking indices added to require() calls
        tree = copy.deepcopy(originalAST)
        for node in ast.walk(tree):
            if isinstance(node, ast.Name):
                node.id = node.id.split('|')[0]

        formattedTree = ast.parse(toVerify, '', 'exec')

        # Make sure reformatted VU (without annotations) is the same as the original VU.
        assert(ast.dump(formattedTree) == ast.dump(tree))

    def beginStyle(self, style, delimiter = ''):
        # Skip the all-encompassing [vu]#...# style added by the formatter
        if delimiter == '#':
            return []
        return ['${', style, '}']

    def endStyle(self, delimiter = ''):
        if delimiter == '#':
            return []
        return ['${}']

    def beginLink(self, link):
        return []

    def endLink(self):
        return []

    def getNameSymbol(self, node):
        symbol = self.codegen.getMappedName(node.id)
        noIdSymbol = symbol.split('|')[0]

        # If the failing require() is already visited, just return the symbol.  See
        # visitSubExpression.
        if self.requireVisited:
            return [noIdSymbol]

        expr, exprType = self.codegen.regenerateSubExpression(node)

        # If this is a loop variable, print its loop index regardless of whether the variable type
        # is printable
        loopIndexSpecifier = None
        if symbol in self.codegen.loopVariableMap:
            loopIndexSpecifier = '@%' + STR_END + ' PRIu32 ' + STR_BEGIN
            self.args.append(self.codegen.loopVariableMap[symbol])

        # If symbol is not printable, just return it (potentially with loop index only, if
        # applicable).  In this case, {{@index}} is printed.
        specifier, entity = self.getPrintfConversionSpecifier(exprType)
        if specifier == None:
            result = [noIdSymbol]
            if loopIndexSpecifier is not None:
                result += self.makePrintfValue(loopIndexSpecifier)
            return result

        arg, obj = self.getPrintfArg(expr, exprType, entity)
        self.onNewArg(arg, obj)

        # Prefix the symbol with its loop index if applicable.  In this case, {{@index:value}} is
        # printed.
        if loopIndexSpecifier is not None:
            specifier = loopIndexSpecifier + ':' + specifier

        return [noIdSymbol] + self.makePrintfValue(specifier)

    def onBuiltInVisit(self, name):
        # Make sure there is no `macro()`
        assert(name != 'macro')

    def onBeginScope(self):
        self.scopeStack.append((len(self.args), len(self.objects)))

    def onEndScope(self, scope):
        argsStart, objectsStart = self.scopeStack.pop()

        # If require was not in this scope, remove all {{...}} annotations from the scope; the
        # failing require is not in this scope, so it doesn't have access to values in it.
        if not self.requireVisited:
            self.args = self.args[:argsStart]
            self.objects = self.args[:objectsStart]

            return [re.sub(styleAndValuePattern, '', ''.join(scope))]
        return scope

    def onCallVisit(self, node):
        isRequire = isinstance(node.func, ast.Name) and node.func.id == 'require'

        if isRequire:
            result = []

            # Identify the failing require
            if self.currentRequireIndex == self.requireIndex:
                self.requireVisited = True
                result = [REQUIRE_FAIL_MESSAGE]

            self.currentRequireIndex += 1
            return result

        # Generate a value for builtins that return something printable (generally boolean)
        return self.visitSubExpression(node)

    def onAttributeVisit(self, node):
        return self.visitSubExpression(node)

    def onSubscriptVisit(self, node):
        return self.visitSubExpression(node)

    def getPrintfConversionSpecifier(self, exprType):
        """Get the %?? conversion specifier for this type.  If the type cannot be printed, None is
        returned."""
        if exprType.arrayLen != None:
            return None, None

        if exprType.pointerLevel != '':
            return None, None

        typeStr = exprType.typeStr

        typedict = self.codegen.typeExtractor.registry.typedict
        if typeStr in typedict:
            info = typedict[typeStr]
            category = info.elem.get('category')
            # Handles are printed with FormatHandle
            if category == 'handle':
                return '%s', info.elem
            # For flags, get the underlying flag type (VkFlags or VkFlags64)
            if category == 'bitmask':
                typeStr = info.elem.find('type').text
            # TODO: any other category that may need special attention

        if typeStr == 'uint8_t':
            return '%' + STR_END + ' PRIu8 ' + STR_BEGIN, None
        if typeStr == 'int8_t':
            return '%' + STR_END + ' PRId8 ' + STR_BEGIN, None
        if typeStr == 'uint16_t':
            return '%' + STR_END + ' PRIu16 ' + STR_BEGIN, None
        if typeStr == 'int16_t':
            return '%' + STR_END + ' PRId16 ' + STR_BEGIN, None
        if typeStr  == 'uint32_t':
            return '%' + STR_END + ' PRIu32 ' + STR_BEGIN, None
        if typeStr == 'int32_t':
            return '%' + STR_END + ' PRId32 ' + STR_BEGIN, None
        if typeStr in ['uint64_t', 'VkDeviceSize', 'VkDeviceAddress']:
            return '%' + STR_END + ' PRIu64 ' + STR_BEGIN, None
        if typeStr == 'int64_t':
            return '%' + STR_END + ' PRId64 ' + STR_BEGIN, None
        if typeStr == 'float':
            return '%f', None
        if typeStr == 'double':
            return '%lf', None
        if typeStr == 'size_t':
            return '%zu', None
        if typeStr in ['VkSampleMask', 'VkFlags']:
            return '%#' + STR_END + ' PRIx32 ' + STR_BEGIN, None
        if typeStr == 'VkFlags64':
            return '%#' + STR_END + ' PRIx32 ' + STR_BEGIN, None
        if typeStr in ['bool', 'VkBool32']:
            return '%s', None

        # TODO: make sure everything is covered.  In particular, check to see if VkSampleMask,
        # VkBool32, VkDeviceSize etc end up with those names or their base type names.
        print('===== No known conversion for ' + exprType.typeStr)
        return None, None

    def getPrintfArg(self, expr, exprType, entity):
        expr = ''.join(expr)

        # For booleans, output `expr ? "true" : "false".
        if exprType.typeStr in ['bool', 'VkBool32']:
            return '(' + expr + ') ? "true" : "false"', None

        # For handles, use FormatHandle
        if entityIsHandle(entity):
            return 'report_data->FormatHandle(' + expr + ').c_str()', expr

        # For the rest, the expression itself is enough.
        return expr, None

    def makePrintfValue(self, specifier):
        return self.beginStyle('vu-value') + ['{{', specifier, '}}'] + self.endStyle()

    def onNewArg(self, arg, obj):
        self.args.append(arg)

        # If this is a handle, add it to the objects list as well.
        if obj is not None:
            self.objects.append(obj)

    def visitSubExpression(self, node):
        # If the failing require() is already visited, don't generate anything.  The code after this
        # require hasn't executed yet.
        if self.requireVisited:
            return []

        expr, exprType = self.codegen.regenerateSubExpression(node)

        specifier, entity = self.getPrintfConversionSpecifier(exprType)
        if specifier == None:
            return []

        arg, obj = self.getPrintfArg(expr, exprType, entity)
        self.onNewArg(arg, obj)

        return self.makePrintfValue(specifier)


def generateVuMessage(vuFormatter, vuAST):
    """Generate a message for the VU at the failed require().

    A printf-consumable format string is returned along with a list of arguments for that format
    string.

    Additionally, a list of objects of interest are returned.  For efficiency, the objects that are
    part of the API token"""

    message = vuFormatter.format(vuAST)
    return ([STR_BEGIN + message + STR_END] + vuFormatter.styler.args,
            deduplicate(vuFormatter.styler.objects))
