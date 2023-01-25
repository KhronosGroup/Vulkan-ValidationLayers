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

"""Tests for vu_merger.py"""

import ast
import pytest

from vu_merger import mergeVus

def cleanUpAST(vuAST, variableIdMap):
    for node in ast.walk(vuAST):
        if isinstance(node, ast.Name) and '|' in node.id:
            varName, varId = node.id.split('|')
            if varId in variableIdMap:
                node.id = variableRenameMap[varId].split('|')[0]
            else:
                node.id = varName
        elif (isinstance(node, ast.Call) and
              isinstance(node.func, ast.Name) and
              node.func.id == 'require'):
            assert(len(node.args) == 2)
            node.args.pop()
    return vuAST

def uniqueVars(vuAST, variableIdMap):
    for node in ast.walk(vuAST):
        if isinstance(node, ast.Name) and '|' in node.id:
            _, varId = node.id.split('|')
            if varId in variableIdMap:
                node.id = variableRenameMap[varId]
            node.id = node.id.replace('|', '_')
    return vuAST


def runTest(vuList, expected):
    vus = [ast.parse(vu) for vu in vuList]
    dumpedVus = [ast.dump(vu, indent=' ') for vu in vus]
    merged, originalASTs, variableIdMap = mergeVus(vus)

    # Make sure original ASTs are identical to given vus.  Remove id from variable names and index
    # from require() calls, then make sure it gets dumped to the same AST.
    dumpedVus2 = [ast.dump(cleanUpAST(vu, {}), indent=' ') for vu in originalASTs]
    for index in range(len(vuList)):
        if dumpedVus[index] != dumpedVus2[index]:
            print('Analyzed AST different from original (index {})'.format(index))
            print(dumpedVus[index])
            print(dumpedVus2[index])
        assert(dumpedVus[index] == dumpedVus2[index])

    # Process the merged result the same way codegen is expected to process them by removing ids
    # from variable names (and if needed replacing them with what they are mapped to)
    merged = uniqueVars(merged, variableIdMap)
    print(ast.unparse(merged))

    assert(ast.dump(merged, indent=' ') == ast.dump(ast.parse(expected), indent=' '))

def test_single_vu():
    vus = ["""var1 = a + b
varC1 = x * c
if varC1 > 0:
  var1 = varC1
  var2 = a + varC1
  for value in values:
    require(value + var2 + var1 > varC1)
  var2 = b + varC1
else:
  var3 = var1.has_bit(foo)
  varC2 = a + varC1
  for value in values:
    require(varC2 == var3)
  var2 = b + varC1
for value in values:
  require(value != -value)
require(varC1 == varC1)"""]

    expect = """var1_0 = a + b
varC1_1 = x * c
if varC1_1 > 0:
 var1_0 = varC1_1
 var2_2 = a + varC1_1
 for value_3 in values:
  require(value_3 + var2_2 + var1_0 > varC1_1, 0)
 var2_2 = b + varC1_1
else:
 var3_4 = var1_0.has_bit(foo)
 varC2_5 = a + varC1_1
 for value_6 in values:
  require(varC2_5 == var3_4, 0)
 var2_7 = b + varC1_1
for value_8 in values:
 require(value_8 != -value_8, 0)
require(varC1_1 == varC1_1, 0)
"""

    runTest(vus, expect)

def test_constant_vars():
    vus = ["""var1 = const1
var2 = var1 + const2
var3 = variable
var3 = var3 + var1
var4 = variable2
var4 = var4 + var2""",
           """name1 = const1
var2 = name1 + const2
var3 = variable
var3 = var3 + name1
name4 = variable2
name4 = name4 + var2"""]

    # name1 and var2 from second VU should be dropped and replaced by var1 and var2 from the first.
    expect = """var1_0 = const1
var2_1 = var1_0 + const2
var3_2 = variable
var3_2 = var3_2 + var1_0
var4_3 = variable2
var4_3 = var4_3 + var2_1
var3_6 = variable
var3_6 = var3_6 + var1_0
name4_7 = variable2
name4_7 = name4_7 + var2_1"""

    runTest(vus, expect)

