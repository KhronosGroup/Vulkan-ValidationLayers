/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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
#include "logging.h"

#include <csignal>
#include <cstring>
#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <debugapi.h>
#endif

#include <vulkan/vk_enum_string_helper.h>
#include "generated/vk_safe_struct.h"
#include "generated/vk_validation_error_messages.h"
#include "external/xxhash.h"
#include "error_location.h"

[[maybe_unused]] const char *kVUIDUndefined = "VUID_Undefined";

VKAPI_ATTR void SetDebugUtilsSeverityFlags(std::vector<VkLayerDbgFunctionState> &callbacks, debug_report_data *debug_data) {
    // For all callback in list, return their complete set of severities and modes
    for (const auto &item : callbacks) {
        if (item.IsUtils()) {
            debug_data->active_severities |= item.debug_utils_msg_flags;
            debug_data->active_types |= item.debug_utils_msg_type;
        } else {
            VkFlags severities = 0;
            VkFlags types = 0;
            DebugReportFlagsToAnnotFlags(item.debug_report_msg_flags, &severities, &types);
            debug_data->active_severities |= severities;
            debug_data->active_types |= types;
        }
    }
}

VKAPI_ATTR void RemoveDebugUtilsCallback(debug_report_data *debug_data, std::vector<VkLayerDbgFunctionState> &callbacks,
                                            uint64_t callback) {
    auto item = callbacks.begin();
    for (item = callbacks.begin(); item != callbacks.end(); item++) {
        if (item->IsUtils()) {
            if (item->debug_utils_callback_object == CastToHandle<VkDebugUtilsMessengerEXT>(callback)) break;
        } else {
            if (item->debug_report_callback_object == CastToHandle<VkDebugReportCallbackEXT>(callback)) break;
        }
    }
    if (item != callbacks.end()) {
        callbacks.erase(item);
    }
    SetDebugUtilsSeverityFlags(callbacks, debug_data);
}

// Returns TRUE if the number of times this message has been logged is over the set limit
static bool UpdateLogMsgCounts(const debug_report_data *debug_data, int32_t vuid_hash) {
    auto vuid_count_it = debug_data->duplicate_message_count_map.find(vuid_hash);
    if (vuid_count_it == debug_data->duplicate_message_count_map.end()) {
        debug_data->duplicate_message_count_map.emplace(vuid_hash, 1);
        return false;
    } else {
        if (vuid_count_it->second >= debug_data->duplicate_message_limit) {
            return true;
        } else {
            vuid_count_it->second++;
            return false;
        }
    }
}

