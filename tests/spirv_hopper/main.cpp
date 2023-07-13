/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <fstream>
#include <iostream>
#include <filesystem>

#include "spirv_hopper.h"
#include "vulkan_object.h"
#include "glslang/SPIRV/GlslangToSpv.h"

static bool IsValidSPIRV(size_t file_size, const char* spirv_data) {
    if (file_size < 4) {
        return false;
    } else if (0x03 == spirv_data[0] && 0x02 == spirv_data[1] && 0x23 == spirv_data[2] && 0x07 == spirv_data[3]) {
        return true;  // Little Endianness
    } else if (0x07 == spirv_data[0] && 0x23 == spirv_data[1] && 0x02 == spirv_data[2] && 0x03 == spirv_data[3]) {
        return true;  // Big Endianness
    } else {
        return false;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Error:\n\tUsage: ./spirv-hopper [file | directory]\n";
        return EXIT_FAILURE;
    } else if (!std::filesystem::exists(argv[1])) {
        std::cout << "Error: " << argv[1] << " Does not exists\n";
        return EXIT_FAILURE;
    }

    std::vector<std::filesystem::path> file_list;
    std::vector<std::filesystem::path> failed_files;

    if (!std::filesystem::is_directory(argv[1])) {
        file_list.push_back(argv[1]);
    } else {
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(argv[1])) {
            if (std::filesystem::is_regular_file(dir_entry)) {
                file_list.push_back(dir_entry.path());
            }
        }
    }

    const size_t files_list_count = file_list.size();
    size_t files_ran_count = 1;
    std::cout << "Found " << files_list_count << " files\n";

    // Main execution loop
    {
        // Single VkInstance scope for every shader
        VulkanInstance vk;
        glslang::InitializeProcess();

        for (const auto& file : file_list) {
            std::ifstream spirv_file(file, std::ios::binary);
            if (!spirv_file.good()) {
                std::cout << "Error: Unable to open the file " << file << "\n";
                failed_files.push_back(file);
                continue;
            }
            spirv_file.seekg(0, spirv_file.end);
            const size_t file_size = static_cast<size_t>(spirv_file.tellg());
            spirv_file.seekg(0, spirv_file.beg);

            std::vector<char> spirv_data(file_size);
            spirv_file.read(spirv_data.data(), static_cast<std::streamsize>(file_size));
            spirv_file.close();

            if (!IsValidSPIRV(file_size, spirv_data.data())) {
                std::cout << "Warning: File " << file << " is not a valid SPIR-V File - skipping\n";
                failed_files.push_back(file);
            } else {
                vk.is_valid = true;
                Hopper hopper(vk, file_size, spirv_data.data());

                std::cout << "[" << files_ran_count++ << "|" << files_list_count << "] Running " << file << "\n";

                if (!hopper.Run() || !vk.is_valid) {
                    failed_files.push_back(file);
                }
            }
        }
        glslang::FinalizeProcess();
    }

    if (!failed_files.empty()) {
        std::cout << "\nRESULT: Failure!\n";
        std::cout << "\nThe following tests failed:\n";
        for (const auto& file : failed_files) {
            std::cout << "\t" << file << "\n";
        }
        return EXIT_FAILURE;
    }

    std::cout << "\nRESULT: Success!\n";
}