/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
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
 *
 * Authors:
 * - Christophe Riccio <christophe@lunarg.com>
 * - Mark Lobodzinski <mark@lunarg.com>
 * - Jon Ashburn
 * - Courtney Goeltzenleuchter
 * - Tobin Ehlis
 */

#include "vk_layer_settings.h"

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdarg>
#include <fstream>
#include <array>
#include <map>
#include <sstream>
#include <regex>
#include <sys/stat.h>

#include <vulkan/vk_layer.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

namespace vku {

static std::string format(const char *message, ...) {
    std::size_t const STRING_BUFFER(4096);

    assert(message != nullptr);
    assert(strlen(message) >= 0 && strlen(message) < STRING_BUFFER);

    char buffer[STRING_BUFFER];
    va_list list;

    va_start(list, message);
    vsnprintf(buffer, STRING_BUFFER, message, list);
    va_end(list);

    return buffer;
}

static bool IsFrames(const std::string &s) {
    static const std::regex FRAME_REGEX("^([0-9]+([-][0-9]+){0,2})(,([0-9]+([-][0-9]+){0,2}))*$");

    return std::regex_search(s, FRAME_REGEX);
}

static bool IsNumber(const std::string &s) {
    static const std::regex FRAME_REGEX("^-?[0-9]*$");

    return std::regex_search(s, FRAME_REGEX);
}

static bool IsFloat(const std::string &s) {
    static const std::regex FRAME_REGEX("^-?[0-9]*([.][0-9]*)?$");

    return std::regex_search(s, FRAME_REGEX);
}

enum Source {
    SOURCE_VKCONFIG,
    SOURCE_ENV_VAR,
    SOURCE_LOCAL,
};

struct SettingsFileInfo {
    SettingsFileInfo() : file_found(false), source(SOURCE_LOCAL) {}

    bool file_found;
    std::string location;
    Source source;
};

class LayerSettings {
   public:
    LayerSettings();
    ~LayerSettings(){};

    void SetCallback(LAYER_SETTING_LOG_CALLBACK callback) { this->callback_ = callback; }
    void Log(const std::string &setting_key, const std::string &message);

    bool Is(const std::string &setting_key);
    const char *Get(const std::string &setting_key);
    void Set(const std::string &setting_key, const std::string &setting_value);

    std::string vk_layer_disables_env_var;
    SettingsFileInfo settings_info;

   private:
    bool file_is_parsed_;
    std::map<std::string, std::string> value_map_;

    std::string last_log_setting;
    std::string last_log_message;

    std::string FindSettings();
    void ParseFile(const char *filename);
    LAYER_SETTING_LOG_CALLBACK callback_;
};

static LayerSettings vk_layer_settings;

#if defined(__ANDROID__)
std::string GetAndroidProperty(const char* name) {
    std::string output;
    const prop_info* pi = __system_property_find(name);
    if (pi) {
        __system_property_read_callback(
            pi,
            [](void* cookie, const char* name, const char* value, uint32_t serial) {
                reinterpret_cast<std::string*>(cookie)->assign(value);
            },
            reinterpret_cast<void*>(&output));
    }
    return output;
}
#endif

static bool IsEnvironment(const char *variable) {
#if defined(__ANDROID__)
    return !GetAndroidProperty(variable).empty();
#else
    return std::getenv(variable) != NULL;
#endif
}

static std::string GetEnvironment(const char *variable) {
#if defined(__ANDROID__)
    return GetAndroidProperty(variable);
#else
    const char *output = std::getenv(variable);
    return output == NULL ? "" : output;
#endif
}

static std::string string_tolower(const std::string &s) {
    std::string result = s;
    for (auto &c : result) {
        c = (char) std::tolower(c);
    }
    return result;
}

static std::string string_toupper(const std::string &s) {
    std::string result = s;
    for (auto &c : result) {
        c = (char) std::toupper(c);
    }
    return result;
}

VK_LAYER_EXPORT const char *GetLayerEnvVar(const char *setting_env) {
    vk_layer_settings.vk_layer_disables_env_var = GetEnvironment(setting_env);
    return vk_layer_settings.vk_layer_disables_env_var.c_str();
}

static std::string TrimPrefix(const std::string &layer_key) {
    std::string key {};
    if (layer_key.find("VK_LAYER_") == 0) {
        std::size_t prefix = std::strlen("VK_LAYER_");
        key = layer_key.substr(prefix, layer_key.size() - prefix);
    } else {
        key = layer_key;
    }
    return key;
}

static std::string GetSettingKey(const char *layer_key, const char *setting_key) {
    std::stringstream result;
    result << string_tolower(TrimPrefix(layer_key)) << "." << setting_key;
    return result.str();
}

static inline std::string TrimVendor(const std::string &layer_key) {
    static const char *separator = "_";

    const std::string &namespace_key = TrimPrefix(layer_key);

    const auto trimmed_beg = namespace_key.find_first_of(separator);
    if (trimmed_beg == std::string::npos) return namespace_key;

    assert(namespace_key.find_last_not_of(separator) != std::string::npos &&
           trimmed_beg <= namespace_key.find_last_not_of(separator));

    return namespace_key.substr(trimmed_beg + 1, namespace_key.size());
}

enum TrimMode {
    TRIM_NONE,
    TRIM_VENDOR,
    TRIM_NAMESPACE,