static bool debug_log_msg(const debug_report_data *debug_data, VkFlags msg_flags, const LogObjectList &objects,
                                 const char *layer_prefix, const char *message, const char *text_vuid) {
    bool bail = false;
    std::vector<VkDebugUtilsLabelEXT> queue_labels;
    std::vector<VkDebugUtilsLabelEXT> cmd_buf_labels;

    // Convert the info to the VK_EXT_debug_utils format
    VkDebugUtilsMessageTypeFlagsEXT types;
    VkDebugUtilsMessageSeverityFlagsEXT severity;
    DebugReportFlagsToAnnotFlags(msg_flags, &severity, &types);

    std::vector<std::string> object_labels;
    // Ensures that push_back will not reallocate, thereby providing pointer
    // stability for pushed strings.
    object_labels.reserve(objects.object_list.size());

    std::vector<VkDebugUtilsObjectNameInfoEXT> object_name_infos;
    object_name_infos.reserve(objects.object_list.size());
    for (uint32_t i = 0; i < objects.object_list.size(); i++) {
        // If only one VkDevice was created, it is just noise to print it out in the error message.
        // Also avoid printing unknown objects, likely if new function is calling error with null LogObjectList
        if ((objects.object_list[i].type == kVulkanObjectTypeDevice && debug_data->device_created <= 1) ||
            (objects.object_list[i].type == kVulkanObjectTypeUnknown) || (objects.object_list[i].handle == 0)) {
            continue;
        }

        VkDebugUtilsObjectNameInfoEXT object_name_info = vku::InitStructHelper();
        object_name_info.objectType = ConvertVulkanObjectToCoreObject(objects.object_list[i].type);
        object_name_info.objectHandle = objects.object_list[i].handle;
        object_name_info.pObjectName = nullptr;

        std::string object_label = {};
        // Look for any debug utils or marker names to use for this object
        object_label = debug_data->DebugReportGetUtilsObjectName(objects.object_list[i].handle);
        if (object_label.empty()) {
            object_label = debug_data->DebugReportGetMarkerObjectName(objects.object_list[i].handle);
        }
        if (!object_label.empty()) {
            object_labels.push_back(std::move(object_label));
            object_name_info.pObjectName = object_labels.back().c_str();
        }

        // If this is a queue, add any queue labels to the callback data.
        if (VK_OBJECT_TYPE_QUEUE == object_name_info.objectType) {
            auto label_iter = debug_data->debugUtilsQueueLabels.find(reinterpret_cast<VkQueue>(object_name_info.objectHandle));
            if (label_iter != debug_data->debugUtilsQueueLabels.end()) {
                auto found_queue_labels = label_iter->second->Export();
                queue_labels.insert(queue_labels.end(), found_queue_labels.begin(), found_queue_labels.end());
            }
            // If this is a command buffer, add any command buffer labels to the callback data.
        } else if (VK_OBJECT_TYPE_COMMAND_BUFFER == object_name_info.objectType) {
            auto label_iter =
                debug_data->debugUtilsCmdBufLabels.find(reinterpret_cast<VkCommandBuffer>(object_name_info.objectHandle));
            if (label_iter != debug_data->debugUtilsCmdBufLabels.end()) {
                auto found_cmd_buf_labels = label_iter->second->Export();
                cmd_buf_labels.insert(cmd_buf_labels.end(), found_cmd_buf_labels.begin(), found_cmd_buf_labels.end());
            }
        }

        object_name_infos.push_back(object_name_info);
    }

    const uint32_t message_id_number = text_vuid ? vvl_vuid_hash(text_vuid) : 0U;

    VkDebugUtilsMessengerCallbackDataEXT callback_data = vku::InitStructHelper();
    callback_data.flags = 0;
    callback_data.pMessageIdName = text_vuid;
    callback_data.messageIdNumber = vvl_bit_cast<int32_t>(message_id_number);
    callback_data.pMessage = nullptr;
    callback_data.queueLabelCount = static_cast<uint32_t>(queue_labels.size());
    callback_data.pQueueLabels = queue_labels.empty() ? nullptr : queue_labels.data();
    callback_data.cmdBufLabelCount = static_cast<uint32_t>(cmd_buf_labels.size());
    callback_data.pCmdBufLabels = cmd_buf_labels.empty() ? nullptr : cmd_buf_labels.data();
    callback_data.objectCount = static_cast<uint32_t>(object_name_infos.size());
    callback_data.pObjects = object_name_infos.data();

    std::ostringstream oss;
    if (msg_flags & kErrorBit) {
        oss << "Validation Error: ";
    } else if (msg_flags & kWarningBit) {
        oss << "Validation Warning: ";
    } else if (msg_flags & kPerformanceWarningBit) {
        oss << "Validation Performance Warning: ";
    } else if (msg_flags & kInformationBit) {
        oss << "Validation Information: ";
    } else if (msg_flags & kVerboseBit) {
        oss << "Verbose Information: ";
    }
    if (text_vuid != nullptr) {
        oss << "[ " << text_vuid << " ] ";
    }
    uint32_t index = 0;
    for (const auto &src_object : object_name_infos) {
        if (0 != src_object.objectHandle) {
            oss << "Object " << index++ << ": handle = 0x" << std::hex << src_object.objectHandle;
            if (src_object.pObjectName) {
                oss << ", name = " << src_object.pObjectName << ", type = ";
            } else {
                oss << ", type = ";
            }
            oss << string_VkObjectType(src_object.objectType) << "; ";
        } else {
            oss << "Object " << index++ << ": VK_NULL_HANDLE, type = " << string_VkObjectType(src_object.objectType) << "; ";
        }
    }
    oss << "| MessageID = 0x" << std::hex << message_id_number << " | " << message;
    std::string composite = oss.str();

    const auto callback_list = &debug_data->debug_callback_list;
    // We only output to default callbacks if there are no non-default callbacks
    bool use_default_callbacks = true;
    for (const auto &current_callback : *callback_list) {
        use_default_callbacks &= current_callback.IsDefault();
    }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    if (debug_data->forceDefaultLogCallback) {
        use_default_callbacks = true;
    }
#endif

    for (const auto &current_callback : *callback_list) {
        // Skip callback if it's a default callback and there are non-default callbacks present
        if (current_callback.IsDefault() && !use_default_callbacks) continue;

        // VK_EXT_debug_utils callback
        if (current_callback.IsUtils() && (current_callback.debug_utils_msg_flags & severity) &&
            (current_callback.debug_utils_msg_type & types)) {
            callback_data.pMessage = composite.c_str();
            if (current_callback.debug_utils_callback_function_ptr(static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(severity),
                                                                   types, &callback_data, current_callback.pUserData)) {
                bail = true;
            }
        } else if (!current_callback.IsUtils() && (current_callback.debug_report_msg_flags & msg_flags)) {
            // VK_EXT_debug_report callback (deprecated)
            if (object_name_infos.empty()) {
                VkDebugUtilsObjectNameInfoEXT null_object_name = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, nullptr,
                                                                  VK_OBJECT_TYPE_UNKNOWN, 0, nullptr};
                // need to have at least one object
                object_name_infos.emplace_back(null_object_name);
            }
            if (current_callback.debug_report_callback_function_ptr(
                    msg_flags, convertCoreObjectToDebugReportObject(object_name_infos[0].objectType),
                    object_name_infos[0].objectHandle, message_id_number, 0, layer_prefix, composite.c_str(),
                    current_callback.pUserData)) {
                bail = true;
            }
        }
    }
    return bail;
}

