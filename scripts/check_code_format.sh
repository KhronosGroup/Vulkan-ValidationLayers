#!/bin/bash
# Copyright (c) 2017-2020 Google Inc.
# Copyright (c) 2020 LunarG, Inc.

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
# Author: Karl Schultz <karl@lunarg.com>
# Author: Mark Lobodzinski <mark@lunarg.com>

# Script to determine if source code in Pull Request is properly formatted.
# Exits with non 0 exit code if formatting is needed.
#
# This script assumes to be invoked at the project root directory.
#
# This script checks for:
#   -- clang-format errors in the PR souurce code
#   -- out-of-date copyrights in PR source files
#   -- improperly formatted commit messages (using the function above)


# Define ASCII text output colors
HELP_MSG='\033[0;36m'
SUCCESS_MSG='\033[0;32m'
CONTENT='\033[0;33m'
ERR_MSG='\033[0;31m'
NO_COLOR='\033[0m'

FOUND_ERROR=0
FILES_TO_CHECK=$(git diff --name-only master | grep -v -E "^include/vulkan" | grep -E ".*\.(cpp|cc|c\+\+|cxx|c|h|hpp)$")
COPYRIGHTED_FILES_TO_CHECK=$(git diff --name-only master | grep -v -E "^include/vulkan")

# Check files in PR for clang-format errors
echo -e ""
echo -e "Checking PR source code for clang-format errors:"
echo -e ""

if [ -z "${FILES_TO_CHECK}" ]; then
  echo -e "${SUCCESS_MSG}No source code to check for clang-formatting.${NO_COLOR}"
else
  if test -f "./scripts/clang-format-diff.py"; then
    FORMAT_DIFF=$(git diff -U0 master -- ${FILES_TO_CHECK} | python ./scripts/clang-format-diff.py -p1 -style=file)
    if [ ! -z "${FORMAT_DIFF}" ]; then
      echo -e "${ERR_MSG}Found formatting errors!${NO_COLOR}"
      echo "${FORMAT_DIFF}"
      FOUND_ERROR=1
    fi
  else
    echo -e "${SUCCESS_MSG}Missing /scripts/clang-format-diff.py, cannot run clang-format checks."
    echo -e "The missing script can be installed by running this command in the root of the repository:"
    echo -e "${CONTENT}curl -L http://llvm.org/svn/llvm-project/cfe/trunk/tools/clang-format/clang-format-diff.py -o scripts/clang-format-diff.py${NO_COLOR};"
  fi
fi

# Check files in PR for out-of-date copyright notices
echo -e ""
echo -e "Checking PR source files for correct copyright information:"
echo -e ""

if [ -z "${COPYRIGHTED_FILES_TO_CHECK}" ]; then
  echo -e "${SUCCESS_MSG}No source code to check for copyright dates.${NO_COLOR}"
else
  THISYEAR=$(date +"%Y")
  for AFILE in ${COPYRIGHTED_FILES_TO_CHECK}
  do
    COPYRIGHT_INFO=$(cat ${AFILE} | grep -E "Copyright (.)*LunarG")
    if [ ! -z "${COPYRIGHT_INFO}" ]; then
      if ! echo "$COPYRIGHT_INFO" | grep -q "$THISYEAR" ; then
        echo -e "${ERR_MSG}"$AFILE" has an out-of-date copyright notice.${NO_COLOR}"
        FOUND_ERROR=1
      fi
    fi
  done
fi

# Check the commit messages in the PR for invalid formats
echo -e ""
echo -e "Checking PR commit messages for consistency issues:"
echo -e ""


if [ -z "${TRAVIS_BUILD_DIR}" ]; then
  PR_COMMIT_RANGE="--no-merges --left-only HEAD...master"
else
  # TRAVIS_COMMIT_RANGE contains range of commits for this PR
  PR_COMMIT_RANGE="${TRAVIS_COMMIT_RANGE/.../..}"
fi

# Get user-supplied commit message text for applicable commits and insert a unique separator string identifier. The
# git command returns ONLY the subject line and body for each of the commits.
COMMIT_TEXT=$(git log ${PR_COMMIT_RANGE} --pretty=format:"XXXNEWLINEXXX"%n%B)

# Return if there is no content
if [[ -z "${COMMIT_TEXT}" || -z "${PR_COMMIT_RANGE}" ]]; then
  echo -e "${ERR_MSG}No commit messages to check for formatting.${NO_COLOR}"
  echo -e ""
