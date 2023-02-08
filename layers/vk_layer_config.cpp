/**************************************************************************
 *
 * Copyright 2014-2023 Valve Software
 * Copyright 2015-2022 Google Inc.
 * Copyright 2019-2023 LunarG, Inc.
 * All Rights Reserved.
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
 **************************************************************************/
#include "vk_layer_config.h"

#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>

#include <vulkan/vk_layer.h>
#include "vk_layer_utils.h"

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

using std::string;

class ConfigFile {
  public:
    ConfigFile();
    ~ConfigFile(){};

    const char *GetOption(const string &option);
    void SetOption(const string &option, const string &value);
    string vk_layer_disables_env_var;
    SettingsFileInfo settings_info{};

  private:
    bool file_is_parsed_;
    std::map<string, string> value_map_;

    string FindSettings();
    void ParseFile(const char *filename);
};

static ConfigFile layer_config;

std::string GetEnvironment(const char *variable) {
#if !defined(__ANDROID__) && !defined(_WIN32)
    const char *output = getenv(variable);
    return output == NULL ? "" : output;
#elif defined(_WIN32)
    int size = GetEnvironmentVariable(variable, NULL, 0);
    if (size == 0) {
        return "";
    }
    char *buffer = new char[size];
    GetEnvironmentVariable(variable, buffer, size);
    string output = buffer;
    delete[] buffer;
    return output;
#elif defined(__ANDROID__)
    string command = "getprop " + string(variable);
    FILE *pPipe = popen(command.c_str(), "r");
    if (pPipe != nullptr) {
        char value[256];
        fgets(value, 256, pPipe);
        pclose(pPipe);

        // Make sure its not an empty line and does not end in a newline
        const auto str_len = strcspn(value, "\r\n");
        if (str_len == 0) {
            return "";
        } else {
            value[str_len] = '\0';
            return string(value);
        }
    } else {
        return "";
    }
#else
    return "";
#endif
}

const char *getLayerOption(const char *option) { return layer_config.GetOption(option); }
const char *GetLayerEnvVar(const char *option) {
    // NOTE: new code should use GetEnvironment directly. This is a workaround for the problem
    // described in https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3048
    static std::string result;
    result = GetEnvironment(option);
    return result.c_str();
}

const SettingsFileInfo *GetLayerSettingsFileInfo() { return &layer_config.settings_info; }

// If option is NULL or stdout, return stdout, otherwise try to open option
// as a filename. If successful, return file handle, otherwise stdout
FILE *getLayerLogOutput(const char *option, const char *layer_name) {
    FILE *log_output = NULL;
    if (!option || !strcmp("stdout", option)) {
        log_output = stdout;
    } else {
        log_output = fopen(option, "w");
        if (log_output == NULL) {
            if (option) {
                std::cout << std::endl
                          << layer_name << " ERROR: Bad output filename specified: " << option << ". Writing to STDOUT instead"
                          << std::endl
                          << std::endl;
            }
            log_output = stdout;
        }
    }
    return log_output;
}

// Map option strings to flag enum values
VkFlags GetLayerOptionFlags(const string &option, vvl::unordered_map<string, VkFlags> const &enum_data, uint32_t option_default) {
    VkDebugReportFlagsEXT flags = option_default;
    string option_list = layer_config.GetOption(option.c_str());

    while (option_list.length() != 0) {
        // Find length of option string
        std::size_t option_length = option_list.find(',');
        if (option_length == option_list.npos) {
            option_length = option_list.size();
        }

        // Get first option item in list
        const string option_item = option_list.substr(0, option_length);

        auto enum_value = enum_data.find(option_item);
        if (enum_value != enum_data.end()) {
            flags |= enum_value->second;
        }

        // Remove first option from option_list
        option_list.erase(0, option_length);
        // Remove possible comma separator
        std::size_t char_position = option_list.find(',');
        if (char_position == 0) {
            option_list.erase(char_position, 1);
        }
        // Remove possible space
        char_position = option_list.find(' ');
        if (char_position == 0) {
            option_list.erase(char_position, 1);
        }
    }
    return flags;
}

void setLayerOption(const char *option, const char *value) { layer_config.SetOption(option, value); }