VKAPI_ATTR void LayerDebugUtilsDestroyInstance(debug_report_data *debug_data) { delete debug_data; }

template <typename TCreateInfo, typename TCallback>
static void LayerCreateCallback(DebugCallbackStatusFlags callback_status, debug_report_data *debug_data,
                                const TCreateInfo *create_info, TCallback *callback) {
    std::unique_lock<std::mutex> lock(debug_data->debug_output_mutex);

    debug_data->debug_callback_list.emplace_back(VkLayerDbgFunctionState());
    auto &callback_state = debug_data->debug_callback_list.back();
    callback_state.callback_status = callback_status;
    callback_state.pUserData = create_info->pUserData;

    if (callback_state.IsUtils()) {
        auto utils_create_info = reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(create_info);
        auto utils_callback = reinterpret_cast<VkDebugUtilsMessengerEXT *>(callback);
        if (!(*utils_callback)) {
            // callback constructed default callbacks have no handle -- so use struct address as unique handle
            *utils_callback = reinterpret_cast<VkDebugUtilsMessengerEXT>(&callback_state);
        }
        callback_state.debug_utils_callback_object = *utils_callback;
        callback_state.debug_utils_callback_function_ptr = utils_create_info->pfnUserCallback;
        callback_state.debug_utils_msg_flags = utils_create_info->messageSeverity;
        callback_state.debug_utils_msg_type = utils_create_info->messageType;
    } else {  // Debug report callback
        auto report_create_info = reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT *>(create_info);
        auto report_callback = reinterpret_cast<VkDebugReportCallbackEXT *>(callback);
        if (!(*report_callback)) {
            // Internally constructed default callbacks have no handle -- so use struct address as unique handle
            *report_callback = reinterpret_cast<VkDebugReportCallbackEXT>(&callback_state);
        }
        callback_state.debug_report_callback_object = *report_callback;
        callback_state.debug_report_callback_function_ptr = report_create_info->pfnCallback;
        callback_state.debug_report_msg_flags = report_create_info->flags;
    }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // On Android, if the default callback system property is set, force the default callback to be printed
    std::string forceLayerLog = GetEnvironment(kForceDefaultCallbackKey);
    int forceDefaultCallback = atoi(forceLayerLog.c_str());
    if (forceDefaultCallback == 1) {
        debug_data->forceDefaultLogCallback = true;
    }
#endif

    SetDebugUtilsSeverityFlags(debug_data->debug_callback_list, debug_data);
}

