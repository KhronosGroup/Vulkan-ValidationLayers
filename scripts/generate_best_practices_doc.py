#!/usr/bin/env python3
#
# Copyright (c) 2026 LunarG, Inc.
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
# Scans the Best Practices source code and generates docs/best_practices_checks.md,
# the list of all Best Practices checks with their messages.

import argparse
import glob
import os
import re
import sys
import common_ci

SOURCE_DIR = common_ci.RepoRelative('layers/best_practices')
OUTPUT_FILE = common_ci.RepoRelative('docs/best_practices_checks.md')

ID_PREFIX = 'BestPractices-'
LOG_FUNCTIONS = {
    'LogError': 'Error',
    'LogWarning': 'Warning',
    'LogPerformanceWarning': 'Performance',
    'LogInfo': 'Info',
    'LogVerbose': 'Verbose',
}
VENDORS = {
    'kBPVendorAMD': 'AMD',
    'kBPVendorArm': 'Arm',
    'kBPVendorIMG': 'IMG',
    'kBPVendorNVIDIA': 'NVIDIA',
}

Token = tuple[str, str]  # (kind, value)

TOKEN_RE = re.compile(r'''
    (?P<comment>//[^\n]*|/\*.*?\*/)
  | (?P<preprocessor>^[ \t]*\#[^\n]*)
  | (?P<string>"(?:\\.|[^"\\])*")
  | (?P<char>'(?:\\.|[^'\\])*')
  | (?P<ident>[A-Za-z_]\w*)
  | (?P<number>\d[\w.]*)
  | (?P<punct>::|->|\S)
''', re.VERBOSE | re.DOTALL | re.MULTILINE)

PLACEHOLDER_RE = re.compile(r'%[-+ #0]*\*?\d*(?:\.\*?\d+)?(?:hh|h|ll|l|z|j|t|L)?(?:u32|u64|x32|x64|d32|d64|[diouxXeEfgGcsp])')

class Check:
    def __init__(self, bpid: str):
        self.bpid = bpid
        self.types = set()
        self.vendors = set()
        self.messages = []

def Tokenize(text: str) -> list[Token]:
    tokens = []
    for match in TOKEN_RE.finditer(text):
        if match.lastgroup not in ('comment', 'preprocessor'):
            tokens.append((match.lastgroup, match.group(match.lastgroup)))
    return tokens

# Concatenates adjacent string literals, where "%" PRIu32 becomes %u32
# Returns None if the tokens are not only made of string literals
def ConcatStrings(tokens: list[Token]) -> str | None:
    text = ''
    for kind, value in tokens:
        if kind == 'string':
            text += value[1:-1].replace('\\"', '"').replace('\\n', ' ').replace('\\\\', '\\')
        elif kind == 'ident' and value.startswith('PRI'):
            text += value[3:]
        else:
            return None
    return text if tokens else None

# Splits the arguments of the call whose '(' is at tokens[start]
def SplitArguments(tokens: list[Token], start: int) -> list[list[Token]]:
    args = [[]]
    depth = 0
    for token in tokens[start:]:
        value = token[1]
        if value in ('(', '[', '{'):
            depth += 1
            if depth == 1:
                continue
        elif value in (')', ']', '}'):
            depth -= 1
            if depth == 0:
                break
        elif value == ',' and depth == 1:
            args.append([])
            continue
        args[-1].append(token)
    return args

# Returns the string literals assigned to the variable in tokens[begin:end]
def ResolveVariable(tokens: list[Token], begin: int, end: int, name: str) -> list[str]:
    values = []
    for i in range(begin, end - 2):
        if tokens[i] == ('ident', name) and tokens[i + 1][1] == '=':
            j = i + 2
            while j < end and (tokens[j][0] == 'string' or tokens[j][1].startswith('PRI')):
                j += 1
            value = ConcatStrings(tokens[i + 2:j])
            if value:
                values.append(value)
    return values

# Returns the possible values of a string argument, which can be a variable
def ResolveString(tokens: list[Token], begin: int, end: int, arg: list[Token]) -> list[str]:
    value = ConcatStrings(arg)
    if value is not None:
        return [value]
    if len(arg) == 1 and arg[0][0] == 'ident':
        return ResolveVariable(tokens, begin, end, arg[0][1])
    return []

# Turns error_obj.location.dot(Field::pCreateInfo).dot(Field::pStages, i) into pCreateInfo.pStages[{}]
def LocationFields(arg: list[Token]) -> str:
    fields = ''
    values = [value for _, value in arg]
    for i, value in enumerate(values):
        if value == 'Field' and i + 2 < len(values) and values[i + 1] == '::':
            fields += f'.{values[i + 2]}'
            if i + 3 < len(values) and values[i + 3] == ',':
                fields += '[{}]'
        elif value == 'pNext' and i + 4 < len(values) and values[i + 1] == '(' and values[i + 2] == 'Struct':
            fields += f'.pNext<{values[i + 4]}>'
    return fields.lstrip('.')