    TRIM_FIRST = TRIM_NONE,
    TRIM_LAST = TRIM_NAMESPACE,
};

static std::string GetEnvVarKey(const char *layer_key, const char *setting_key, TrimMode trim_mode) {
    std::stringstream result;

#if defined(__ANDROID__)
    switch (trim_mode) {
        default:
        case TRIM_NONE: {
            result << "debug.vulkan." << GetSettingKey(layer_key, setting_key);
            break;
        }
        case TRIM_VENDOR: {
            result << "debug.vulkan." << GetSettingKey(TrimVendor(layer_key).c_str(), setting_key);
            break;
        }
        case TRIM_NAMESPACE: {
            result << "debug.vulkan." << setting_key;
            break;
        }
    }
#else
    switch (trim_mode) {
        default:
        case TRIM_NONE: {
            result << "VK_" << string_toupper(TrimPrefix(layer_key)) << "_" << string_toupper(setting_key);
            break;
        }
        case TRIM_VENDOR: {
            result << "VK_" << string_toupper(TrimVendor(layer_key)) << "_" << string_toupper(setting_key);
            break;
        }
        case TRIM_NAMESPACE: {
            result << "VK_" << string_toupper(setting_key);
            break;
        }
    }

#endif
    return result.str();
}

VK_LAYER_EXPORT void InitLayerSettingsLogCallback(LAYER_SETTING_LOG_CALLBACK callback) {
    vk_layer_settings.SetCallback(callback);
    return;
}

VK_LAYER_EXPORT bool IsLayerSetting(const char *layer_key, const char *setting_key) {
    assert(layer_key);
    assert(!std::string(layer_key).empty());
    assert(setting_key);
    assert(!std::string(setting_key).empty());

    for (int i = TRIM_FIRST, n = TRIM_LAST; i <= n; ++i) {
        if (IsEnvironment(GetEnvVarKey(layer_key, setting_key, static_cast<TrimMode>(i)).c_str())) return true;
    }

    return vk_layer_settings.Is(GetSettingKey(layer_key, setting_key).c_str());
}

static std::string GetLayerSettingData(const char *layer_key, const char *setting_key) {
    // First search in the environment variables
    for (int i = TRIM_FIRST, n = TRIM_LAST; i <= n; ++i) {
        std::string setting = GetLayerEnvVar(GetEnvVarKey(layer_key, setting_key, static_cast<TrimMode>(i)).c_str());
        if (!setting.empty()) return setting;
    }

    // Second search in vk_layer_settings.txt
    return vk_layer_settings.Get(GetSettingKey(layer_key, setting_key).c_str());
}

VK_LAYER_EXPORT bool GetLayerSettingBool(const char *layer_key, const char *setting_key) {
    assert(IsLayerSetting(layer_key, setting_key));

    bool result = false;  // default value

    std::string setting = string_tolower(GetLayerSettingData(layer_key, setting_key));
    if (setting.empty()) {
        vk_layer_settings.Log(setting_key,
                              "The setting is used but the value is empty which is invalid for a boolean setting type.");
    } else if (IsNumber(setting)) {
        result = std::atoi(setting.c_str()) != 0;
    } else if (setting == "true" || setting == "false") {
        result = setting == "true";
    } else {
        std::string message = format("The data provided (%s) is not a boolean value.", setting.c_str());
        vk_layer_settings.Log(setting_key, message);
    }

    return result;
}

VK_LAYER_EXPORT int GetLayerSettingInt(const char *layer_key, const char *setting_key) {
    assert(IsLayerSetting(layer_key, setting_key));

    int result = 0;  // default value

    std::string setting = GetLayerSettingData(layer_key, setting_key);
    if (setting.empty()) {
        std::string message = "The setting is used but the value is empty which is invalid for a integer setting type.";
        vk_layer_settings.Log(setting_key, message);
    } else if (!IsNumber(setting)) {
        std::string message = format("The data provided (%s) is not an integer value.", setting.c_str());
        vk_layer_settings.Log(setting_key, message);
    } else {
        result = std::atoi(setting.c_str());
    }

    return result;
}

VK_LAYER_EXPORT double GetLayerSettingFloat(const char *layer_key, const char *setting_key) {
    assert(IsLayerSetting(layer_key, setting_key));

    double result = 0.0;  // default value

    std::string setting = GetLayerSettingData(layer_key, setting_key);
    if (setting.empty()) {
        std::string message = "The setting is used but the value is empty which is invalid for a floating-point setting type.";
        vk_layer_settings.Log(setting_key, message);
    } else if (!IsFloat(setting)) {
        std::string message = format("The data provided (%s) is not a floating-point value.", setting.c_str());
        vk_layer_settings.Log(setting_key, message);
    } else {
        result = std::atof(setting.c_str());
    }

    return result;
}

VK_LAYER_EXPORT std::string GetLayerSettingString(const char *layer_key, const char *setting_key) {
    assert(IsLayerSetting(layer_key, setting_key));

    std::string setting = GetLayerSettingData(layer_key, setting_key);
    if (setting.empty()) {
        std::string message = "The setting is used but the value is empty which is invalid for a string setting type.";
        vk_layer_settings.Log(setting_key, message);
    }

    return setting;
}

VK_LAYER_EXPORT std::string GetLayerSettingFrames(const char *layer_key, const char *setting_key) {
    assert(IsLayerSetting(layer_key, setting_key));

    std::string setting = GetLayerSettingData(layer_key, setting_key);
    if (!setting.empty() && !IsFrames(setting)) {
        std::string message = format("The data provided (%s) is not a frames value.", setting.c_str());
        vk_layer_settings.Log(setting_key, message);
    }

    return setting;
}

static inline std::vector<std::string> Split(const std::string &value, const std::string &delimiter) {
    std::vector<std::string> result;

    std::string parse = value;

    std::size_t start = 0;
    std::size_t end = parse.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(parse.substr(start, end - start));
        start = end + delimiter.length();
        end = parse.find(delimiter, start);
    }