VKAPI_ATTR VkResult LayerCreateMessengerCallback(debug_report_data *debug_data, bool default_callback,
                                                 const VkDebugUtilsMessengerCreateInfoEXT *create_info,
                                                 VkDebugUtilsMessengerEXT *messenger) {
    LayerCreateCallback((DEBUG_CALLBACK_UTILS | (default_callback ? DEBUG_CALLBACK_DEFAULT : 0)), debug_data, create_info,
                        messenger);
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult LayerCreateReportCallback(debug_report_data *debug_data, bool default_callback,
                                              const VkDebugReportCallbackCreateInfoEXT *create_info,
                                              VkDebugReportCallbackEXT *callback) {
    LayerCreateCallback((default_callback ? DEBUG_CALLBACK_DEFAULT : 0), debug_data, create_info, callback);
    return VK_SUCCESS;
}

VKAPI_ATTR void ActivateInstanceDebugCallbacks(debug_report_data *debug_data) {
    auto current = debug_data->instance_pnext_chain;
    for (;;) {
        auto create_info = vku::FindStructInPNextChain<VkDebugUtilsMessengerCreateInfoEXT>(current);
        if (!create_info) break;
        current = create_info->pNext;
        VkDebugUtilsMessengerEXT utils_callback{};
        LayerCreateCallback((DEBUG_CALLBACK_UTILS | DEBUG_CALLBACK_INSTANCE), debug_data, create_info, &utils_callback);
    }
    for (;;) {
        auto create_info = vku::FindStructInPNextChain<VkDebugReportCallbackCreateInfoEXT>(current);
        if (!create_info) break;
        current = create_info->pNext;
        VkDebugReportCallbackEXT report_callback{};
        LayerCreateCallback(DEBUG_CALLBACK_INSTANCE, debug_data, create_info, &report_callback);
    }
}

VKAPI_ATTR void DeactivateInstanceDebugCallbacks(debug_report_data *debug_data) {
    if (!vku::FindStructInPNextChain<VkDebugUtilsMessengerCreateInfoEXT>(debug_data->instance_pnext_chain) &&
        !vku::FindStructInPNextChain<VkDebugReportCallbackCreateInfoEXT>(debug_data->instance_pnext_chain))
        return;
    std::vector<VkDebugUtilsMessengerEXT> instance_utils_callback_handles{};
    std::vector<VkDebugReportCallbackEXT> instance_report_callback_handles{};
    for (const auto &item : debug_data->debug_callback_list) {
        if (item.IsInstance()) {
            if (item.IsUtils()) {
                instance_utils_callback_handles.push_back(item.debug_utils_callback_object);
            } else {
                instance_report_callback_handles.push_back(item.debug_report_callback_object);
            }
        }
    }
    for (const auto &item : instance_utils_callback_handles) {
        LayerDestroyCallback(debug_data, item);
    }
    for (const auto &item : instance_report_callback_handles) {
        LayerDestroyCallback(debug_data, item);
    }
}

// helper for VUID based filtering. This needs to be separate so it can be called before incurring
// the cost of sprintf()-ing the err_msg needed by LogMsgLocked().
static bool LogMsgEnabled(const debug_report_data *debug_data, std::string_view vuid_text,
                          VkDebugUtilsMessageSeverityFlagsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type) {
    if (!(debug_data->active_severities & severity) || !(debug_data->active_types & type)) {
        return false;
    }
    // If message is in filter list, bail out very early
    const uint32_t message_id = vvl_vuid_hash(vuid_text);
    if (debug_data->filter_message_ids.find(message_id) != debug_data->filter_message_ids.end()) {
        return false;
    }
    if ((debug_data->duplicate_message_limit > 0) && UpdateLogMsgCounts(debug_data, static_cast<int32_t>(message_id))) {
        // Count for this particular message is over the limit, ignore it
        return false;
    }
    return true;
}

VKAPI_ATTR bool LogMsg(const debug_report_data *debug_data, VkFlags msg_flags, const LogObjectList &objects, const Location *loc,
                       std::string_view vuid_text, const char *format, va_list argptr) {
    assert(*(vuid_text.data() + vuid_text.size()) == '\0');

    VkDebugUtilsMessageSeverityFlagsEXT severity;
    VkDebugUtilsMessageTypeFlagsEXT type;

    DebugReportFlagsToAnnotFlags(msg_flags, &severity, &type);
    std::unique_lock<std::mutex> lock(debug_data->debug_output_mutex);
    // Avoid logging cost if msg is to be ignored
    if (!LogMsgEnabled(debug_data, vuid_text, severity, type)) {
        return false;
    }

    // Best guess at an upper bound for message length. At least some of the extra space
    // should get used to store the VUID URL and text in the common case, without additional allocations.
    std::string str_plus_spec_text(1024, '\0');

    // vsnprintf() returns the number of characters that *would* have been printed, if there was
    // enough space. If we have a huge message, reallocate the string and try again.
    int result;
    size_t old_size = str_plus_spec_text.size();
    // The va_list will be destroyed by the call to vsnprintf(), so use a copy in case we need
    // to try again.
    va_list arg_copy;
    va_copy(arg_copy, argptr);
    result = vsnprintf(str_plus_spec_text.data(), str_plus_spec_text.size(), format, arg_copy);
    va_end(arg_copy);

    assert(result >= 0);
    if (result < 0) {
        str_plus_spec_text = "Message generation failure";
    } else if (static_cast<size_t>(result) <= old_size) {
        // Shrink the string to exactly fit the successfully printed string
        str_plus_spec_text.resize(result);
    } else {
        // Grow buffer to fit needed size. Note that the input size to vsnprintf() must
        // include space for the trailing '\0' character, but the return value DOES NOT
        // include the `\0' character.
        str_plus_spec_text.resize(result + 1);
        // consume the va_list passed to us by the caller
        result = vsnprintf(str_plus_spec_text.data(), str_plus_spec_text.size(), format, argptr);
        // remove the `\0' character from the string
        str_plus_spec_text.resize(result);
    }

    // TODO - make Location a reference once old LogError is gone
    if (loc) {
        str_plus_spec_text = loc->Message() + " " + str_plus_spec_text;
    }

    // Append the spec error text to the error message, unless it contains a word treated as special
    if ((vuid_text.find("UNASSIGNED-") == std::string::npos) && (vuid_text.find(kVUIDUndefined) == std::string::npos) &&
        (vuid_text.rfind("SYNC-", 0) == std::string::npos) && (vuid_text.find("INTERNAL-ERROR-") == std::string::npos)) {
        // Linear search makes no assumptions about the layout of the string table. This is not fast, but it does not need to be at
        // this point in the error reporting path
        uint32_t num_vuids = sizeof(vuid_spec_text) / sizeof(vuid_spec_text_pair);
        const char *spec_text = nullptr;
        std::string spec_type;
        for (uint32_t i = 0; i < num_vuids; i++) {
            if (0 == strncmp(vuid_text.data(), vuid_spec_text[i].vuid, vuid_text.size())) {
                spec_text = vuid_spec_text[i].spec_text;
                spec_type = vuid_spec_text[i].url_id;
                break;
            }
        }

        // Construct and append the specification text and link to the appropriate version of the spec
        if (nullptr != spec_text) {
            std::string spec_link = "https://www.khronos.org/registry/vulkan/specs/_MAGIC_KHRONOS_SPEC_TYPE_/html/vkspec.html";
#ifdef ANNOTATED_SPEC_LINK
            spec_link = ANNOTATED_SPEC_LINK;
#endif
            static std::string kAtToken = "_MAGIC_ANNOTATED_SPEC_TYPE_";
            static std::string kKtToken = "_MAGIC_KHRONOS_SPEC_TYPE_";
            static std::string kVeToken = "_MAGIC_VERSION_ID_";
            auto Replace = [](std::string &dest_string, const std::string &to_replace, const std::string &replace_with) {
                if (dest_string.find(to_replace) != std::string::npos) {
                    dest_string.replace(dest_string.find(to_replace), to_replace.size(), replace_with);
                }
            };

            // Add period at end if forgotten
            // This provides better seperation between error message and spec text
            if (str_plus_spec_text.back() != '.') {
                str_plus_spec_text.append(".");
            }

            str_plus_spec_text.append(" The Vulkan spec states: ");
            str_plus_spec_text.append(spec_text);
            if (0 == spec_type.compare("default")) {
                str_plus_spec_text.append(" (https://github.com/KhronosGroup/Vulkan-Docs/search?q=)");
            } else {
                str_plus_spec_text.append(" (");
                str_plus_spec_text.append(spec_link);
                std::string major_version = std::to_string(VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE));
                std::string minor_version = std::to_string(VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE));
                std::string patch_version = std::to_string(VK_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));
                std::string header_version = major_version + "." + minor_version + "." + patch_version;
                std::string annotated_spec_type = major_version + "." + minor_version + "-extensions";
                Replace(str_plus_spec_text, kKtToken, spec_type);
                Replace(str_plus_spec_text, kAtToken, annotated_spec_type);
                Replace(str_plus_spec_text, kVeToken, header_version);
                str_plus_spec_text.append("#");  // CMake hates hashes
            }
            str_plus_spec_text.append(vuid_text);
            str_plus_spec_text.append(")");
        }
    }

    return debug_log_msg(debug_data, msg_flags, objects, "Validation", str_plus_spec_text.c_str(), vuid_text.data());
}

