/*
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <memory>
#include <cstring>
#include <chrono>

#include "module.h"

static constexpr uint32_t kDefaultShaderId = 23;
static constexpr uint32_t kInstDefaultDescriptorSet = 3;

static bool timer = false;
static bool all_passes = false;
static bool bindless_descriptor_pass = false;
static bool buffer_device_address_pass = false;
static bool ray_query_pass = false;

void PrintUsage(const char* program) {
    printf(R"(
%s - Test the SPIR-V Instrumentation used for GPU-AV

USAGE: %s <input> -o <output> <passes>
)",
           program, program);

    printf(R"(
  --all-passes
               Runs all passes together
  --bindless-descriptor
               Runs BindlessDescriptorPass
  --buffer-device-address
               Runs BufferDeviceAddressPass
  --ray-query
               Runs RayQueryPass
  --timer
               Prints time it takes to instrument entire module
  -h, --help
               Print this help)");
    printf("\n");
}

bool ParseFlags(int argc, char** argv, const char** out_file) {
    std::vector<std::string> pass_flags;
    for (int argi = 1; argi < argc; ++argi) {
        const char* cur_arg = argv[argi];
        if (0 == strcmp(cur_arg, "--help") || 0 == strcmp(cur_arg, "-h")) {
            PrintUsage(argv[0]);
            return false;
        } else if (0 == strcmp(cur_arg, "-o")) {
            if (!*out_file && argi + 1 < argc) {
                *out_file = argv[++argi];
            } else {
                PrintUsage(argv[0]);
                return false;
            }
        } else if (0 == strcmp(cur_arg, "--timer")) {
            timer = true;
        } else if (0 == strcmp(cur_arg, "--all-passes")) {
            all_passes = true;
        } else if (0 == strcmp(cur_arg, "--bindless-descriptor")) {
            bindless_descriptor_pass = true;
        } else if (0 == strcmp(cur_arg, "--buffer-device-address")) {
            buffer_device_address_pass = true;
        } else if (0 == strcmp(cur_arg, "--ray-query")) {
            ray_query_pass = true;
        } else if (0 == strncmp(cur_arg, "--", 2)) {
            printf("Unknown pass %s\n", cur_arg);
            PrintUsage(argv[0]);
            return false;
        }
    }

    return true;  // valid
}

int main(int argc, char** argv) {
    if (argc < 5) {
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    } else if (!std::filesystem::exists(argv[1])) {
        std::cout << "ERROR: " << argv[1] << " Does not exists\n";
        return EXIT_FAILURE;
    }

    const char* out_file = nullptr;
    if (!ParseFlags(argc, argv, &out_file)) {
        return EXIT_FAILURE;
    }

    if (out_file == nullptr) {
        std::cout << "ERROR: output file is required ( -o )";
        return EXIT_FAILURE;
    }

    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        std::cout << "ERROR: Unable to open the input file " << argv[1] << "\n";
        return EXIT_FAILURE;
    }

    std::vector<uint32_t> spirv_data;
    const int buf_size = 1024;
    uint32_t buf[buf_size];
    while (size_t len = fread(buf, sizeof(uint32_t), buf_size, fp)) {
        spirv_data.insert(spirv_data.end(), buf, buf + len);
    }
    fclose(fp);

    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    if (timer) {
        start_time = std::chrono::high_resolution_clock::now();
    }

    gpuav::spirv::Module module(spirv_data, kDefaultShaderId, kInstDefaultDescriptorSet);
    if (all_passes || bindless_descriptor_pass) {
        module.RunPassBindlessDescriptor();
    }
    if (all_passes || buffer_device_address_pass) {
        module.RunPassBufferDeviceAddress();
    }
    if (all_passes || ray_query_pass) {
        module.RunPassRayQuery();
    }

    for (const auto info : module.link_info_) {
        module.LinkFunction(info);
    }
    module.ToBinary(spirv_data);

    if (timer) {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        std::cout << "Time = " << duration.count() << "ms\n";
    }

    fp = fopen(out_file, "wb");
    if (!fp) {
        std::cout << "ERROR: Unable to open the output file " << out_file << "\n";
        return EXIT_FAILURE;
    }

    fwrite(spirv_data.data(), sizeof(uint32_t), spirv_data.size(), fp);
    fclose(fp);

    return 0;
}