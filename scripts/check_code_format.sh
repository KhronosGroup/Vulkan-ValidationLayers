#!/bin/bash
# Copyright (c) 2017-2019 Google Inc.
# Copyright (c) 2019 LunarG, Inc.

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
# Script to determine if source code in Pull Request is properly formatted.
# Exits with non 0 exit code if formatting is needed.
#
# This script assumes to be invoked at the project root directory.

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
FOUND_ERROR=0

FILES_TO_CHECK=$(git diff --name-only master | grep -v -E "^include/vulkan" | grep -E ".*\.(cpp|cc|c\+\+|cxx|c|h|hpp)$")
COPYRIGHTED_FILES_TO_CHECK=$(git diff --name-only master | grep -v -E "^include/vulkan")

if [ -z "${FILES_TO_CHECK}" ]; then
  echo -e "${GREEN}No source code to check for formatting.${NC}"
else
  # Check source files in PR for clang-format errors
  FORMAT_DIFF=$(git diff -U0 master -- ${FILES_TO_CHECK} | python ./scripts/clang-format-diff.py -p1 -style=file)

  if [ ! -z "${FORMAT_DIFF}" ]; then
    echo -e "${RED}Found formatting errors!${NC}"
    echo "${FORMAT_DIFF}"
    FOUND_ERROR=1
  fi

  # Check files in PR out-of-date copyright notices
  if [ -z "${COPYRIGHTED_FILES_TO_CHECK}" ]; then
    echo -e "${GREEN}No source code to check for copyright dates.${NC}"
  else
    THISYEAR=$(date +"%Y")
    # Look for current year in copyright lines
    for AFILE in ${COPYRIGHTED_FILES_TO_CHECK}
    do
      COPYRIGHT_INFO=$(cat ${AFILE} | grep -E "Copyright (.)*LunarG")
      if [ ! -z "${COPYRIGHT_INFO}" ]; then
        if ! echo "$COPYRIGHT_INFO" | grep -q "$THISYEAR" ; then
          echo -e "${RED} "$AFILE" has an out-of-date copyright notice.${NC}"
          FOUND_ERROR=1
        fi
      fi
    done
  fi
fi

if [ $FOUND_ERROR  -gt 0 ]; then
  exit 1
else
  echo -e "${GREEN}All source code in PR properly formatted.${NC}"
  exit 0
fi