VKAPI_ATTR VkBool32 VKAPI_CALL MessengerBreakCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                      [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                      [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                      [[maybe_unused]] void *user_data) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    DebugBreak();
#else
    raise(SIGTRAP);
#endif

    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL MessengerLogCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
    std::ostringstream msg_buffer;
    char msg_severity[30];
    char msg_type[30];

    PrintMessageSeverity(message_severity, msg_severity);
    PrintMessageType(message_type, msg_type);

    msg_buffer << callback_data->pMessageIdName << "(" << msg_severity << " / " << msg_type
        << "): msgNum: " << callback_data->messageIdNumber << " - " << callback_data->pMessage << "\n";
    msg_buffer << "    Objects: " << callback_data->objectCount << "\n";
    for (uint32_t obj = 0; obj < callback_data->objectCount; ++obj) {
        msg_buffer << "        [" << obj << "] " << std::hex << std::showbase
            << HandleToUint64(callback_data->pObjects[obj].objectHandle) << ", type: " << std::dec << std::noshowbase
            << callback_data->pObjects[obj].objectType
            << ", name: " << (callback_data->pObjects[obj].pObjectName ? callback_data->pObjects[obj].pObjectName : "NULL")
            << "\n";
    }
    const std::string tmp = msg_buffer.str();
    const char *cstr = tmp.c_str();
    fprintf((FILE *)user_data, "%s", cstr);
    fflush((FILE *)user_data);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    LOGCONSOLE("%s", cstr);
#endif

    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL MessengerWin32DebugOutputMsg(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                            VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                            const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                            [[maybe_unused]] void *user_data) {
    std::ostringstream msg_buffer;
    char msg_severity[30];
    char msg_type[30];

    PrintMessageSeverity(message_severity, msg_severity);
    PrintMessageType(message_type, msg_type);

    msg_buffer << callback_data->pMessageIdName << "(" << msg_severity << " / " << msg_type
               << "): msgNum: " << callback_data->messageIdNumber << " - " << callback_data->pMessage << "\n";
    msg_buffer << "    Objects: " << callback_data->objectCount << "\n";

    for (uint32_t obj = 0; obj < callback_data->objectCount; ++obj) {
        msg_buffer << "       [" << obj << "]  " << std::hex << std::showbase
                   << HandleToUint64(callback_data->pObjects[obj].objectHandle) << ", type: " << std::dec << std::noshowbase
                   << callback_data->pObjects[obj].objectType
                   << ", name: " << (callback_data->pObjects[obj].pObjectName ? callback_data->pObjects[obj].pObjectName : "NULL")
                   << "\n";
    }
    const std::string tmp = msg_buffer.str();
    [[maybe_unused]] const char *cstr = tmp.c_str();

#ifdef VK_USE_PLATFORM_WIN32_KHR
    OutputDebugString(cstr);
#endif

    return false;
}

uint32_t vvl_vuid_hash(std::string_view vuid) {
    constexpr uint32_t seed = 8;
    return XXH32(vuid.data(), vuid.size(), seed);
}