    const std::string last = parse.substr(start, end);
    if (!last.empty()) {
        result.push_back(last);
    }

    return result;
}

VK_LAYER_EXPORT Strings GetLayerSettingStrings(const char *layer_key, const char *setting_key) {
    assert(IsLayerSetting(layer_key, setting_key));

    std::string setting = GetLayerSettingData(layer_key, setting_key);
    if (setting.find_first_of(",") != std::string::npos) {
        return Split(setting, ",");
    } else {
#ifdef _WIN32
        const char *delimiter = ";";
#else
        const char *delimiter = ":";
#endif
        return Split(setting, delimiter);
    }
}

VK_LAYER_EXPORT List GetLayerSettingList(const char *layer_key, const char *setting_key) {
    assert(IsLayerSetting(layer_key, setting_key));

    std::vector<std::string> inputs = GetLayerSettingStrings(layer_key, setting_key);

    List result;
    for (std::size_t i = 0, n = inputs.size(); i < n; ++i) {
        std::pair<std::string, int> value;
        if (IsNumber(inputs[i])) {
            value.second = atoi(inputs[i].c_str());
        } else {
            value.first = inputs[i];
        }
        result.push_back(value);
    }
    return result;
}

// Constructor for ConfigFile. Initialize layers to log error messages to stdout by default. If a vk_layer_settings file is present,
// its settings will override the defaults.
LayerSettings::LayerSettings() : file_is_parsed_(false), callback_(nullptr) {}

void LayerSettings::Log(const std::string &setting_key, const std::string &message) {
    this->last_log_setting = setting_key;
    this->last_log_message = message;

    if (this->callback_ == nullptr) {
        fprintf(stderr, "LAYER SETTING (%s) error: %s\n", this->last_log_setting.c_str(), this->last_log_message.c_str());
    } else {
        this->callback_(this->last_log_setting.c_str(), this->last_log_message.c_str());
    }
}

