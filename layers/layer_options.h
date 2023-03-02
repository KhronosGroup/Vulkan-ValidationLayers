/* Copyright (c) 2022-2023 The Khronos Group Inc.
 * Copyright (c) 2022-2023 Valve Corporation
 * Copyright (c) 2022-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "chassis.h"
#include "vk_layer_settings.h"

#define OBJECT_LAYER_NAME "VK_LAYER_KHRONOS_validation"

class Settings {
  private:
    typedef std::vector<std::string> Strings;
    typedef std::vector<std::pair<std::string, int>> List;
    typedef void *(*LAYER_SETTING_LOG_CALLBACK)(const char *layer_key, const char *setting_key, const char *message);

    /*
  public:
    enum DebugAction {
        DEBUG_ACTION_LOG_MSG_BIT = (1 << 0),
        DEBUG_ACTION_CALLBACK_BIT = (1 << 1),
        DEBUG_ACTION_DEBUG_ACTION_BIT = (1 << 2),
        DEBUG_ACTION_BREAK_BIT = (1 << 3)
    };

    enum Report {
        REPORT_INFO_BIT = (1 << 0),
        REPORT_WARN_BIT = (1 << 1),
        REPORT_PERF_BIT = (1 << 2),
        REPORT_ERROR_BIT = (1 << 3),
        REPORT_DEBUG_BIT = (1 << 4)
    };

    enum Locking { GLOBAL = 0, FINE_GRAIN };

    enum ShaderBased { SHADER_BASED_NONE = 0, SHADER_BASED_DEBUG_PRINTF, SHADER_BASED_GPU_ASSISTED };

    enum VMAMode { VMA_LINEAR = 0, VMA_BEST };
*/
  private:
    Settings();

  public:
    static const Settings &Get();

    enum LOCKING_ENUM { LOCKING_FINE_GRAIN = 0, LOCKING_GLOBAL };
    enum SHADER_BASED_ENUM {
        SHADER_BASED_NONE = 0,
        SHADER_BASED_DEBUG_PRINTF,
        SHADER_BASED_GPU_ASSISTED,
    };
    enum BEST_VENDOR_ENUM { BEST_ARM_BIT = (1 << 0), BEST_AMD_BIT = (1 << 1), BEST_IMG_BIT = (1 << 2), BEST_NV_BIT = (1 << 3) };
    enum VMA_MODE_ENUM { GPUAV_VMA_LINEAR = 0, GPUAV_VMA_BEST };

    struct {
        struct Core {
          private:
            bool value;

          public:
            Core::Core() : value(true){};
            bool Get() const { return this->value; }

            struct Locking {
              private:
                LOCKING_ENUM value;

              public:
                Locking::Locking() : value(LOCKING_FINE_GRAIN) {}

                LOCKING_ENUM Get() const { return Settings::Get().area.core.Get() ? this->value : LOCKING_FINE_GRAIN; }
            } locking;

            struct Check_Image_Layout {
              private:
                bool value;

              public:
                Check_Image_Layout() : value(true) {}

                bool Get() const { return this->value && Settings::Get().area.core.Get(); }
            } check_image_layout;

            struct Check_Command_Buffer {
              private:
                bool value;

              public:
                Check_Command_Buffer() : value(true) {}

                bool Get() const { return this->value && Settings::Get().area.core.Get(); }
            } check_command_buffer;

            struct Check_Object_In_Use {
              private:
                bool value;

              public:
                Check_Object_In_Use() : value(true) {}
                bool Get() const { return this->value && Settings::Get().area.core.Get(); }
            } check_object_in_use;

            struct Check_Query {
              private:
                bool value;

              public:
                Check_Query() : value(true) {}
                bool Get() const { return this->value && Settings::Get().area.core.Get(); }
            } check_query;

            struct Check_Shaders {
              private:
                bool value;

              public:
                Check_Shaders() : value(true) {
                    if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "check_shaders")) {
                        this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "check_shaders");
                    }
                }
                bool Get() const { return this->value && Settings::Get().area.core.Get(); }

                struct Check_Shaders_Caching {
                  private:
                    bool value;

                  public:
                    Check_Shaders_Caching() : value(true) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "check_shaders_caching")) {
                            this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "check_shaders_caching");
                        }
                    }
                    bool Get() const {
                        return this->value && Settings::Get().area.core.Get() && 
                               Settings::Get().area.core.check_shaders.check_shaders_caching.Get();
                    }
                } check_shaders_caching;
            } check_shaders;
        } core;

        struct Unique_Handles {
          private:
            bool value;

          public:
            Unique_Handles() : value(true) {
                if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "unique_handles")) {
                    this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "unique_handles");
                }
            }
            bool Get() const { return this->value; }
        } unique_handles;

        struct Object_Lifetime {
          private:
            bool value;

          public:
            Object_Lifetime() : value(true) {
                if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "object_lifetime")) {
                    this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "object_lifetime");
                }
            }
            bool Get() const { return this->value; }
        } object_lifetime;

        struct Stateless_Param {
          private:
            bool value;

          public:
            Stateless_Param() : value(true) {
                if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "stateless_param")) {
                    this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "stateless_param");
                }
            }
            bool Get() const { return this->value; }
        } stateless_param;

        struct Thread_Safety {
          private:
            bool value;

          public:
            Thread_Safety() : value(true) {
                if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "thread_safety")) {
                    this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "thread_safety");
                }
            }
            bool Get() const { return this->value; }
        } thread_safety;

        struct Sync {
          private:
            bool value;

          public:
            Sync() : value(false) {
                if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "sync")) {
                    this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "sync");
                }
            }

            bool Get() const { return this->value; }

            struct Sync_Queue_Submit {
              private:
                bool value;

              public:
                Sync_Queue_Submit() : value(false) {
                    if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "sync_queue_submit")) {
                        this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "sync_queue_submit");
                    }
                }
                bool Get() const { return this->value && Settings::Get().area.sync.Get(); }
            } validate_sync_queue_submit;
        } sync;

        struct ShaderBased {
          private:
            SHADER_BASED_ENUM value;

          public:
            ShaderBased() : value(SHADER_BASED_NONE) {
                if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "shader_based")) {
                    this->value = static_cast<SHADER_BASED_ENUM>(vku::GetLayerSettingInt(OBJECT_LAYER_NAME, "shader_based"));
                }
            }
            SHADER_BASED_ENUM Get() const { return this->value; }

            struct Shader_Based_Debug_Printf {
                struct Debug_Printf_To_Stdout {
                  private:
                    bool value;

                  public:
                    Debug_Printf_To_Stdout() : value(true) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "debug_printf_to_stdout")) {
                            this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "debug_printf_to_stdout");
                        }
                    }
                    bool Get() const { return this->value && Settings::Get().area.shader_based.Get() == SHADER_BASED_DEBUG_PRINTF; }
                } debug_printf_to_stdout;

                struct Debug_Printf_Verbose {
                  private:
                    bool value;

                  public:
                    Debug_Printf_Verbose() : value(true) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "debug_printf_verbose")) {
                            this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "debug_printf_verbose");
                        }
                    }
                    bool Get() const { return this->value && Settings::Get().area.shader_based.Get() == SHADER_BASED_DEBUG_PRINTF; }
                } debug_printf_verbose;

                struct Debug_Printf_Buffer_Size {
                  private:
                    int value;

                  public:
                    Debug_Printf_Buffer_Size() : value(1024) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "debug_printf_buffer_size")) {
                            this->value = vku::GetLayerSettingInt(OBJECT_LAYER_NAME, "debug_printf_buffer_size");
                        }
                    }
                    int Get() const { return Settings::Get().area.shader_based.Get() == SHADER_BASED_DEBUG_PRINTF ? this->value : 1024; }
                } debug_printf_buffer_size;
            } shader_based_debug_printf;

            struct Shader_Based_Gpu_Assisted {
                struct Reserve_Binding_Slot {
                  private:
                    bool value;

                  public:
                    Reserve_Binding_Slot() : value(true) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "reserve_binding_slot")) {
                            this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "reserve_binding_slot");
                        }
                    }
                    bool Get() const { return this->value && Settings::Get().area.shader_based.Get() == SHADER_BASED_GPU_ASSISTED; }
                } reserve_binding_slot;

                struct Vma_Mode {
                  private:
                    VMA_MODE_ENUM value;

                  public:
                    Vma_Mode() : value(GPUAV_VMA_LINEAR) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "vma_mode")) {
                            this->value = static_cast<VMA_MODE_ENUM>(vku::GetLayerSettingInt(OBJECT_LAYER_NAME, "vma_mode"));
                        }
                    }
                    VMA_MODE_ENUM Get() const { return this->value; }
                } vma_mode;

                struct Check_Descriptor_Indexing {
                  private:
                    bool value;

                  public:
                    Check_Descriptor_Indexing() : value(true) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "check_descriptor_indexing")) {
                            this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "check_descriptor_indexing");
                        }
                    }
                    bool Get() const { return this->value && Settings::Get().area.shader_based.Get() == SHADER_BASED_GPU_ASSISTED; }
                } check_descriptor_indexing;

                struct Check_Buffer_Oob {
                  private:
                    bool value;

                  public:
                    Check_Buffer_Oob() : value(true) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "check_buffer_oob")) {
                            this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "check_buffer_oob");
                        }
                    }
                    bool Get() const { return this->value && Settings::Get().area.shader_based.Get() == SHADER_BASED_GPU_ASSISTED; }

                    struct Warn_On_Bobust_Oob {
                      private:
                        bool value;

                      public:
                        Warn_On_Bobust_Oob() : value(true) {
                            if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "warn_on_robust_oob")) {
                                this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "warn_on_robust_oob");
                            }
                        }

                        bool Get() const {
                            return this->value &&
                                   Settings::Get().area.shader_based.shader_based_gpu_assisted.check_buffer_oob.Get();
                        }
                    } warn_on_robust_oob;
                } check_buffer_oob;

                struct Check_Draw_Indirect {
                  private:
                    bool value;

                  public:
                    Check_Draw_Indirect() : value(true) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "check_draw_indirect")) {
                            this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "check_draw_indirect");
                        }
                    }

                    bool Get() const { return this->value && Settings::Get().area.shader_based.Get() == SHADER_BASED_GPU_ASSISTED; }
                } check_draw_indirect;

                struct Check_Dispatch_Indirect {
                  private:
                    bool value;

                  public:
                    Check_Dispatch_Indirect() : value(true) {
                        if (vku::IsLayerSetting(OBJECT_LAYER_NAME, "check_dispatch_indirect")) {
                            this->value = vku::GetLayerSettingBool(OBJECT_LAYER_NAME, "check_dispatch_indirect");
                        }
                    }

                    bool Get() const { return this->value && Settings::Get().area.shader_based.Get() == SHADER_BASED_GPU_ASSISTED; }
                } check_dispatch_indirect;
            } shader_based_gpu_assisted;
        } shader_based;

        struct Best {
          private:
            bool value;

          public:
            Best() : value(false) {}
            bool Get() const { return this->value; }

            struct Best_Vendor {
              private:
                int value;

              public:
                Best_Vendor() : value(0) {}
                int Get() const { return Settings::Get().area.best.Get() ? this->value : 0; }
            } best_vendor;
        } best;
    } area;

    struct Debug_Action {
      private:
        enum Debug_Action_Enum {
            VK_DBG_LAYER_ACTION_LOG_MSG = 1 << 0,
            VK_DBG_LAYER_ACTION_CALLBACK = 1 << 1,
            VK_DBG_LAYER_ACTION_DEBUG_OUTPUT = 1 << 2,
            VK_DBG_LAYER_ACTION_BREAK = 1 << 3
        } value;

      public:
        Debug_Action() : value(VK_DBG_LAYER_ACTION_LOG_MSG) {}
        int Get() const { return this->value; }

        struct Log_Filename {
          private:
            std::string value;

          public:
            Log_Filename() : value("stdout") {}
            const char *Get() const { return value.c_str(); }
        } log_filename;
    } debug_action;

    struct Report_Flags {
      private:
        enum Report_Flags_Enum { info = 1 << 0, warn = 1 << 1, perf = 1 << 2, error = 1 << 3, debug = 1 << 4 } value;

      public:
        Report_Flags() : value(error) {}
        int Get() const { return this->value; }
    } report_flags;

    struct Enable_Message_Limit {
      private:
        bool value;

      public:
        Enable_Message_Limit() : value(true) {}
        bool Get() const { return this->value; }

        struct Duplicate_Message_Limit {
          private:
            int value;

          public:
            Duplicate_Message_Limit() : value(10) {}
            bool Get() const { return this->value; }
        } duplicate_message_limit;
    } enable_message_limit;

    struct Message_Id_Filter {
      private:
        std::vector<std::string> values;

      public:
        Message_Id_Filter() {}
        std::vector<std::string> Get() const { return this->values; }
    } message_id_filter;
    /*
    struct {
        DebugAction debug_action;                       // debug_action
        std::string log_filename;                       // log_filename
        Report report_flags;                            // report_flags
        bool enable_message_limit;                      // enable_message_limit
        int duplicate_message_limit;                    // duplicate_message_limit
        std::vector<uint32_t> message_id_filter;        // message_id_filter
    } log;

    struct {
        bool enabled;                     // validate_core
        Locking locking;                  // validate_core_locking
        bool check_image_layout;     // core_check_image_layout
        bool check_command_buffer;   // core_check_command_buffer
        bool check_object_in_use;    // core_check_object_in_use
        bool check_query;            // core_check_query
        bool check_shaders;          // core_check_shaders
        bool check_shaders_caching;  // core_check_shaders_caching
    } core;

    bool validate_unique_handles;   // validate_unique_handles
    bool validate_object_lifetime;  // validate_object_lifetime
    bool validate_stateless_param;  // validate_stateless_param
    bool validate_thread_safety;    // validate_thread_safety

    struct {
        bool enabled;               // validate_sync
        bool sync_queue_submit;  // validate_sync_queue_submit
    } sync;

    struct {
        bool enabled;                     // validate_best_practices
        int vendors;
    } best;

    struct {
        ShaderBased mode;

        struct {
            bool to_stdout;        // debug_printf_to_stdout
            bool verbose;          // debug_printf_verbose
            uint32_t buffer_size;  // debug_printf_buffer_size
        } debug_printf;

        struct {
            bool reserve_binding_slot;       // gpuav_reserve_binding_slot
            VMAMode vma_mode;                // gpuav_vma_mode
            bool check_descriptor_indexing;  // gpuav_check_descriptor_indexing
            bool check_buffer_oob;           // gpuav_check_buffer_oob
            bool check_draw_indirect;        // gpuav_check_draw_indirect
            bool check_dispatch_indirect;    // gpuav_check_dispatch_indirect
            bool warn_on_robust_oob;         // gpuav_warn_on_robust_oob
        } gpu_assisted;
    } shader_based;
*/
};

extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

/*
typedef struct {
    const char *layer_description;
    const void *pnext_chain;
    CHECK_ENABLED &enables;
    CHECK_DISABLED &disables;
    std::vector<uint32_t> &message_filter_list;
    int32_t *duplicate_message_limit;
    bool *fine_grained_locking;
} ConfigAndEnvSettings;
*/

/*
// Process validation features, flags and settings specified through extensions, a layer settings file, or environment variables

extern const layer_data::unordered_map<std::string, VkValidationFeatureDisableEXT> VkValFeatureDisableLookup;
extern const layer_data::unordered_map<std::string, VkValidationFeatureEnableEXT> VkValFeatureEnableLookup;
extern const layer_data::unordered_map<std::string, VkValidationFeatureEnable> VkValFeatureEnableLookup2;
extern const layer_data::unordered_map<std::string, ValidationCheckDisables> ValidationDisableLookup;
extern const layer_data::unordered_map<std::string, ValidationCheckEnables> ValidationEnableLookup;
extern const std::vector<std::string> DisableFlagNameHelper;
extern const std::vector<std::string> EnableFlagNameHelper;

void ProcessConfigAndEnvSettings(ConfigAndEnvSettings *settings_data);
*/