// Constructor for ConfigFile. Initialize layers to log error messages to stdout by default. If a vk_layer_settings file is present,
// its settings will override the defaults.
ConfigFile::ConfigFile() : file_is_parsed_(false) {
    value_map_["khronos_validation.report_flags"] = "error";

#ifdef WIN32
    // For Windows, enable message logging AND OutputDebugString
    value_map_["khronos_validation.debug_action"] =
        "VK_DBG_LAYER_ACTION_DEFAULT,VK_DBG_LAYER_ACTION_LOG_MSG,VK_DBG_LAYER_ACTION_DEBUG_OUTPUT";
#else   // WIN32
    value_map_["khronos_validation.debug_action"] = "VK_DBG_LAYER_ACTION_DEFAULT,VK_DBG_LAYER_ACTION_LOG_MSG";
#endif  // WIN32
    value_map_["khronos_validation.log_filename"] = "stdout";
    value_map_["khronos_validation.fine_grained_locking"] = "true";
}

const char *ConfigFile::GetOption(const string &option) {
    std::map<string, string>::const_iterator it;
    if (!file_is_parsed_) {
        string settings_file = FindSettings();
        ParseFile(settings_file.c_str());
    }

    if ((it = value_map_.find(option)) == value_map_.end()) {
        return "";
    } else {
        return it->second.c_str();
    }
}

void ConfigFile::SetOption(const string &option, const string &val) {
    if (!file_is_parsed_) {
        string settings_file = FindSettings();
        ParseFile(settings_file.c_str());
    }

    value_map_[option] = val;
}

#if defined(WIN32)
// Check for admin rights
static inline bool IsHighIntegrity() {
    HANDLE process_token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &process_token)) {
        // Maximum possible size of SID_AND_ATTRIBUTES is maximum size of a SID + size of attributes DWORD.
        uint8_t mandatory_label_buffer[SECURITY_MAX_SID_SIZE + sizeof(DWORD)];
        DWORD buffer_size;
        if (GetTokenInformation(process_token, TokenIntegrityLevel, mandatory_label_buffer, sizeof(mandatory_label_buffer),
                                &buffer_size) != 0) {
            const TOKEN_MANDATORY_LABEL *mandatory_label = (const TOKEN_MANDATORY_LABEL *)mandatory_label_buffer;
            const DWORD sub_authority_count = *GetSidSubAuthorityCount(mandatory_label->Label.Sid);
            const DWORD integrity_level = *GetSidSubAuthority(mandatory_label->Label.Sid, sub_authority_count - 1);

            CloseHandle(process_token);
            return integrity_level > SECURITY_MANDATORY_MEDIUM_RID;
        }

        CloseHandle(process_token);
    }

    return false;
}
#endif