def test_constant_vars2():
    vus = ["""a = constant1""",
           """b = constant1
a = variable
a = a + 1"""]

    # name1 and var2 from second VU should be dropped and replaced by var1 and var2 from the first.
    expect = """a_0 = constant1
a_2 = variable
a_2 = a_2 + 1"""

    runTest(vus, expect)

def test_constant_vars3():
    vus = ["""a = constant1""",
           """b = constant1
a = constant2"""]

    # name1 and var2 from second VU should be dropped and replaced by var1 and var2 from the first.
    expect = """a_0 = constant1
a_2 = constant2"""

    runTest(vus, expect)

def test_ifs():
    vus = ["""a = constant1
x = constant4
if a == constant1:
 b = variable1
 while not b:
  b = not b
 c = b
 require(c)

if a != constant1 and x == constant4:
 b = constant2
 require(a == b)""",
           """d = constant1
if d == constant1:
 b = variable1
 while b:
  b = b and constant3
 c = b
 require(c)

y = constant4
if d != constant1 and y == constant4:
 a = constant2
 require(a + d > 0)"""]

    # Both pairs of ifs should be merged.  The variables and constants inside each pair of bodies
    # shouldn't get mixed up.
    expect = """a_0 = constant1
x_1 = constant4
if a_0 == constant1:
 b_2 = variable1
 while not b_2:
  b_2 = not b_2
 c_3 = b_2
 require(c_3, 0)
 b_6 = variable1
 while b_6:
  b_6 = b_6 and constant3
 c_7 = b_6
 require(c_7, 1)

if a_0 != constant1 and x_1 == constant4:
 b_4 = constant2
 require(a_0 == b_4, 0)
 require(b_4 + a_0 > 0, 1)"""

    runTest(vus, expect)

def test_if_elses():
    vus = ["""a = constant1
if a == constant1:
 b = variable1
 while not b:
  b = not b
 require(b)
else:
 b = constant2
 require(b)

if a != constant1:
 b = constant2
 require(a == b)""",
           """d = constant1
if d == constant1:
 b = constant2
 require(b)
else:
 b = variable1
 while b:
  b = b and constant3
 require(b)

if d != constant1:
 a = constant2
 require(a + d > 0)
else:
 require(True)""",
           """c = constant1
if c == constant1:
 require(c == constant1)"""]

    # Basic test of else bodies getting merged
    expect = """a_0 = constant1
if a_0 == constant1:
 b_1 = variable1
 while not b_1:
  b_1 = not b_1
 require(b_1, 0)
 b_5 = constant2
 require(b_5, 1)
 require(a_0 == constant1, 2)
else:
 b_2 = constant2
 require(b_2, 0)
 b_6 = variable1
 while b_6:
  b_6 = b_6 and constant3
 require(b_6, 1)

if a_0 != constant1:
 b_3 = constant2
 require(a_0 == b_3, 0)
 require(b_3 + a_0 > 0, 1)
else:
 require(True, 1)"""

    runTest(vus, expect)

def test_ifs_negative():
    vus = ["""a = constant1
if a == constant1:
 require(a)

if a != constant1:
 b = constant2
 require(a == b)""",
           """d = constant1
if d == constant1 or constant3:
 require(d != constant3)
elif d == constant1:
 require(should_not_merge())

if d != constant1:
 a = constant2
 require(a + d > 0)"""]

    # First pair of ifs shouldn't merge, the second pair should.
    expect = """a_0 = constant1
if a_0 == constant1:
 require(a_0, 0)

if a_0 != constant1:
 b_1 = constant2
 require(a_0 == b_1, 0)
if a_0 == constant1 or constant3:
 require(a_0 != constant3, 1)
elif a_0 == constant1:
 require(should_not_merge(), 1)
if a_0 != constant1:
 a_3 = constant2
 require(a_3 + a_0 > 0, 1)
"""

    runTest(vus, expect)