def CleanMessage(message: str, vendor_tags: int, location: str) -> str:
    message = message.replace('%%', '\0')
    # Vendor tags are always printed first with a %s
    for _ in range(vendor_tags):
        message = re.sub(r'^\s*%s\s*', '', message, count=1)
    message = PLACEHOLDER_RE.sub('{}', message).replace('\0', '%')
    message = ' '.join(message.split()).lstrip(': ')
    # The message continues the location when it does not start a sentence
    if location and not message[:1].isupper():
        message = f'{location} {message}'
    return message

def ParseFile(filename: str, checks: dict[str, Check]) -> None:
    with open(filename, 'r', encoding='utf-8') as file:
        tokens = Tokenize(file.read())

    depth = 0
    body_start = 0  # start of the current top level function
    for i, (kind, value) in enumerate(tokens):
        if value == '{':
            if depth == 0:
                body_start = i
            depth += 1
        elif value == '}':
            depth -= 1
        elif kind == 'ident' and value in LOG_FUNCTIONS and i + 1 < len(tokens) and tokens[i + 1][1] == '(':
            args = SplitArguments(tokens, i + 1)
            if len(args) < 4:
                continue  # declaration
            bpids = [x for x in ResolveString(tokens, body_start, i, args[0]) if x.startswith(ID_PREFIX)]
            if not bpids:
                continue

            vendors = set()
            vendor_tags = 0
            for arg in args[4:]:
                if arg and arg[0] == ('ident', 'VendorSpecificTag'):
                    vendor_tags += 1
                vendors.update(VENDORS[x] for _, x in arg if x in VENDORS)

            location = LocationFields(args[2])
            messages = [CleanMessage(x, vendor_tags, location) for x in ResolveString(tokens, body_start, i, args[3])]

            for bpid in bpids:
                check = checks.setdefault(bpid, Check(bpid))
                check.types.add(LOG_FUNCTIONS[value])
                check.vendors.update(vendors)
                check.vendors.update(x for x in VENDORS.values() if bpid.startswith(f'{ID_PREFIX}{x}-'))
                check.messages += [x for x in messages if x not in check.messages]

def Escape(text: str) -> str:
    return text.replace('|', '\\|').replace('<', '&lt;').replace('>', '&gt;')

def GenerateDoc(checks: dict[str, Check]) -> str:
    out = []
    out.append('<!-- markdownlint-disable MD041 -->\n')
    out.append('<!-- Copyright 2026 LunarG, Inc. -->\n')
    out.append('<!-- Generated by scripts/generate_best_practices_doc.py, do not edit -->\n')
    out.append('\n')
    out.append('# Best Practices Checks\n')
    out.append('\n')
    out.append('All the checks done by [Best Practices Validation](best_practices.md), with the ID found in their message.\n')
    out.append('`{}` is a value that is only known when the message is printed.\n')
    out.append('\n')
    out.append('Vendor specific checks are only done when the vendor is enabled in the settings, '
               'their message starts with the vendor tag (such as `[Arm]`).\n')

    sections = [('Common', set())] + [(x, {x}) for x in sorted(VENDORS.values(), key=str.lower)]
    for title, vendors in sections:
        out.append('\n')
        out.append(f'## {title}\n')
        out.append('\n')
        out.append('| ID | Type | Message |\n')
        out.append('| -- | ---- | ------- |\n')
        for check in sorted(checks.values(), key=lambda x: x.bpid.lower()):
            if vendors <= check.vendors and (vendors or not check.vendors):
                types = ', '.join(sorted(check.types))
                messages = '<br>'.join(Escape(x) for x in check.messages)
                out.append(f'| `{check.bpid}` | {types} | {messages} |\n')
    return ''.join(out)

def GenerateBestPracticesDoc() -> str:
    checks = {}
    for filename in sorted(glob.glob(os.path.join(SOURCE_DIR, '*.cpp')) + glob.glob(os.path.join(SOURCE_DIR, '*.h'))):
        ParseFile(filename, checks)
    return GenerateDoc(checks)

# Also called by generate_source.py --verify
def VerifyBestPracticesDoc() -> bool:
    with open(OUTPUT_FILE, 'r', encoding='utf-8') as file:
        if file.read() == GenerateBestPracticesDoc():
            return True
    print(f'ERROR: {OUTPUT_FILE} is out of date, run scripts/generate_best_practices_doc.py')
    return False

def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description='Generate docs/best_practices_checks.md from the Best Practices source code')
    parser.add_argument('--verify', action='store_true', help='Verify the checked in file is up to date')
    args = parser.parse_args(argv)

    if args.verify:
        return 0 if VerifyBestPracticesDoc() else 1

    with open(OUTPUT_FILE, 'w', encoding='utf-8', newline='\n') as file:
        file.write(GenerateBestPracticesDoc())
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
