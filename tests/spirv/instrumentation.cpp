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
#include <cstring>
#include <chrono>

#include "module.h"

static constexpr uint32_t kDefaultShaderId = 23;
static constexpr uint32_t kInstDefaultDescriptorSet = 3;

static bool timer = false;
static bool print_debug_info = false;
static bool all_passes = false;
static bool bindless_descriptor_pass = false;
static bool non_bindless_oob_buffer_pass = false;
static bool non_bindless_oob_texel_buffer_pass = false;
static bool buffer_device_address_pass = false;
static bool ray_query_pass = false;
static bool debug_printf_pass = false;

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
  --non-bindless-oob-buffer
               Runs NonBindlessOOBBufferPass
  --non-bindless-oob-texel-buffer
               Runs NonBindlessOOBTexelBufferPass
  --buffer-device-address
               Runs BufferDeviceAddressPass
  --ray-query
               Runs RayQueryPass
  --debug-printf
               Runs DebugPrintfPass
  --timer
               Prints time it takes to instrument entire module
  --print-debug-info
               Prints debug info for each pass
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
        } else if (0 == strcmp(cur_arg, "--print-debug-info")) {
            print_debug_info = true;
        } else if (0 == strcmp(cur_arg, "--all-passes")) {
            all_passes = true;
        } else if (0 == strcmp(cur_arg, "--bindless-descriptor")) {
            bindless_descriptor_pass = true;
        } else if (0 == strcmp(cur_arg, "--non-bindless-oob-buffer")) {
            non_bindless_oob_buffer_pass = true;
        } else if (0 == strcmp(cur_arg, "--non-bindless-oob-texel-buffer")) {
            non_bindless_oob_texel_buffer_pass = true;
        } else if (0 == strcmp(cur_arg, "--buffer-device-address")) {
            buffer_device_address_pass = true;
        } else if (0 == strcmp(cur_arg, "--ray-query")) {
            ray_query_pass = true;
        } else if (0 == strcmp(cur_arg, "--debug-printf")) {
            debug_printf_pass = true;
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
        std::cout << "ERROR: Unable to open the input file " << argv[1] << '\n';
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

    gpu::spirv::Settings module_settings{};
    module_settings.shader_id = kDefaultShaderId;
    module_settings.output_buffer_descriptor_set = kInstDefaultDescriptorSet;
    module_settings.print_debug_info = print_debug_info;
    module_settings.max_instrumented_count = 0;
    module_settings.support_int64 = true;
    module_settings.support_memory_model_device_scope = true;
    // for all passes, test worst case of using bindless
    module_settings.has_bindless_descriptors = all_passes || bindless_descriptor_pass;

    gpu::spirv::Module module(spirv_data, nullptr, module_settings);
    if (all_passes || bindless_descriptor_pass) {
        module.RunPassBindlessDescriptor();
    }
    if (all_passes || non_bindless_oob_buffer_pass) {
        module.RunPassNonBindlessOOBBuffer();
    }
    if (all_passes || non_bindless_oob_texel_buffer_pass) {
        module.RunPassNonBindlessOOBTexelBuffer();
    }
    if (all_passes || buffer_device_address_pass) {
        module.RunPassBufferDeviceAddress();
    }
    if (all_passes || ray_query_pass) {
        module.RunPassRayQuery();
    }
    if (all_passes || debug_printf_pass) {
        module.RunPassDebugPrintf();
    }

    for (const auto& info : module.link_info_) {
        module.LinkFunction(info);
    }

    module.PostProcess();
    module.ToBinary(spirv_data);

    if (timer) {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        std::cout << "Time = " << duration.count() << "ms\n";
    }

    fp = fopen(out_file, "wb");
    if (!fp) {
        std::cout << "ERROR: Unable to open the output file " << out_file << '\n';
        return EXIT_FAILURE;
    }

    fwrite(spirv_data.data(), sizeof(uint32_t), spirv_data.size(), fp);
    fclose(fp);

    return 0;
}
