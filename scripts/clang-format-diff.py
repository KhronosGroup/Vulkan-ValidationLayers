#!/usr/bin/env python3
#
#===- clang-format-diff.py - ClangFormat Diff Reformatter ----*- python -*--===#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#===------------------------------------------------------------------------===#

"""
This script reads input from a unified diff and reformats all the changed
lines. This is useful to reformat all the lines touched by a specific patch.
Example usage for git/svn users:

  git diff -U0 --no-color HEAD^ | clang-format-diff.py -p1 -i
  svn diff --diff-cmd=diff -x-U0 | clang-format-diff.py -i

"""

import argparse
import difflib
import re
import subprocess
import sys

if sys.version_info.major >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO


def main():
  parser = argparse.ArgumentParser(description=__doc__,
                                   formatter_class=
                                           argparse.RawDescriptionHelpFormatter)
  parser.add_argument('-i', action='store_true', default=False,
                      help='apply edits to files instead of displaying a diff')
  parser.add_argument('-p', metavar='NUM', default=0,
                      help='strip the smallest prefix containing P slashes')
  parser.add_argument('-regex', metavar='PATTERN', default=None,
                      help='custom pattern selecting file paths to reformat '
                      '(case sensitive, overrides -iregex)')
  parser.add_argument('-iregex', metavar='PATTERN', default=
                      r'.*\.(cpp|cc|c\+\+|cxx|c|cl|h|hh|hpp|m|mm|inc|js|ts|proto'
                      r'|protodevel|java|cs)',
                      help='custom pattern selecting file paths to reformat '
                      '(case insensitive, overridden by -regex)')
  parser.add_argument('-sort-includes', action='store_true', default=False,
                      help='let clang-format sort include blocks')
  parser.add_argument('-v', '--verbose', action='store_true',
                      help='be more verbose, ineffective without -i')
  parser.add_argument('-style',
                      help='formatting style to apply (LLVM, Google, Chromium, '
                      'Mozilla, WebKit)')
  parser.add_argument('-binary', default='clang-format',
                      help='location of binary to use for clang-format')
  args = parser.parse_args()

  # Extract changed lines for each file.
  filename = None
  lines_by_file = {}
  for line in sys.stdin:
    match = re.search(r'^\+\+\+\ (.*?/){%s}(\S*)' % args.p, line)
    if match:
      filename = match.group(2)
    if filename is None:
      continue

    if args.regex is not None:
      if not re.match('^%s$' % args.regex, filename):
        continue
    else:
      if not re.match('^%s$' % args.iregex, filename, re.IGNORECASE):
        continue

    match = re.search(r'^@@.*\+(\d+)(,(\d+))?', line)
    if match:
      start_line = int(match.group(1))
      line_count = 1
      if match.group(3):
        line_count = int(match.group(3))
      if line_count == 0:
        continue
      end_line = start_line + line_count - 1
      lines_by_file.setdefault(filename, []).extend(
          ['-lines', str(start_line) + ':' + str(end_line)])

  # Reformat files containing changes in place.
  for filename, lines in lines_by_file.items():
    if args.i and args.verbose:
      print('Formatting {}'.format(filename))

    command = [args.binary, filename]
    # We no longer pass '-i' to clang-format directly.
    # We need to intercept the stdout to filter out whitespace-only changes.
    if args.sort_includes:
      command.append('-sort-includes')
    command.extend(lines)
    if args.style:
      command.extend(['-style', args.style])

    p = subprocess.Popen(command,
                         stdout=subprocess.PIPE,
                         stderr=None,
                         stdin=subprocess.PIPE,
                         universal_newlines=True)
    stdout, stderr = p.communicate()
    if p.returncode != 0:
      sys.exit(p.returncode)

    with open(filename) as f:
      code = f.readlines()
    formatted_code = StringIO(stdout).readlines()

    # From Clang-Format 18 to 20+ there were some dumb changes, things like
    #
    # const char* x;
    # const char *x;
    #
    # were getting triggered as errors and failing CI
    # We want clang-format for general formatting, but this nit-pick level
    # is dumb to flag an error for what clearing seems to be a clang-format regression
    #
    # Filter out 1:1 line changes where the only difference is whitespace
    # directly adjacent to an asterisk (*) or ampersand (&).
    adjusted_formatted_code = []
    sm = difflib.SequenceMatcher(None, code, formatted_code)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
      if tag == 'equal':
        adjusted_formatted_code.extend(code[i1:i2])
      elif tag == 'replace' and (i2 - i1) == (j2 - j1):
        for original, formatted in zip(code[i1:i2], formatted_code[j1:j2]):
          # Strip spaces around * and & using regex, then compare.
          # r'\s*([*&])\s*' matches any whitespace, an asterisk or ampersand, and any trailing whitespace.
          # r'\1' replaces it with just the captured character (* or &).
          norm_orig = re.sub(r'\s*([*&])\s*', r'\1', original)
          norm_fmt = re.sub(r'\s*([*&])\s*', r'\1', formatted)

          if norm_orig == norm_fmt:
            adjusted_formatted_code.append(original)
          else:
            adjusted_formatted_code.append(formatted)
      else:
        # Non 1:1 replacement (e.g. line wrapping or breaking), accept formatting
        adjusted_formatted_code.extend(formatted_code[j1:j2])

    if args.i:
      # If in-place mode is on, write back to file ONLY if there are actual changes left
      if code != adjusted_formatted_code:
        with open(filename, 'w') as f:
          f.writelines(adjusted_formatted_code)
    else:
      # Otherwise, print the unified diff based on the adjusted code
      diff = difflib.unified_diff(code, adjusted_formatted_code,
                                  filename, filename,
                                  '(before formatting)', '(after formatting)')
      diff_string = ''.join(diff)
      if len(diff_string) > 0:
        sys.stdout.write(diff_string)

if __name__ == '__main__':
  main()