bool LayerSettings::Is(const std::string &setting_key) {
    std::map<std::string, std::string>::const_iterator it;
    if (!file_is_parsed_) {
        std::string settings_file = FindSettings();
        ParseFile(settings_file.c_str());
    }

    return value_map_.find(setting_key) != value_map_.end();
}

const char *LayerSettings::Get(const std::string &setting_key) {
    std::map<std::string, std::string>::const_iterator it;
    if (!file_is_parsed_) {
        std::string settings_file = FindSettings();
        ParseFile(settings_file.c_str());
    }

    if ((it = value_map_.find(setting_key)) == value_map_.end()) {
        return "";
    } else {
        return it->second.c_str();
    }
}

void LayerSettings::Set(const std::string &setting_key, const std::string &value) {
    if (!file_is_parsed_) {
        std::string settings_file = FindSettings();
        ParseFile(settings_file.c_str());
    }

    value_map_[setting_key] = value;
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

std::string LayerSettings::FindSettings() {
    struct stat info;

#if defined(WIN32)
    // Look for VkConfig-specific settings location specified in the windows registry
    HKEY key;

    const std::array<HKEY, 2> hives = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};
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
                settings_info.source = SOURCE_VKCONFIG;
                settings_info.location = name;
                return name;
            }

            RegCloseKey(key);
        }
    }

#else
    // Look for VkConfig-specific settings location specified in a specific spot in the linux settings store
    std::string search_path = GetEnvironment("XDG_DATA_HOME");
    if (search_path == "") {
        search_path = GetEnvironment("HOME");
        if (search_path != "") {
            search_path += "/.local/share";
        }
    }
    // Use the vk_layer_settings.txt file from here, if it is present
    if (search_path != "") {
        std::string home_file = search_path + "/vulkan/settings.d/vk_layer_settings.txt";
        if (stat(home_file.c_str(), &info) == 0) {
            if (info.st_mode & S_IFREG) {
                settings_info.source = SOURCE_VKCONFIG;
                settings_info.location = home_file;
                return home_file;
            }
        }
    }

#endif

#ifdef __ANDROID__
    std::string env_path = GetEnvironment("debug.vulkan.khronos_profiles.settings_path");
#else
    // Look for an environment variable override for the settings file location
    std::string env_path = GetEnvironment("VK_LAYER_SETTINGS_PATH");
#endif

    // If the path exists use it, else use vk_layer_settings
    if (stat(env_path.c_str(), &info) == 0) {
        // If this is a directory, append settings file name
        if (info.st_mode & S_IFDIR) {
            env_path.append("/vk_layer_settings.txt");
        }
        settings_info.source = SOURCE_ENV_VAR;
        settings_info.location = env_path;
        return env_path;
    }

    // Default -- use the current working directory for the settings file location
    settings_info.source = SOURCE_LOCAL;
    char buff[512];
    auto buf_ptr = GetCurrentDir(buff, 512);
    if (buf_ptr) {
        settings_info.location = buf_ptr;
        settings_info.location.append("/vk_layer_settings.txt");
    }
    return "vk_layer_settings.txt";
}

static inline std::string TrimWhitespace(const std::string &s) {
    const char *whitespace = " \t\f\v\n\r";

    const auto trimmed_beg = s.find_first_not_of(whitespace);
    if (trimmed_beg == std::string::npos) return "";

    const auto trimmed_end = s.find_last_not_of(whitespace);
    assert(trimmed_end != std::string::npos && trimmed_beg <= trimmed_end);

    return s.substr(trimmed_beg, trimmed_end - trimmed_beg + 1);
}

void LayerSettings::ParseFile(const char *filename) {
    file_is_parsed_ = true;

    // Extract option = value pairs from a file
    std::ifstream file(filename);
    if (file.good()) {
        settings_info.file_found = true;
        for (std::string line; std::getline(file, line);) {
            // discard comments, which start with '#'
            const auto comments_pos = line.find_first_of('#');
            if (comments_pos != std::string::npos) line.erase(comments_pos);

            const auto value_pos = line.find_first_of('=');
            if (value_pos != std::string::npos) {
                const std::string setting_key = TrimWhitespace(line.substr(0, value_pos));
                const std::string setting_value = TrimWhitespace(line.substr(value_pos + 1));
                value_map_[setting_key] = setting_value;
            }
        }
    }
}

}  // namespace vku