else
  # Process commit messages
  MSG_SUCCESS=1
  MSG_CUR_LINE=0
  MSG_PREV_LINE=""

  # Process each line of the commit message output, resetting counter on separator
  printf %s "$COMMIT_TEXT" | while IFS='' read -r MSG_LINE_TEXT; do
    MSG_CUR_LINE=$((MSG_CUR_LINE+1))
    if [ "$MSG_LINE_TEXT" = "XXXNEWLINEXXX" ]; then
      MSG_CUR_LINE=0
    fi

    LINE_LENGTH=${#MSG_LINE_TEXT}
    if [ $MSG_CUR_LINE -eq 1 ]; then
      # Enforce subject line must be 64 chars or less
      if [ $LINE_LENGTH -gt 64 ]; then
        echo -e "${ERR_MSG}The following subject line exceeds 64 characters in length.${NO_COLOR}"
        echo -e "${CONTENT}     '$MSG_LINE_TEXT'${NO_COLOR}\n"
        MSG_SUCCESS=0
      fi
      LAST_POS=$(($LINE_LENGTH-1))
      LAST_CHAR=${MSG_LINE_TEXT:$LAST_POS:1}
      # Output error if last char of subject line is not alpha-numeric
      if [[ LAST_CHAR =~ [.,] ]]; then
        echo -e "${ERR_MSG}For the following commit, the last character of the subject line must not be a period or comma.${NO_COLOR}"
        echo -e "${CONTENT}     '$MSG_LINE_TEXT'${NO_COLOR}\n"
        MSG_SUCCESS=0
      fi
      # Ignore 'Revert' commits
      SUBJECT_PREFIX=$(echo $MSG_LINE_TEXT | cut -f1 -d " ")
      if [ "${SUBJECT_PREFIX}" != "Revert" ]; then
        # Output error if subject line doesn't start with 'module: '
        if [ "${SUBJECT_PREFIX: -1}" != ":" ]; then
          echo -e "${ERR_MSG}The following subject line must start with a single word specifying the functional area of the change, followed by a colon and space.${NO_COLOR}"
          echo -e "${ERR_MSG}e.g., 'layers: Subject line here' or 'corechecks: Fix off-by-one error in ValidateFences'${NO_COLOR}"
          echo -e "${ERR_MSG}Other common module names include layers, build, cmake, tests, docs, scirpts, stateless, gpu, syncval, practices, etc.${NO_COLOR}"
          echo -e "${CONTENT}     '$MSG_LINE_TEXT'${NO_COLOR}\n"
          MSG_SUCCESS=0
        fi
        # Check that first character of subject line is not capitalized
        FIRST_CHAR=$(echo ${MSG_LINE_TEXT} | cut -c 1)
        if [[ "${FIRST_CHAR}" =~ [A-Z] ]]; then
          echo -e "${ERR_MSG}The first word of the subject line must be lower case.${NO_COLOR}"
          echo -e "${CONTENT}     '$MSG_LINE_TEXT'${NO_COLOR}\n"
          MSG_SUCCESS=0
        fi
        # Check if first character after the colon is lower-case
        SUBJECT_BODY=$(echo $MSG_LINE_TEXT | cut -f2 -d " ")
        FIRST_CHAR=$(echo ${SUBJECT_BODY} | cut -c 1)
        if [[ "${FIRST_CHAR}" =~ [a-z] ]]; then
          echo -e "${ERR_MSG}The first word of the subject line after the ':' character must be capitalized.${NO_COLOR}"
          echo -e "${CONTENT}     '$MSG_LINE_TEXT'${NO_COLOR}\n"
          MSG_SUCCESS=0
        fi
      fi
    elif [ $MSG_CUR_LINE -eq 2 ]; then
      # Commit message must have a blank line between subject and body
      if [ $LINE_LENGTH -ne 0 ]; then
        echo -e "${ERR_MSG}The following subject line must be followed by a blank line.${NO_COLOR}"
        echo -e "${CONTENT}     '$MSG_PREV_LINE'${NO_COLOR}\n"
        MSG_SUCCESS=0
      fi
    else
      # Lines in a commit message body must be less than 72 characters in length (but give some slack)
      if [ $LINE_LENGTH -gt 76 ]; then
        echo -e "${ERR_MSG}The following commit message body line exceeds the 72 character limit.${NO_COLOR}"
        echo -e "${CONTENT}     '$MSG_LINE_TEXT'${NO_COLOR}\n"
        MSG_SUCCESS=0
      fi
    fi
    MSG_PREV_LINE=$MSG_LINE_TEXT
  done
  
  if [ $MSG_SUCCESS -eq 1 ]; then
    echo -e "${GREEN}All commit messages in pull request are properly formatted.${NO_COLOR}"
  else
    echo -e "${HELP_MSG}"
    echo -e "Commit Message Format Requirements:"
    echo -e "-----------------------------------"
    echo -e 'o  Subject lines must be <= 64 characters in length'
    echo -e 'o  Subject lines must start with a module keyword which is lower-case and followed by a colon and a space'
    echo -e "o  The first word following the colon must be capitalized and the subject line must not end in a '.'"
    echo -e 'o  The subject line must be followed by a blank line'
    echo -e 'o  The commit description must be <= 72 characters in width'
    echo -e ''
    echo -e 'Examples:'
    echo -e '---------'
    echo -e '     build: Fix Vulkan header/registry detection for SDK'
    echo -e '     tests: Fix QueryPerformanceIncompletePasses stride usage'
    echo -e '     corechecks: Fix validation of VU 03227'
    echo -e "     state_tracker: Remove 'using std::*' statements"
    echo -e '     stateless: Account for DynStateWithCount for multiViewport'
    echo -e ''
    echo -e "Refer to this document for additional detail:" 
    echo -e "https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/CONTRIBUTING.md#coding-conventions-and-formatting"
    echo -e "${NO_COLOR}"
    FOUND_ERROR=1
  fi
fi

# Terminate script appropriately
if [ $FOUND_ERROR  -gt 0 ]; then
  exit 1
else
  echo -e "${SUCCESS_MSG}All source code in PR properly formatted.${NO_COLOR}"
  exit 0
fi