string ConfigFile::FindSettings() {
    struct stat info;

#if defined(WIN32)
    // Look for VkConfig-specific settings location specified in the windows registry
    HKEY key;

    const std::array<HKEY, 2> hives = {DEFAULT_VK_REGISTRY_HIVE, SECONDARY_VK_REGISTRY_HIVE};
    const size_t hives_to_check_count = IsHighIntegrity() ? 1 : hives.size();  // Admin checks only the default hive

    for (size_t hive_index = 0; hive_index < hives_to_check_count; ++hive_index) {
        LSTATUS err = RegOpenKeyEx(hives[hive_index], "Software\\Khronos\\Vulkan\\Settings", 0, KEY_READ, &key);
        if (err == ERROR_SUCCESS) {
            char name[2048];
            DWORD i = 0, name_size, type, value, value_size;
            while (ERROR_SUCCESS == RegEnumValue(key, i++, name, &(name_size = sizeof(name)), nullptr, &type,
                                                 reinterpret_cast<LPBYTE>(&value), &(value_size = sizeof(value)))) {
                // Check if the registry entry is a dword with a value of zero
                if (type != REG_DWORD || value != 0) {
                    continue;
                }

                // Check if this actually points to a file
                if ((stat(name, &info) != 0) || !(info.st_mode & S_IFREG)) {
                    continue;
                }

                // Use this file
                RegCloseKey(key);
                settings_info.source = kVkConfig;
                settings_info.location = name;
                return name;
            }

            RegCloseKey(key);
        }
    }

#else
    // Look for VkConfig-specific settings location specified in a specific spot in the linux settings store
    string search_path = GetEnvironment("XDG_DATA_HOME");
    if (search_path == "") {
        search_path = GetEnvironment("HOME");
        if (search_path != "") {
            search_path += "/.local/share";
        }
    }
    // Use the vk_layer_settings.txt file from here, if it is present
    if (search_path != "") {
        string home_file = search_path + "/vulkan/settings.d/vk_layer_settings.txt";
        if (stat(home_file.c_str(), &info) == 0) {
            if (info.st_mode & S_IFREG) {
                settings_info.source = kVkConfig;
                settings_info.location = home_file;
                return home_file;
            }
        }
    }

#endif
    // Look for an enviornment variable override for the settings file location
    string env_path = GetEnvironment("VK_LAYER_SETTINGS_PATH");

    // If the path exists use it, else use vk_layer_settings
    if (stat(env_path.c_str(), &info) == 0) {
        // If this is a directory, append settings file name
        if (info.st_mode & S_IFDIR) {
            env_path.append("/vk_layer_settings.txt");
        }
        settings_info.source = kEnvVar;
        settings_info.location = env_path;
        return env_path;
    }

    // Default -- use the current working directory for the settings file location
    settings_info.source = kLocal;
    char buff[512];
    auto buf_ptr = GetCurrentDir(buff, 512);
    if (buf_ptr) {
        settings_info.location = buf_ptr;
        settings_info.location.append("\\vk_layer_settings.txt");
    }
    return "vk_layer_settings.txt";
}

void ConfigFile::ParseFile(const char *filename) {
    file_is_parsed_ = true;

    // Extract option = value pairs from a file
    std::ifstream file(filename);
    if (file.good()) {
        settings_info.file_found = true;
        for (string line; std::getline(file, line);) {
            // discard comments, which start with '#'
            const auto comments_pos = line.find_first_of('#');
            if (comments_pos != string::npos) line.erase(comments_pos);

            const auto value_pos = line.find_first_of('=');
            if (value_pos != string::npos) {
                const string option = string_trim(line.substr(0, value_pos));
                const string value = string_trim(line.substr(value_pos + 1));
                value_map_[option] = value;
            }
        }
    }
}

void PrintMessageFlags(VkFlags vk_flags, char *msg_flags) {
    bool separator = false;

    msg_flags[0] = 0;
    if (vk_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        strcat(msg_flags, "DEBUG");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "INFO");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "WARN");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "PERF");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "ERROR");
    }
}

void PrintMessageSeverity(VkFlags vk_flags, char *msg_flags) {
    bool separator = false;

    msg_flags[0] = 0;
    if (vk_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        strcat(msg_flags, "VERBOSE");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "INFO");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "WARN");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "ERROR");
    }
}

void PrintMessageType(VkFlags vk_flags, char *msg_flags) {
    bool separator = false;

    msg_flags[0] = 0;
    if (vk_flags & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        strcat(msg_flags, "GEN");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "SPEC");
        separator = true;
    }
    if (vk_flags & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        if (separator) strcat(msg_flags, ",");
        strcat(msg_flags, "PERF");
    }
}

// This catches before dlopen fails if the default Android-26 layers are being used and attempted to be ran on Android 25 or below
#if defined(__ANDROID__)
#include "android_ndk_types.h"  // get AHB_VALIDATION_SUPPORT macro
void __attribute__((constructor)) CheckAndroidVersion();
void CheckAndroidVersion() {
#ifdef AHB_VALIDATION_SUPPORT
    string version_env = GetEnvironment("ro.build.version.sdk");
    int target_version = atoi(version_env.c_str());

    // atoi returns 0 if GetEnvironment fails and don't want false positive errors
    if ((target_version != 0) && (target_version < 26)) {
        LOGCONSOLE(
            "ERROR - Targeted Android version is %d and needs to be 26 or above. Please read "
            "https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/BUILD.md for how to build the Validation Layers "
            "for Android 25 and below",
            target_version);
    }
#endif  // AHB_VALIDATION_SUPPORT
}
#endif  // defined(__ANDROID__)
