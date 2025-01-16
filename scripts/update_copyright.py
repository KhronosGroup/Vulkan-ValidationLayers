#!/usr/bin/env python3

# Copyright (c) 2025 Valve Corporation
# Copyright (c) 2025 LunarG, Inc.

import argparse
import os
import shutil
from datetime import date
import re

SEARCH_AT_START = [
    "layers",
    "tests",
    "docs",
]

SEARCH_ALL_FILE = [
    "scripts"
]

# Mainly files at root level
STAND_ALONE_FILES = [
    "BUILD.gn",
    "CMakeLists.txt",
]

CURRENT_YEAR = int(date.today().year)
# We need the space at the end so it doesn't mess with the scripts (praying we don't leave spaces in code after)
COPYRIGHT_PATTERN = re.compile("Copyright .*(\\d{4}) ")
FILE_PATTERN = re.compile(".*\\.(cpp|cc|c++|cxx|c|h|hpp|md|py|txt|gn|gni|sh)$")

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("repository_location", type=str, help="Root directory of the Vulkan-ValidationLayers repository")
    parser.add_argument("--generate-sources", type=str, metavar="BASE_DIRECTORY", help="Path to the base directory containing the Vulkan-Headers and SPIRV-Headers repositories")
    return parser.parse_args()

def find_outdated_copyright(file_segment : str, offset : int = 0):
    copyright_match = COPYRIGHT_PATTERN.search(file_segment, offset)
    if copyright_match:
        return copyright_match
    return None


def replace_copyright(file_segment : str):
    location = 0
    match = find_outdated_copyright(file_segment)
    replaced = ""
    while match:
        copyright_year = match.group(1)
        if CURRENT_YEAR > int(copyright_year):
            # Naive way to understand if we have a year range like 2020-2024
            year_start = match.start(1)
            replaced += (file_segment[location:year_start])
            if file_segment[year_start - 1] != "-":
                replaced += copyright_year + "-"
            replaced += str(CURRENT_YEAR)
        else:
            replaced += file_segment[location:match.end(1)]
        location = match.end(1)
        match = find_outdated_copyright(file_segment, location)

    # Write remainder of the file segment
    replaced += file_segment[location:]

    return replaced

def copy_file_with_updated_header_copyright(path : str):
    source_file = open(path, encoding="utf-8", errors='ignore')
    file_header = source_file.read(1024)
    replacement = replace_copyright(file_header)
    if replacement != "":
        replacement_file_path = path + "_temp"
        replacement_file = open(replacement_file_path, mode="w", encoding="utf-8", errors="ignore")
        replacement_file.write(replacement)

        chunk = source_file.read(1024)
        while chunk != "":
            replacement_file.write(chunk)
            chunk = source_file.read(1024)

        return replacement_file_path

    return None

def copy_file_with_updated_all_copyright(path : str):
    source_file = open(path, encoding="utf-8", errors='ignore')
    replacement_file_path = path + "_temp"
    # Hack to keep file permissions the same, otherwise it'll show up in changes even if we didn't modify the copyright
    shutil.copy(path, replacement_file_path)
    replacement_file = open(replacement_file_path, mode="w", encoding="utf-8", errors="ignore")
    chunk = source_file.read(4096)
    remainder = ""
    while chunk != "":
        # We may partition a line halfway where a copyright notice happens which leads to incorrect checking
        # Just do the check next interation for that partitioned line
        # Assuming lines are less than 4096 which should be a fair assumption
        last_newline = chunk.rfind('\n')
        remainder += chunk[:last_newline]
        replacement = replace_copyright(remainder)
        replacement_file.write(replacement)
        remainder = chunk[last_newline:]
        chunk = source_file.read(4096)

    # We may have something left
    replacement = replace_copyright(remainder)
    replacement_file.write(replacement)

    return replacement_file_path

def update_directory(directory : str, copy_file_with_updated_copyright):
    for root, children_directories, files in os.walk(directory):
        for file in files:
            if FILE_PATTERN.search(file):
                file_path = os.path.join(root, file)
                copied_file = copy_file_with_updated_copyright(file_path)
                if copied_file:
                    shutil.move(copied_file, file_path)

def main():
    args = parse_arguments()
    absolute_path = os.path.abspath(args.repository_location)

    for search_directory in SEARCH_AT_START:
        base_directory = os.path.join(absolute_path, search_directory)
        update_directory(base_directory, copy_file_with_updated_header_copyright)

    for search_directory in SEARCH_ALL_FILE:
        base_directory = os.path.join(absolute_path, search_directory)
        update_directory(base_directory, copy_file_with_updated_all_copyright)

    for file in STAND_ALONE_FILES:
        if FILE_PATTERN.search(file):
            file_path = os.path.join(absolute_path, file)
            copied_file = copy_file_with_updated_header_copyright(file_path)
            if copied_file:
                shutil.move(copied_file, file_path)


if __name__ == '__main__':
    main()