def test_fors():
    vus = ["""b = variable1
for r in rs:
 b = b + 1
require(b < constant1)

for t in ts:
 if t.a != -1:
  require(t.a >= 0)""",

           """for a in rs:
 require(a != -1)
 if a == 0:
  require(loop_index(a) == 0)

for b in ts:
 if b.a != -1:
  require(b.b > b.a)
 require(constant2)"""]

    # Basic test of for bodies getting merged
    expect = """b_0 = variable1
for r_1 in rs:
 b_0 = b_0 + 1
 require(r_1 != -1, 1)
 if r_1 == 0:
  require(loop_index(r_1) == 0, 1)
require(b_0 < constant1, 0)

for t_2 in ts:
 if t_2.a != -1:
  require(t_2.a >= 0, 0)
  require(t_2.b > t_2.a, 1)
 require(constant2, 1)"""

    runTest(vus, expect)

def test_fors_iter_mismatch():
    vus = ["""for r in rs:
 require(r < constant1)""",

           """array = rs
for r in array:
 require(r > constant2)""",

           """for r in rs2:
 require(r == constant3)"""]

    # None of the loops should merge because their iterators are mismatching
    expect = """array_1 = rs
for r_0 in rs:
 require(r_0 < constant1, 0)

for r_2 in array_1:
 require(r_2 > constant2, 1)

for r_3 in rs2:
 require(r_3 == constant3, 2)"""

    runTest(vus, expect)

def test_fors_branch():
    vus = ["""for r in rs:
 if loop_index(r) == 0:
  continue
 require(r < constant1)""",

           """for r in rs:
 require(r > constant2)""",

           """for r in rs:
 if r > 10:
  break
 require(r == constant3)""",

           """for r in rs:
 continue
require(True)""",

           """for r in rs:
 if r > 10:
  continue
 require(r == constant3)"""]

    # None of the loops should merge because their bodies contain branches
    expect = """for r_0 in rs:
 if loop_index(r_0) == 0:
  continue
 require(r_0 < constant1, 0)

for r_1 in rs:
 require(r_1 > constant2, 1)

for r_2 in rs:
 if r_2 > 10:
  break
 require(r_2 == constant3, 2)

for r_3 in rs:
 continue
require(True, 3)

for r_4 in rs:
 if r_4 > 10:
  continue
 require(r_4 == constant3, 4)"""

    runTest(vus, expect)

def test_fors_branch_nested_in_loop():
    vus = ["""for r in rs:
 a = r
 while loop_index(r) == 0:
  a = a + 1
  if a > 0:
   break
 require(a < constant1)""",

           """for r in rs:
 require(r > constant2)""",

           """for r in rs:
 if r > 10:
  break
 require(r == constant3)""",

           """for r in rs:
 for t in ts:
  require(t)
  break
require(True)""",

           """for r in rs:
 if r > 10:
  while False:
   continue
 require(r == constant3)"""]

    # The loops which contain branches inside other loops are ok to merge
    expect = """for r_0 in rs:
 a_1 = r_0
 while loop_index(r_0) == 0:
  a_1 = a_1 + 1
  if a_1 > 0:
   break
 require(a_1 < constant1, 0)
 require(r_0 > constant2, 1)
 for t_5 in ts:
  require(t_5, 3)
  break
 if r_0 > 10:
  while False:
   continue
 require(r_0 == constant3, 4)
require(True, 3)

for r_3 in rs:
 if r_3 > 10:
  break
 require(r_3 == constant3, 2)"""

    runTest(vus, expect)

def test_while():
    vus = ["""a = False
while not a:
 a = not a
require(a)""",

           """b = False
while not b:
 b = b or not b
require(b)""",

           """c = True
while not c:
 c = False
require(c)"""]

    # Not attempt is made to merge while loops
    expect = """a_0 = False
while not a_0:
 a_0 = not a_0
require(a_0, 0)

b_1 = False
while not b_1:
 b_1 = b_1 or not b_1
require(b_1, 1)

c_2 = True
while not c_2:
 c_2 = False
require(c_2, 2)"""

    runTest(vus, expect)
