// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (c) 2015-2020 Google Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 *
 ****************************************************************************/



#include "corechecks_instrumentation.h"

#ifdef INSTRUMENT_CORECHECKS
std::ofstream instrumentation_data_file;
static uint32_t refcount = 0;

static TargetFrameInfo target_frame_info{};
static bool select_frames = false;
static bool key_frames = false;
static bool key_capture_state = false;
static uint64_t frame_counter = 0;

std::vector<std::string> TokenizeRangeList(const std::string& str, char splitter) {
    std::vector<std::string> tokens;
    std::istringstream string_stream(str);
    std::string token;
    while (std::getline(string_stream, token, splitter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool GetFrameRangeInfo(const std::string& data) {
    bool use_ranges = false;
    std::vector<std::string> tokens = TokenizeRangeList(data, ',');

    for (const auto& token : tokens) {
        std::vector<std::string> range = TokenizeRangeList(token, '-');
        if ((range.size() == 1) && range[0].length()) {
            target_frame_info.target_frames.push_back(std::stoi(range[0]));
        } else if ((range.size() == 2) && range[0].length() && range[1].length()) {
            uint32_t r_begin = std::stoi(range[0]);
            uint32_t r_end = std::stoi(range[1]);
            for (uint32_t i = r_begin; i <= r_end; i++) {
                target_frame_info.target_frames.push_back(i);
            }
        }
        if (target_frame_info.target_frames.size()) {
            target_frame_info.min_frame = *min_element(target_frame_info.target_frames.begin(), target_frame_info.target_frames.end());
            target_frame_info.max_frame = *max_element(target_frame_info.target_frames.begin(), target_frame_info.target_frames.end());
            use_ranges = true;
        }
    }
    return use_ranges;
}
#endif

// Comment out this definition to get human-readable (and much larger) file output
#define BINARY_INSTRUMENTATION_OUTPUT

#ifdef BINARY_INSTRUMENTATION_OUTPUT
const char* output_filename = "CorechecksInstrumentationData.bin";
#else
const char* output_filename = "CorechecksInstrumentationData.txt";
#endif

// These functions are the chassis interface to instrumentation -- define them as no-ops for default case
void OpenInstrumentationFile() {
#ifdef INSTRUMENT_CORECHECKS
    if (!instrumentation_data_file.is_open()) {
        instrumentation_data_file.open(output_filename, std::ofstream::binary | std::ofstream::app);
        std::string frames_to_instrument{};
        std::string env_instrumentation_frames = GetLayerEnvVar("VK_LAYER_INSTRUMENTATION_FRAMES");
        std::string config_instrumentation_frames = getLayerOption("khronos_validation.instrumentation_frames");

        // Check for settings, env var takes precedence over layer settings file
        if (env_instrumentation_frames.length() != 0) {
            frames_to_instrument = env_instrumentation_frames;
        } else if (config_instrumentation_frames.length() != 0) {
            frames_to_instrument = config_instrumentation_frames;
        } else {
            frames_to_instrument = "all";
        }
        // Convert to items to lower-case
        std::transform(frames_to_instrument.begin(), frames_to_instrument.end(), frames_to_instrument.begin(), [](unsigned char c) { return std::tolower(c); });

        // Valid options are "all", "hotkey", or string of ints and ranges: "1-5,10,-12,15-16,25-35,67,69,99-105"
        if (frames_to_instrument != "all") {
            if (frames_to_instrument == "hotkey") {
                key_frames = true;
                key_capture_state = false;
                GetAsyncKeyState(VK_F12);
                GetAsyncKeyState(VK_SNAPSHOT);
            } else {
                select_frames = GetFrameRangeInfo(frames_to_instrument);
            }
        }

        // TODO: Output instrumentation header content

    }
    refcount++;
#endif
}
void CloseInstrumentationFile() {
#ifdef INSTRUMENT_CORECHECKS
    refcount--;
    if (!refcount) {
        instrumentation_data_file.close();
    }
#endif
}

#ifdef INSTRUMENT_CORECHECKS

static inline int64_t StartCounting() {
    LARGE_INTEGER payload;
    QueryPerformanceCounter(&payload);
    return payload.QuadPart;
}

static inline void StopCounting(int64_t start_time, uint32_t api_id, const char* api_name) {
    LARGE_INTEGER payload;
    QueryPerformanceCounter(&payload);

    if (key_frames) {
        if ((GetAsyncKeyState(VK_F12) & 0x01) || (GetAsyncKeyState(VK_SNAPSHOT))) key_capture_state = !key_capture_state;
        if (!key_capture_state) return;
    } else if (select_frames) {
        if (frame_counter < target_frame_info.min_frame || frame_counter > target_frame_info.max_frame) return;
        if (std::find(target_frame_info.target_frames.begin(), target_frame_info.target_frames.end(), frame_counter) ==
            target_frame_info.target_frames.end()) return;
    }

    uint32_t diff = (payload.QuadPart - start_time) & 0xFFFFFFFF;

#ifdef BINARY_INSTRUMENTATION_OUTPUT
    TimingRecord out_rec = { start_time, api_id, diff };
    instrumentation_data_file.write(reinterpret_cast<char*>(&out_rec), sizeof(TimingRecord));
#else // Human-readable instrumentation output
    std::stringstream out_string;
    out_string << "Frame " << frame_counter << " : " << start_time << ": " << api_name << ": " << diff << std::endl;
    instrumentation_data_file << out_string.str();
#endif // BINARY_INSTRUMENTATION_OUTPUT
}

// Manually written intercepts
void CoreChecksInstrumented::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordQueuePresentKHR(queue, pPresentInfo, result);
    StopCounting(start, 21, "PostCallRecordQueuePresentKHR");
    // Track frames for selective output. Bumping in PostCallRecord regardless of result value matches api_dump behavior
    frame_counter++;
};

// Code-generated intercepts
bool CoreChecksInstrumented::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance);
    StopCounting(start, 0000, "PreCallValidateCreateInstance");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);
    StopCounting(start, 0000, "PreCallRecordCreateInstance");
}

void CoreChecksInstrumented::PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, result);
    StopCounting(start, 0000, "PostCallRecordCreateInstance");
}

bool CoreChecksInstrumented::PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyInstance(instance, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyInstance");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyInstance(instance, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyInstance");
}

void CoreChecksInstrumented::PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyInstance(instance, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyInstance");
}

bool CoreChecksInstrumented::PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    StopCounting(start, 0000, "PreCallValidateEnumeratePhysicalDevices");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    StopCounting(start, 0000, "PreCallRecordEnumeratePhysicalDevices");
}

void CoreChecksInstrumented::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices, result);
    StopCounting(start, 0000, "PostCallRecordEnumeratePhysicalDevices");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceFeatures");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceFeatures");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceFeatures");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceFormatProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceFormatProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceFormatProperties");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceImageFormatProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceImageFormatProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceImageFormatProperties");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceProperties(physicalDevice, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceProperties");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceQueueFamilyProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceQueueFamilyProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceQueueFamilyProperties");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceMemoryProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceMemoryProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceMemoryProperties");
}

bool CoreChecksInstrumented::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    StopCounting(start, 0000, "PreCallValidateCreateDevice");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, extra_data);
    StopCounting(start, 0000, "PreCallRecordCreateDevice");
}

void CoreChecksInstrumented::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, result);
    StopCounting(start, 0000, "PostCallRecordCreateDevice");
}

bool CoreChecksInstrumented::PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyDevice(device, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyDevice");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyDevice(device, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyDevice");
}

void CoreChecksInstrumented::PostCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyDevice(device, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyDevice");
}

bool CoreChecksInstrumented::PreCallValidateEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateEnumerateInstanceExtensionProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordEnumerateInstanceExtensionProperties");
}

void CoreChecksInstrumented::PostCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordEnumerateInstanceExtensionProperties");
}

bool CoreChecksInstrumented::PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateEnumerateDeviceExtensionProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordEnumerateDeviceExtensionProperties");
}

void CoreChecksInstrumented::PostCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordEnumerateDeviceExtensionProperties");
}

bool CoreChecksInstrumented::PreCallValidateEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateEnumerateInstanceLayerProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordEnumerateInstanceLayerProperties");
}

void CoreChecksInstrumented::PostCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEnumerateInstanceLayerProperties(pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordEnumerateInstanceLayerProperties");
}

bool CoreChecksInstrumented::PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateEnumerateDeviceLayerProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordEnumerateDeviceLayerProperties");
}

void CoreChecksInstrumented::PostCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordEnumerateDeviceLayerProperties");
}

bool CoreChecksInstrumented::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    StopCounting(start, 0000, "PreCallValidateGetDeviceQueue");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    StopCounting(start, 0000, "PreCallRecordGetDeviceQueue");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    StopCounting(start, 0000, "PostCallRecordGetDeviceQueue");
}

bool CoreChecksInstrumented::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence);
    StopCounting(start, 0000, "PreCallValidateQueueSubmit");
    return result;
}

void CoreChecksInstrumented::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
    StopCounting(start, 0000, "PreCallRecordQueueSubmit");
}

void CoreChecksInstrumented::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
    StopCounting(start, 0000, "PostCallRecordQueueSubmit");
}

bool CoreChecksInstrumented::PreCallValidateQueueWaitIdle(VkQueue queue) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateQueueWaitIdle(queue);
    StopCounting(start, 0000, "PreCallValidateQueueWaitIdle");
    return result;
}

void CoreChecksInstrumented::PreCallRecordQueueWaitIdle(VkQueue queue) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordQueueWaitIdle(queue);
    StopCounting(start, 0000, "PreCallRecordQueueWaitIdle");
}

void CoreChecksInstrumented::PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordQueueWaitIdle(queue, result);
    StopCounting(start, 0000, "PostCallRecordQueueWaitIdle");
}

bool CoreChecksInstrumented::PreCallValidateDeviceWaitIdle(VkDevice device) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDeviceWaitIdle(device);
    StopCounting(start, 0000, "PreCallValidateDeviceWaitIdle");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDeviceWaitIdle(VkDevice device) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDeviceWaitIdle(device);
    StopCounting(start, 0000, "PreCallRecordDeviceWaitIdle");
}

void CoreChecksInstrumented::PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDeviceWaitIdle(device, result);
    StopCounting(start, 0000, "PostCallRecordDeviceWaitIdle");
}

bool CoreChecksInstrumented::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    StopCounting(start, 0000, "PreCallValidateAllocateMemory");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    StopCounting(start, 0000, "PreCallRecordAllocateMemory");
}

void CoreChecksInstrumented::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);
    StopCounting(start, 0000, "PostCallRecordAllocateMemory");
}

bool CoreChecksInstrumented::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateFreeMemory(device, memory, pAllocator);
    StopCounting(start, 0000, "PreCallValidateFreeMemory");
    return result;
}

void CoreChecksInstrumented::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordFreeMemory(device, memory, pAllocator);
    StopCounting(start, 0000, "PreCallRecordFreeMemory");
}

void CoreChecksInstrumented::PostCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordFreeMemory(device, memory, pAllocator);
    StopCounting(start, 0000, "PostCallRecordFreeMemory");
}

bool CoreChecksInstrumented::PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateMapMemory(device, memory, offset, size, flags, ppData);
    StopCounting(start, 0000, "PreCallValidateMapMemory");
    return result;
}

void CoreChecksInstrumented::PreCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordMapMemory(device, memory, offset, size, flags, ppData);
    StopCounting(start, 0000, "PreCallRecordMapMemory");
}

void CoreChecksInstrumented::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordMapMemory(device, memory, offset, size, flags, ppData, result);
    StopCounting(start, 0000, "PostCallRecordMapMemory");
}

bool CoreChecksInstrumented::PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory memory) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateUnmapMemory(device, memory);
    StopCounting(start, 0000, "PreCallValidateUnmapMemory");
    return result;
}

void CoreChecksInstrumented::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordUnmapMemory(device, memory);
    StopCounting(start, 0000, "PreCallRecordUnmapMemory");
}

void CoreChecksInstrumented::PostCallRecordUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordUnmapMemory(device, memory);
    StopCounting(start, 0000, "PostCallRecordUnmapMemory");
}

bool CoreChecksInstrumented::PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    StopCounting(start, 0000, "PreCallValidateFlushMappedMemoryRanges");
    return result;
}

void CoreChecksInstrumented::PreCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    StopCounting(start, 0000, "PreCallRecordFlushMappedMemoryRanges");
}

void CoreChecksInstrumented::PostCallRecordFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    StopCounting(start, 0000, "PostCallRecordFlushMappedMemoryRanges");
}

bool CoreChecksInstrumented::PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    StopCounting(start, 0000, "PreCallValidateInvalidateMappedMemoryRanges");
    return result;
}

void CoreChecksInstrumented::PreCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    StopCounting(start, 0000, "PreCallRecordInvalidateMappedMemoryRanges");
}

void CoreChecksInstrumented::PostCallRecordInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges, result);
    StopCounting(start, 0000, "PostCallRecordInvalidateMappedMemoryRanges");
}

bool CoreChecksInstrumented::PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    StopCounting(start, 0000, "PreCallValidateGetDeviceMemoryCommitment");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    StopCounting(start, 0000, "PreCallRecordGetDeviceMemoryCommitment");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    StopCounting(start, 0000, "PostCallRecordGetDeviceMemoryCommitment");
}

bool CoreChecksInstrumented::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBindBufferMemory(device, buffer, memory, memoryOffset);
    StopCounting(start, 0000, "PreCallValidateBindBufferMemory");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBindBufferMemory(device, buffer, memory, memoryOffset);
    StopCounting(start, 0000, "PreCallRecordBindBufferMemory");
}

void CoreChecksInstrumented::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBindBufferMemory(device, buffer, memory, memoryOffset, result);
    StopCounting(start, 0000, "PostCallRecordBindBufferMemory");
}

bool CoreChecksInstrumented::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBindImageMemory(device, image, memory, memoryOffset);
    StopCounting(start, 0000, "PreCallValidateBindImageMemory");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBindImageMemory(device, image, memory, memoryOffset);
    StopCounting(start, 0000, "PreCallRecordBindImageMemory");
}

void CoreChecksInstrumented::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBindImageMemory(device, image, memory, memoryOffset, result);
    StopCounting(start, 0000, "PostCallRecordBindImageMemory");
}

bool CoreChecksInstrumented::PreCallValidateGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetBufferMemoryRequirements");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetBufferMemoryRequirements");
}

void CoreChecksInstrumented::PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetBufferMemoryRequirements");
}

bool CoreChecksInstrumented::PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageMemoryRequirements(device, image, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetImageMemoryRequirements");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetImageMemoryRequirements");
}

void CoreChecksInstrumented::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetImageMemoryRequirements");
}

bool CoreChecksInstrumented::PreCallValidateGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetImageSparseMemoryRequirements");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetImageSparseMemoryRequirements");
}

void CoreChecksInstrumented::PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetImageSparseMemoryRequirements");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSparseImageFormatProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSparseImageFormatProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSparseImageFormatProperties");
}

bool CoreChecksInstrumented::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    StopCounting(start, 0000, "PreCallValidateQueueBindSparse");
    return result;
}

void CoreChecksInstrumented::PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    StopCounting(start, 0000, "PreCallRecordQueueBindSparse");
}

void CoreChecksInstrumented::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence, result);
    StopCounting(start, 0000, "PostCallRecordQueueBindSparse");
}

bool CoreChecksInstrumented::PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateFence(device, pCreateInfo, pAllocator, pFence);
    StopCounting(start, 0000, "PreCallValidateCreateFence");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence);
    StopCounting(start, 0000, "PreCallRecordCreateFence");
}

void CoreChecksInstrumented::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence, result);
    StopCounting(start, 0000, "PostCallRecordCreateFence");
}

bool CoreChecksInstrumented::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyFence(device, fence, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyFence");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyFence(device, fence, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyFence");
}

void CoreChecksInstrumented::PostCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyFence(device, fence, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyFence");
}

bool CoreChecksInstrumented::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateResetFences(device, fenceCount, pFences);
    StopCounting(start, 0000, "PreCallValidateResetFences");
    return result;
}

void CoreChecksInstrumented::PreCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordResetFences(device, fenceCount, pFences);
    StopCounting(start, 0000, "PreCallRecordResetFences");
}

void CoreChecksInstrumented::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordResetFences(device, fenceCount, pFences, result);
    StopCounting(start, 0000, "PostCallRecordResetFences");
}

bool CoreChecksInstrumented::PreCallValidateGetFenceStatus(VkDevice device, VkFence fence) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetFenceStatus(device, fence);
    StopCounting(start, 0000, "PreCallValidateGetFenceStatus");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetFenceStatus(VkDevice device, VkFence fence) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetFenceStatus(device, fence);
    StopCounting(start, 0000, "PreCallRecordGetFenceStatus");
}

void CoreChecksInstrumented::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetFenceStatus(device, fence, result);
    StopCounting(start, 0000, "PostCallRecordGetFenceStatus");
}

bool CoreChecksInstrumented::PreCallValidateWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    StopCounting(start, 0000, "PreCallValidateWaitForFences");
    return result;
}

void CoreChecksInstrumented::PreCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    StopCounting(start, 0000, "PreCallRecordWaitForFences");
}

void CoreChecksInstrumented::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, result);
    StopCounting(start, 0000, "PostCallRecordWaitForFences");
}

bool CoreChecksInstrumented::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    StopCounting(start, 0000, "PreCallValidateCreateSemaphore");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    StopCounting(start, 0000, "PreCallRecordCreateSemaphore");
}

void CoreChecksInstrumented::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore, result);
    StopCounting(start, 0000, "PostCallRecordCreateSemaphore");
}

bool CoreChecksInstrumented::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroySemaphore(device, semaphore, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroySemaphore");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroySemaphore(device, semaphore, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroySemaphore");
}

void CoreChecksInstrumented::PostCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroySemaphore(device, semaphore, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroySemaphore");
}

bool CoreChecksInstrumented::PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    StopCounting(start, 0000, "PreCallValidateCreateEvent");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    StopCounting(start, 0000, "PreCallRecordCreateEvent");
}

void CoreChecksInstrumented::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent, result);
    StopCounting(start, 0000, "PostCallRecordCreateEvent");
}

bool CoreChecksInstrumented::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyEvent(device, event, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyEvent");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyEvent(device, event, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyEvent");
}

void CoreChecksInstrumented::PostCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyEvent(device, event, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyEvent");
}

bool CoreChecksInstrumented::PreCallValidateGetEventStatus(VkDevice device, VkEvent event) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetEventStatus(device, event);
    StopCounting(start, 0000, "PreCallValidateGetEventStatus");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetEventStatus(VkDevice device, VkEvent event) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetEventStatus(device, event);
    StopCounting(start, 0000, "PreCallRecordGetEventStatus");
}

void CoreChecksInstrumented::PostCallRecordGetEventStatus(VkDevice device, VkEvent event, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetEventStatus(device, event, result);
    StopCounting(start, 0000, "PostCallRecordGetEventStatus");
}

bool CoreChecksInstrumented::PreCallValidateSetEvent(VkDevice device, VkEvent event) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSetEvent(device, event);
    StopCounting(start, 0000, "PreCallValidateSetEvent");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSetEvent(VkDevice device, VkEvent event) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSetEvent(device, event);
    StopCounting(start, 0000, "PreCallRecordSetEvent");
}

void CoreChecksInstrumented::PostCallRecordSetEvent(VkDevice device, VkEvent event, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSetEvent(device, event, result);
    StopCounting(start, 0000, "PostCallRecordSetEvent");
}

bool CoreChecksInstrumented::PreCallValidateResetEvent(VkDevice device, VkEvent event) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateResetEvent(device, event);
    StopCounting(start, 0000, "PreCallValidateResetEvent");
    return result;
}

void CoreChecksInstrumented::PreCallRecordResetEvent(VkDevice device, VkEvent event) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordResetEvent(device, event);
    StopCounting(start, 0000, "PreCallRecordResetEvent");
}

void CoreChecksInstrumented::PostCallRecordResetEvent(VkDevice device, VkEvent event, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordResetEvent(device, event, result);
    StopCounting(start, 0000, "PostCallRecordResetEvent");
}

bool CoreChecksInstrumented::PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    StopCounting(start, 0000, "PreCallValidateCreateQueryPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    StopCounting(start, 0000, "PreCallRecordCreateQueryPool");
}

void CoreChecksInstrumented::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool, result);
    StopCounting(start, 0000, "PostCallRecordCreateQueryPool");
}

bool CoreChecksInstrumented::PreCallValidateDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyQueryPool(device, queryPool, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyQueryPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyQueryPool(device, queryPool, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyQueryPool");
}

void CoreChecksInstrumented::PostCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyQueryPool(device, queryPool, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyQueryPool");
}

bool CoreChecksInstrumented::PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    StopCounting(start, 0000, "PreCallValidateGetQueryPoolResults");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    StopCounting(start, 0000, "PreCallRecordGetQueryPoolResults");
}

void CoreChecksInstrumented::PostCallRecordGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags, result);
    StopCounting(start, 0000, "PostCallRecordGetQueryPoolResults");
}

bool CoreChecksInstrumented::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    StopCounting(start, 0000, "PreCallValidateCreateBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, extra_data);
    StopCounting(start, 0000, "PreCallRecordCreateBuffer");
}

void CoreChecksInstrumented::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, result);
    StopCounting(start, 0000, "PostCallRecordCreateBuffer");
}

bool CoreChecksInstrumented::PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyBuffer(device, buffer, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyBuffer(device, buffer, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyBuffer");
}

void CoreChecksInstrumented::PostCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyBuffer(device, buffer, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyBuffer");
}

bool CoreChecksInstrumented::PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView);
    StopCounting(start, 0000, "PreCallValidateCreateBufferView");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView);
    StopCounting(start, 0000, "PreCallRecordCreateBufferView");
}

void CoreChecksInstrumented::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView, result);
    StopCounting(start, 0000, "PostCallRecordCreateBufferView");
}

bool CoreChecksInstrumented::PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyBufferView(device, bufferView, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyBufferView");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyBufferView(device, bufferView, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyBufferView");
}

void CoreChecksInstrumented::PostCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyBufferView(device, bufferView, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyBufferView");
}

bool CoreChecksInstrumented::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage);
    StopCounting(start, 0000, "PreCallValidateCreateImage");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage);
    StopCounting(start, 0000, "PreCallRecordCreateImage");
}

void CoreChecksInstrumented::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage, result);
    StopCounting(start, 0000, "PostCallRecordCreateImage");
}

bool CoreChecksInstrumented::PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyImage(device, image, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyImage");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyImage(device, image, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyImage");
}

void CoreChecksInstrumented::PostCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyImage(device, image, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyImage");
}

bool CoreChecksInstrumented::PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    StopCounting(start, 0000, "PreCallValidateGetImageSubresourceLayout");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    StopCounting(start, 0000, "PreCallRecordGetImageSubresourceLayout");
}

void CoreChecksInstrumented::PostCallRecordGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    StopCounting(start, 0000, "PostCallRecordGetImageSubresourceLayout");
}

bool CoreChecksInstrumented::PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView);
    StopCounting(start, 0000, "PreCallValidateCreateImageView");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView);
    StopCounting(start, 0000, "PreCallRecordCreateImageView");
}

void CoreChecksInstrumented::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView, result);
    StopCounting(start, 0000, "PostCallRecordCreateImageView");
}

bool CoreChecksInstrumented::PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyImageView(device, imageView, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyImageView");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyImageView(device, imageView, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyImageView");
}

void CoreChecksInstrumented::PostCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyImageView(device, imageView, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyImageView");
}

bool CoreChecksInstrumented::PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    StopCounting(start, 0000, "PreCallValidateCreateShaderModule");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, extra_data);
    StopCounting(start, 0000, "PreCallRecordCreateShaderModule");
}

void CoreChecksInstrumented::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, VkResult result, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, result, extra_data);
    StopCounting(start, 0000, "PostCallRecordCreateShaderModule");
}

bool CoreChecksInstrumented::PreCallValidateDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyShaderModule(device, shaderModule, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyShaderModule");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyShaderModule");
}

void CoreChecksInstrumented::PostCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyShaderModule");
}

bool CoreChecksInstrumented::PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    StopCounting(start, 0000, "PreCallValidateCreatePipelineCache");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    StopCounting(start, 0000, "PreCallRecordCreatePipelineCache");
}

void CoreChecksInstrumented::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache, result);
    StopCounting(start, 0000, "PostCallRecordCreatePipelineCache");
}

bool CoreChecksInstrumented::PreCallValidateDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyPipelineCache(device, pipelineCache, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyPipelineCache");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyPipelineCache");
}

void CoreChecksInstrumented::PostCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyPipelineCache");
}

bool CoreChecksInstrumented::PreCallValidateGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    StopCounting(start, 0000, "PreCallValidateGetPipelineCacheData");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    StopCounting(start, 0000, "PreCallRecordGetPipelineCacheData");
}

void CoreChecksInstrumented::PostCallRecordGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData, result);
    StopCounting(start, 0000, "PostCallRecordGetPipelineCacheData");
}

bool CoreChecksInstrumented::PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    StopCounting(start, 0000, "PreCallValidateMergePipelineCaches");
    return result;
}

void CoreChecksInstrumented::PreCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    StopCounting(start, 0000, "PreCallRecordMergePipelineCaches");
}

void CoreChecksInstrumented::PostCallRecordMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches, result);
    StopCounting(start, 0000, "PostCallRecordMergePipelineCaches");
}

bool CoreChecksInstrumented::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    StopCounting(start, 0000, "PreCallValidateCreateGraphicsPipelines");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    StopCounting(start, 0000, "PreCallRecordCreateGraphicsPipelines");
}

void CoreChecksInstrumented::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, extra_data);
    StopCounting(start, 0000, "PostCallRecordCreateGraphicsPipelines");
}

bool CoreChecksInstrumented::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    StopCounting(start, 0000, "PreCallValidateCreateComputePipelines");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    StopCounting(start, 0000, "PreCallRecordCreateComputePipelines");
}

void CoreChecksInstrumented::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, extra_data);
    StopCounting(start, 0000, "PostCallRecordCreateComputePipelines");
}

bool CoreChecksInstrumented::PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyPipeline(device, pipeline, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyPipeline");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyPipeline");
}

void CoreChecksInstrumented::PostCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyPipeline(device, pipeline, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyPipeline");
}

bool CoreChecksInstrumented::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    StopCounting(start, 0000, "PreCallValidateCreatePipelineLayout");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, extra_data);
    StopCounting(start, 0000, "PreCallRecordCreatePipelineLayout");
}

void CoreChecksInstrumented::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);
    StopCounting(start, 0000, "PostCallRecordCreatePipelineLayout");
}

bool CoreChecksInstrumented::PreCallValidateDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyPipelineLayout");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyPipelineLayout");
}

void CoreChecksInstrumented::PostCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyPipelineLayout");
}

bool CoreChecksInstrumented::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    StopCounting(start, 0000, "PreCallValidateCreateSampler");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    StopCounting(start, 0000, "PreCallRecordCreateSampler");
}

void CoreChecksInstrumented::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler, result);
    StopCounting(start, 0000, "PostCallRecordCreateSampler");
}

bool CoreChecksInstrumented::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroySampler(device, sampler, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroySampler");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroySampler(device, sampler, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroySampler");
}

void CoreChecksInstrumented::PostCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroySampler(device, sampler, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroySampler");
}

bool CoreChecksInstrumented::PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    StopCounting(start, 0000, "PreCallValidateCreateDescriptorSetLayout");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    StopCounting(start, 0000, "PreCallRecordCreateDescriptorSetLayout");
}

void CoreChecksInstrumented::PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout, result);
    StopCounting(start, 0000, "PostCallRecordCreateDescriptorSetLayout");
}

bool CoreChecksInstrumented::PreCallValidateDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyDescriptorSetLayout");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyDescriptorSetLayout");
}

void CoreChecksInstrumented::PostCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyDescriptorSetLayout");
}

bool CoreChecksInstrumented::PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    StopCounting(start, 0000, "PreCallValidateCreateDescriptorPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    StopCounting(start, 0000, "PreCallRecordCreateDescriptorPool");
}

void CoreChecksInstrumented::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool, result);
    StopCounting(start, 0000, "PostCallRecordCreateDescriptorPool");
}

bool CoreChecksInstrumented::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyDescriptorPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyDescriptorPool");
}

void CoreChecksInstrumented::PostCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyDescriptorPool");
}

bool CoreChecksInstrumented::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
    StopCounting(start, 0000, "PreCallValidateResetDescriptorPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordResetDescriptorPool(device, descriptorPool, flags);
    StopCounting(start, 0000, "PreCallRecordResetDescriptorPool");
}

void CoreChecksInstrumented::PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordResetDescriptorPool(device, descriptorPool, flags, result);
    StopCounting(start, 0000, "PostCallRecordResetDescriptorPool");
}

bool CoreChecksInstrumented::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, void* extra_data) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, extra_data);
    StopCounting(start, 0000, "PreCallValidateAllocateDescriptorSets");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    StopCounting(start, 0000, "PreCallRecordAllocateDescriptorSets");
}

void CoreChecksInstrumented::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, VkResult result, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, result, extra_data);
    StopCounting(start, 0000, "PostCallRecordAllocateDescriptorSets");
}

bool CoreChecksInstrumented::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    StopCounting(start, 0000, "PreCallValidateFreeDescriptorSets");
    return result;
}

void CoreChecksInstrumented::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    StopCounting(start, 0000, "PreCallRecordFreeDescriptorSets");
}

void CoreChecksInstrumented::PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets, result);
    StopCounting(start, 0000, "PostCallRecordFreeDescriptorSets");
}

bool CoreChecksInstrumented::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    StopCounting(start, 0000, "PreCallValidateUpdateDescriptorSets");
    return result;
}

void CoreChecksInstrumented::PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    StopCounting(start, 0000, "PreCallRecordUpdateDescriptorSets");
}

void CoreChecksInstrumented::PostCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    StopCounting(start, 0000, "PostCallRecordUpdateDescriptorSets");
}

bool CoreChecksInstrumented::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    StopCounting(start, 0000, "PreCallValidateCreateFramebuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    StopCounting(start, 0000, "PreCallRecordCreateFramebuffer");
}

void CoreChecksInstrumented::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer, result);
    StopCounting(start, 0000, "PostCallRecordCreateFramebuffer");
}

bool CoreChecksInstrumented::PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyFramebuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyFramebuffer");
}

void CoreChecksInstrumented::PostCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyFramebuffer");
}

bool CoreChecksInstrumented::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    StopCounting(start, 0000, "PreCallValidateCreateRenderPass");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    StopCounting(start, 0000, "PreCallRecordCreateRenderPass");
}

void CoreChecksInstrumented::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass, result);
    StopCounting(start, 0000, "PostCallRecordCreateRenderPass");
}

bool CoreChecksInstrumented::PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyRenderPass(device, renderPass, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyRenderPass");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyRenderPass(device, renderPass, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyRenderPass");
}

void CoreChecksInstrumented::PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyRenderPass(device, renderPass, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyRenderPass");
}

bool CoreChecksInstrumented::PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetRenderAreaGranularity(device, renderPass, pGranularity);
    StopCounting(start, 0000, "PreCallValidateGetRenderAreaGranularity");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
    StopCounting(start, 0000, "PreCallRecordGetRenderAreaGranularity");
}

void CoreChecksInstrumented::PostCallRecordGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
    StopCounting(start, 0000, "PostCallRecordGetRenderAreaGranularity");
}

bool CoreChecksInstrumented::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    StopCounting(start, 0000, "PreCallValidateCreateCommandPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    StopCounting(start, 0000, "PreCallRecordCreateCommandPool");
}

void CoreChecksInstrumented::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool, result);
    StopCounting(start, 0000, "PostCallRecordCreateCommandPool");
}

bool CoreChecksInstrumented::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyCommandPool(device, commandPool, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyCommandPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyCommandPool");
}

void CoreChecksInstrumented::PostCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyCommandPool");
}

bool CoreChecksInstrumented::PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateResetCommandPool(device, commandPool, flags);
    StopCounting(start, 0000, "PreCallValidateResetCommandPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordResetCommandPool(device, commandPool, flags);
    StopCounting(start, 0000, "PreCallRecordResetCommandPool");
}

void CoreChecksInstrumented::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordResetCommandPool(device, commandPool, flags, result);
    StopCounting(start, 0000, "PostCallRecordResetCommandPool");
}

bool CoreChecksInstrumented::PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    StopCounting(start, 0000, "PreCallValidateAllocateCommandBuffers");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    StopCounting(start, 0000, "PreCallRecordAllocateCommandBuffers");
}

void CoreChecksInstrumented::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers, result);
    StopCounting(start, 0000, "PostCallRecordAllocateCommandBuffers");
}

bool CoreChecksInstrumented::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    StopCounting(start, 0000, "PreCallValidateFreeCommandBuffers");
    return result;
}

void CoreChecksInstrumented::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    StopCounting(start, 0000, "PreCallRecordFreeCommandBuffers");
}

void CoreChecksInstrumented::PostCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    StopCounting(start, 0000, "PostCallRecordFreeCommandBuffers");
}

bool CoreChecksInstrumented::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo);
    StopCounting(start, 0000, "PreCallValidateBeginCommandBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
    StopCounting(start, 0000, "PreCallRecordBeginCommandBuffer");
}

void CoreChecksInstrumented::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);
    StopCounting(start, 0000, "PostCallRecordBeginCommandBuffer");
}

bool CoreChecksInstrumented::PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEndCommandBuffer(commandBuffer);
    StopCounting(start, 0000, "PreCallValidateEndCommandBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEndCommandBuffer(commandBuffer);
    StopCounting(start, 0000, "PreCallRecordEndCommandBuffer");
}

void CoreChecksInstrumented::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEndCommandBuffer(commandBuffer, result);
    StopCounting(start, 0000, "PostCallRecordEndCommandBuffer");
}

bool CoreChecksInstrumented::PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateResetCommandBuffer(commandBuffer, flags);
    StopCounting(start, 0000, "PreCallValidateResetCommandBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordResetCommandBuffer(commandBuffer, flags);
    StopCounting(start, 0000, "PreCallRecordResetCommandBuffer");
}

void CoreChecksInstrumented::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordResetCommandBuffer(commandBuffer, flags, result);
    StopCounting(start, 0000, "PostCallRecordResetCommandBuffer");
}

bool CoreChecksInstrumented::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    StopCounting(start, 0000, "PreCallValidateCmdBindPipeline");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    StopCounting(start, 0000, "PreCallRecordCmdBindPipeline");
}

void CoreChecksInstrumented::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    StopCounting(start, 0000, "PostCallRecordCmdBindPipeline");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    StopCounting(start, 0000, "PreCallValidateCmdSetViewport");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    StopCounting(start, 0000, "PreCallRecordCmdSetViewport");
}

void CoreChecksInstrumented::PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    StopCounting(start, 0000, "PostCallRecordCmdSetViewport");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    StopCounting(start, 0000, "PreCallValidateCmdSetScissor");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    StopCounting(start, 0000, "PreCallRecordCmdSetScissor");
}

void CoreChecksInstrumented::PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    StopCounting(start, 0000, "PostCallRecordCmdSetScissor");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth);
    StopCounting(start, 0000, "PreCallValidateCmdSetLineWidth");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    StopCounting(start, 0000, "PreCallRecordCmdSetLineWidth");
}

void CoreChecksInstrumented::PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    StopCounting(start, 0000, "PostCallRecordCmdSetLineWidth");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    StopCounting(start, 0000, "PreCallValidateCmdSetDepthBias");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    StopCounting(start, 0000, "PreCallRecordCmdSetDepthBias");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    StopCounting(start, 0000, "PostCallRecordCmdSetDepthBias");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants);
    StopCounting(start, 0000, "PreCallValidateCmdSetBlendConstants");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    StopCounting(start, 0000, "PreCallRecordCmdSetBlendConstants");
}

void CoreChecksInstrumented::PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    StopCounting(start, 0000, "PostCallRecordCmdSetBlendConstants");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    StopCounting(start, 0000, "PreCallValidateCmdSetDepthBounds");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    StopCounting(start, 0000, "PreCallRecordCmdSetDepthBounds");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    StopCounting(start, 0000, "PostCallRecordCmdSetDepthBounds");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    StopCounting(start, 0000, "PreCallValidateCmdSetStencilCompareMask");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    StopCounting(start, 0000, "PreCallRecordCmdSetStencilCompareMask");
}

void CoreChecksInstrumented::PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    StopCounting(start, 0000, "PostCallRecordCmdSetStencilCompareMask");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    StopCounting(start, 0000, "PreCallValidateCmdSetStencilWriteMask");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    StopCounting(start, 0000, "PreCallRecordCmdSetStencilWriteMask");
}

void CoreChecksInstrumented::PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    StopCounting(start, 0000, "PostCallRecordCmdSetStencilWriteMask");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference);
    StopCounting(start, 0000, "PreCallValidateCmdSetStencilReference");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    StopCounting(start, 0000, "PreCallRecordCmdSetStencilReference");
}

void CoreChecksInstrumented::PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    StopCounting(start, 0000, "PostCallRecordCmdSetStencilReference");
}

bool CoreChecksInstrumented::PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    StopCounting(start, 0000, "PreCallValidateCmdBindDescriptorSets");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    StopCounting(start, 0000, "PreCallRecordCmdBindDescriptorSets");
}

void CoreChecksInstrumented::PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    StopCounting(start, 0000, "PostCallRecordCmdBindDescriptorSets");
}

bool CoreChecksInstrumented::PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    StopCounting(start, 0000, "PreCallValidateCmdBindIndexBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    StopCounting(start, 0000, "PreCallRecordCmdBindIndexBuffer");
}

void CoreChecksInstrumented::PostCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    StopCounting(start, 0000, "PostCallRecordCmdBindIndexBuffer");
}

bool CoreChecksInstrumented::PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    StopCounting(start, 0000, "PreCallValidateCmdBindVertexBuffers");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    StopCounting(start, 0000, "PreCallRecordCmdBindVertexBuffers");
}

void CoreChecksInstrumented::PostCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    StopCounting(start, 0000, "PostCallRecordCmdBindVertexBuffers");
}

bool CoreChecksInstrumented::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    StopCounting(start, 0000, "PreCallValidateCmdDraw");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    StopCounting(start, 0000, "PreCallRecordCmdDraw");
}

void CoreChecksInstrumented::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    StopCounting(start, 0000, "PostCallRecordCmdDraw");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndexed");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndexed");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndexed");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndirect");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndirect");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndirect");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndexedIndirect");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndexedIndirect");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndexedIndirect");
}

bool CoreChecksInstrumented::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PreCallValidateCmdDispatch");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PreCallRecordCmdDispatch");
}

void CoreChecksInstrumented::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PostCallRecordCmdDispatch");
}

bool CoreChecksInstrumented::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
    StopCounting(start, 0000, "PreCallValidateCmdDispatchIndirect");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    StopCounting(start, 0000, "PreCallRecordCmdDispatchIndirect");
}

void CoreChecksInstrumented::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    StopCounting(start, 0000, "PostCallRecordCmdDispatchIndirect");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallValidateCmdCopyBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallRecordCmdCopyBuffer");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    StopCounting(start, 0000, "PostCallRecordCmdCopyBuffer");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallValidateCmdCopyImage");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallRecordCmdCopyImage");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PostCallRecordCmdCopyImage");
}

bool CoreChecksInstrumented::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    StopCounting(start, 0000, "PreCallValidateCmdBlitImage");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    StopCounting(start, 0000, "PreCallRecordCmdBlitImage");
}

void CoreChecksInstrumented::PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    StopCounting(start, 0000, "PostCallRecordCmdBlitImage");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallValidateCmdCopyBufferToImage");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallRecordCmdCopyBufferToImage");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PostCallRecordCmdCopyBufferToImage");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallValidateCmdCopyImageToBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallRecordCmdCopyImageToBuffer");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    StopCounting(start, 0000, "PostCallRecordCmdCopyImageToBuffer");
}

bool CoreChecksInstrumented::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    StopCounting(start, 0000, "PreCallValidateCmdUpdateBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    StopCounting(start, 0000, "PreCallRecordCmdUpdateBuffer");
}

void CoreChecksInstrumented::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    StopCounting(start, 0000, "PostCallRecordCmdUpdateBuffer");
}

bool CoreChecksInstrumented::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    StopCounting(start, 0000, "PreCallValidateCmdFillBuffer");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    StopCounting(start, 0000, "PreCallRecordCmdFillBuffer");
}

void CoreChecksInstrumented::PostCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    StopCounting(start, 0000, "PostCallRecordCmdFillBuffer");
}

bool CoreChecksInstrumented::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    StopCounting(start, 0000, "PreCallValidateCmdClearColorImage");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    StopCounting(start, 0000, "PreCallRecordCmdClearColorImage");
}

void CoreChecksInstrumented::PostCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    StopCounting(start, 0000, "PostCallRecordCmdClearColorImage");
}

bool CoreChecksInstrumented::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    StopCounting(start, 0000, "PreCallValidateCmdClearDepthStencilImage");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    StopCounting(start, 0000, "PreCallRecordCmdClearDepthStencilImage");
}

void CoreChecksInstrumented::PostCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    StopCounting(start, 0000, "PostCallRecordCmdClearDepthStencilImage");
}

bool CoreChecksInstrumented::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    StopCounting(start, 0000, "PreCallValidateCmdClearAttachments");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    StopCounting(start, 0000, "PreCallRecordCmdClearAttachments");
}

void CoreChecksInstrumented::PostCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    StopCounting(start, 0000, "PostCallRecordCmdClearAttachments");
}

bool CoreChecksInstrumented::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallValidateCmdResolveImage");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PreCallRecordCmdResolveImage");
}

void CoreChecksInstrumented::PostCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    StopCounting(start, 0000, "PostCallRecordCmdResolveImage");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetEvent(commandBuffer, event, stageMask);
    StopCounting(start, 0000, "PreCallValidateCmdSetEvent");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    StopCounting(start, 0000, "PreCallRecordCmdSetEvent");
}

void CoreChecksInstrumented::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    StopCounting(start, 0000, "PostCallRecordCmdSetEvent");
}

bool CoreChecksInstrumented::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdResetEvent(commandBuffer, event, stageMask);
    StopCounting(start, 0000, "PreCallValidateCmdResetEvent");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    StopCounting(start, 0000, "PreCallRecordCmdResetEvent");
}

void CoreChecksInstrumented::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    StopCounting(start, 0000, "PostCallRecordCmdResetEvent");
}

bool CoreChecksInstrumented::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    StopCounting(start, 0000, "PreCallValidateCmdWaitEvents");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    StopCounting(start, 0000, "PreCallRecordCmdWaitEvents");
}

void CoreChecksInstrumented::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    StopCounting(start, 0000, "PostCallRecordCmdWaitEvents");
}

bool CoreChecksInstrumented::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    StopCounting(start, 0000, "PreCallValidateCmdPipelineBarrier");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    StopCounting(start, 0000, "PreCallRecordCmdPipelineBarrier");
}

void CoreChecksInstrumented::PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    StopCounting(start, 0000, "PostCallRecordCmdPipelineBarrier");
}

bool CoreChecksInstrumented::PreCallValidateCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBeginQuery(commandBuffer, queryPool, query, flags);
    StopCounting(start, 0000, "PreCallValidateCmdBeginQuery");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
    StopCounting(start, 0000, "PreCallRecordCmdBeginQuery");
}

void CoreChecksInstrumented::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
    StopCounting(start, 0000, "PostCallRecordCmdBeginQuery");
}

bool CoreChecksInstrumented::PreCallValidateCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdEndQuery(commandBuffer, queryPool, query);
    StopCounting(start, 0000, "PreCallValidateCmdEndQuery");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdEndQuery(commandBuffer, queryPool, query);
    StopCounting(start, 0000, "PreCallRecordCmdEndQuery");
}

void CoreChecksInstrumented::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdEndQuery(commandBuffer, queryPool, query);
    StopCounting(start, 0000, "PostCallRecordCmdEndQuery");
}

bool CoreChecksInstrumented::PreCallValidateCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PreCallValidateCmdResetQueryPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PreCallRecordCmdResetQueryPool");
}

void CoreChecksInstrumented::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PostCallRecordCmdResetQueryPool");
}

bool CoreChecksInstrumented::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    StopCounting(start, 0000, "PreCallValidateCmdWriteTimestamp");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    StopCounting(start, 0000, "PreCallRecordCmdWriteTimestamp");
}

void CoreChecksInstrumented::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    StopCounting(start, 0000, "PostCallRecordCmdWriteTimestamp");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    StopCounting(start, 0000, "PreCallValidateCmdCopyQueryPoolResults");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    StopCounting(start, 0000, "PreCallRecordCmdCopyQueryPoolResults");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    StopCounting(start, 0000, "PostCallRecordCmdCopyQueryPoolResults");
}

bool CoreChecksInstrumented::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    StopCounting(start, 0000, "PreCallValidateCmdPushConstants");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    StopCounting(start, 0000, "PreCallRecordCmdPushConstants");
}

void CoreChecksInstrumented::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    StopCounting(start, 0000, "PostCallRecordCmdPushConstants");
}

bool CoreChecksInstrumented::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    StopCounting(start, 0000, "PreCallValidateCmdBeginRenderPass");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    StopCounting(start, 0000, "PreCallRecordCmdBeginRenderPass");
}

void CoreChecksInstrumented::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    StopCounting(start, 0000, "PostCallRecordCmdBeginRenderPass");
}

bool CoreChecksInstrumented::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdNextSubpass(commandBuffer, contents);
    StopCounting(start, 0000, "PreCallValidateCmdNextSubpass");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdNextSubpass(commandBuffer, contents);
    StopCounting(start, 0000, "PreCallRecordCmdNextSubpass");
}

void CoreChecksInstrumented::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdNextSubpass(commandBuffer, contents);
    StopCounting(start, 0000, "PostCallRecordCmdNextSubpass");
}

bool CoreChecksInstrumented::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdEndRenderPass(commandBuffer);
    StopCounting(start, 0000, "PreCallValidateCmdEndRenderPass");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdEndRenderPass(commandBuffer);
    StopCounting(start, 0000, "PreCallRecordCmdEndRenderPass");
}

void CoreChecksInstrumented::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdEndRenderPass(commandBuffer);
    StopCounting(start, 0000, "PostCallRecordCmdEndRenderPass");
}

bool CoreChecksInstrumented::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    StopCounting(start, 0000, "PreCallValidateCmdExecuteCommands");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    StopCounting(start, 0000, "PreCallRecordCmdExecuteCommands");
}

void CoreChecksInstrumented::PostCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    StopCounting(start, 0000, "PostCallRecordCmdExecuteCommands");
}

bool CoreChecksInstrumented::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallValidateBindBufferMemory2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallRecordBindBufferMemory2");
}

void CoreChecksInstrumented::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, result);
    StopCounting(start, 0000, "PostCallRecordBindBufferMemory2");
}

bool CoreChecksInstrumented::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallValidateBindImageMemory2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallRecordBindImageMemory2");
}

void CoreChecksInstrumented::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result);
    StopCounting(start, 0000, "PostCallRecordBindImageMemory2");
}

bool CoreChecksInstrumented::PreCallValidateGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    StopCounting(start, 0000, "PreCallValidateGetDeviceGroupPeerMemoryFeatures");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    StopCounting(start, 0000, "PreCallRecordGetDeviceGroupPeerMemoryFeatures");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    StopCounting(start, 0000, "PostCallRecordGetDeviceGroupPeerMemoryFeatures");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask);
    StopCounting(start, 0000, "PreCallValidateCmdSetDeviceMask");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
    StopCounting(start, 0000, "PreCallRecordCmdSetDeviceMask");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
    StopCounting(start, 0000, "PostCallRecordCmdSetDeviceMask");
}

bool CoreChecksInstrumented::PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PreCallValidateCmdDispatchBase");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PreCallRecordCmdDispatchBase");
}

void CoreChecksInstrumented::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PostCallRecordCmdDispatchBase");
}

bool CoreChecksInstrumented::PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    StopCounting(start, 0000, "PreCallValidateEnumeratePhysicalDeviceGroups");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    StopCounting(start, 0000, "PreCallRecordEnumeratePhysicalDeviceGroups");
}

void CoreChecksInstrumented::PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    StopCounting(start, 0000, "PostCallRecordEnumeratePhysicalDeviceGroups");
}

bool CoreChecksInstrumented::PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetImageMemoryRequirements2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetImageMemoryRequirements2");
}

void CoreChecksInstrumented::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetImageMemoryRequirements2");
}

bool CoreChecksInstrumented::PreCallValidateGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetBufferMemoryRequirements2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetBufferMemoryRequirements2");
}

void CoreChecksInstrumented::PostCallRecordGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetBufferMemoryRequirements2");
}

bool CoreChecksInstrumented::PreCallValidateGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetImageSparseMemoryRequirements2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetImageSparseMemoryRequirements2");
}

void CoreChecksInstrumented::PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetImageSparseMemoryRequirements2");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceFeatures2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceFeatures2");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceFeatures2");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceProperties2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceProperties2");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceProperties2");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceFormatProperties2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceFormatProperties2");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceFormatProperties2");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceImageFormatProperties2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceImageFormatProperties2");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceImageFormatProperties2");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceQueueFamilyProperties2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceQueueFamilyProperties2");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceQueueFamilyProperties2");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceMemoryProperties2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceMemoryProperties2");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceMemoryProperties2");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2");
}

bool CoreChecksInstrumented::PreCallValidateTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateTrimCommandPool(device, commandPool, flags);
    StopCounting(start, 0000, "PreCallValidateTrimCommandPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordTrimCommandPool(device, commandPool, flags);
    StopCounting(start, 0000, "PreCallRecordTrimCommandPool");
}

void CoreChecksInstrumented::PostCallRecordTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordTrimCommandPool(device, commandPool, flags);
    StopCounting(start, 0000, "PostCallRecordTrimCommandPool");
}

bool CoreChecksInstrumented::PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue);
    StopCounting(start, 0000, "PreCallValidateGetDeviceQueue2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    StopCounting(start, 0000, "PreCallRecordGetDeviceQueue2");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    StopCounting(start, 0000, "PostCallRecordGetDeviceQueue2");
}

bool CoreChecksInstrumented::PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    StopCounting(start, 0000, "PreCallValidateCreateSamplerYcbcrConversion");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    StopCounting(start, 0000, "PreCallRecordCreateSamplerYcbcrConversion");
}

void CoreChecksInstrumented::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    StopCounting(start, 0000, "PostCallRecordCreateSamplerYcbcrConversion");
}

bool CoreChecksInstrumented::PreCallValidateDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroySamplerYcbcrConversion");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroySamplerYcbcrConversion");
}

void CoreChecksInstrumented::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroySamplerYcbcrConversion");
}

bool CoreChecksInstrumented::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    StopCounting(start, 0000, "PreCallValidateCreateDescriptorUpdateTemplate");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    StopCounting(start, 0000, "PreCallRecordCreateDescriptorUpdateTemplate");
}

void CoreChecksInstrumented::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    StopCounting(start, 0000, "PostCallRecordCreateDescriptorUpdateTemplate");
}

bool CoreChecksInstrumented::PreCallValidateDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyDescriptorUpdateTemplate");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyDescriptorUpdateTemplate");
}

void CoreChecksInstrumented::PostCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyDescriptorUpdateTemplate");
}

bool CoreChecksInstrumented::PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    StopCounting(start, 0000, "PreCallValidateUpdateDescriptorSetWithTemplate");
    return result;
}

void CoreChecksInstrumented::PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    StopCounting(start, 0000, "PreCallRecordUpdateDescriptorSetWithTemplate");
}

void CoreChecksInstrumented::PostCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    StopCounting(start, 0000, "PostCallRecordUpdateDescriptorSetWithTemplate");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceExternalBufferProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceExternalBufferProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceExternalBufferProperties");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceExternalFenceProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceExternalFenceProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceExternalFenceProperties");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties");
}

bool CoreChecksInstrumented::PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    StopCounting(start, 0000, "PreCallValidateGetDescriptorSetLayoutSupport");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    StopCounting(start, 0000, "PreCallRecordGetDescriptorSetLayoutSupport");
}

void CoreChecksInstrumented::PostCallRecordGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    StopCounting(start, 0000, "PostCallRecordGetDescriptorSetLayoutSupport");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndirectCount");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndirectCount");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndirectCount");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndexedIndirectCount");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndexedIndirectCount");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndexedIndirectCount");
}

bool CoreChecksInstrumented::PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    StopCounting(start, 0000, "PreCallValidateCreateRenderPass2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    StopCounting(start, 0000, "PreCallRecordCreateRenderPass2");
}

void CoreChecksInstrumented::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, result);
    StopCounting(start, 0000, "PostCallRecordCreateRenderPass2");
}

bool CoreChecksInstrumented::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    StopCounting(start, 0000, "PreCallValidateCmdBeginRenderPass2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    StopCounting(start, 0000, "PreCallRecordCmdBeginRenderPass2");
}

void CoreChecksInstrumented::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    StopCounting(start, 0000, "PostCallRecordCmdBeginRenderPass2");
}

bool CoreChecksInstrumented::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    StopCounting(start, 0000, "PreCallValidateCmdNextSubpass2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    StopCounting(start, 0000, "PreCallRecordCmdNextSubpass2");
}

void CoreChecksInstrumented::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    StopCounting(start, 0000, "PostCallRecordCmdNextSubpass2");
}

bool CoreChecksInstrumented::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    StopCounting(start, 0000, "PreCallValidateCmdEndRenderPass2");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    StopCounting(start, 0000, "PreCallRecordCmdEndRenderPass2");
}

void CoreChecksInstrumented::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    StopCounting(start, 0000, "PostCallRecordCmdEndRenderPass2");
}

bool CoreChecksInstrumented::PreCallValidateResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PreCallValidateResetQueryPool");
    return result;
}

void CoreChecksInstrumented::PreCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PreCallRecordResetQueryPool");
}

void CoreChecksInstrumented::PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PostCallRecordResetQueryPool");
}

bool CoreChecksInstrumented::PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue);
    StopCounting(start, 0000, "PreCallValidateGetSemaphoreCounterValue");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue);
    StopCounting(start, 0000, "PreCallRecordGetSemaphoreCounterValue");
}

void CoreChecksInstrumented::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, result);
    StopCounting(start, 0000, "PostCallRecordGetSemaphoreCounterValue");
}

bool CoreChecksInstrumented::PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateWaitSemaphores(device, pWaitInfo, timeout);
    StopCounting(start, 0000, "PreCallValidateWaitSemaphores");
    return result;
}

void CoreChecksInstrumented::PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordWaitSemaphores(device, pWaitInfo, timeout);
    StopCounting(start, 0000, "PreCallRecordWaitSemaphores");
}

void CoreChecksInstrumented::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, result);
    StopCounting(start, 0000, "PostCallRecordWaitSemaphores");
}

bool CoreChecksInstrumented::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSignalSemaphore(device, pSignalInfo);
    StopCounting(start, 0000, "PreCallValidateSignalSemaphore");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSignalSemaphore(device, pSignalInfo);
    StopCounting(start, 0000, "PreCallRecordSignalSemaphore");
}

void CoreChecksInstrumented::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSignalSemaphore(device, pSignalInfo, result);
    StopCounting(start, 0000, "PostCallRecordSignalSemaphore");
}

bool CoreChecksInstrumented::PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetBufferDeviceAddress(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetBufferDeviceAddress");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetBufferDeviceAddress(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetBufferDeviceAddress");
}

void CoreChecksInstrumented::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetBufferDeviceAddress(device, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordGetBufferDeviceAddress");
}

bool CoreChecksInstrumented::PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetBufferOpaqueCaptureAddress");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetBufferOpaqueCaptureAddress");
}

void CoreChecksInstrumented::PostCallRecordGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
    StopCounting(start, 0000, "PostCallRecordGetBufferOpaqueCaptureAddress");
}

bool CoreChecksInstrumented::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetDeviceMemoryOpaqueCaptureAddress");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetDeviceMemoryOpaqueCaptureAddress");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    StopCounting(start, 0000, "PostCallRecordGetDeviceMemoryOpaqueCaptureAddress");
}

bool CoreChecksInstrumented::PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroySurfaceKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroySurfaceKHR");
}

void CoreChecksInstrumented::PostCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroySurfaceKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSurfaceSupportKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSurfaceSupportKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSurfaceSupportKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR");
}

bool CoreChecksInstrumented::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    StopCounting(start, 0000, "PreCallValidateCreateSwapchainKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    StopCounting(start, 0000, "PreCallRecordCreateSwapchainKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain, result);
    StopCounting(start, 0000, "PostCallRecordCreateSwapchainKHR");
}

bool CoreChecksInstrumented::PreCallValidateDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroySwapchainKHR(device, swapchain, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroySwapchainKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroySwapchainKHR");
}

void CoreChecksInstrumented::PostCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroySwapchainKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    StopCounting(start, 0000, "PreCallValidateGetSwapchainImagesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    StopCounting(start, 0000, "PreCallRecordGetSwapchainImagesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, result);
    StopCounting(start, 0000, "PostCallRecordGetSwapchainImagesKHR");
}

bool CoreChecksInstrumented::PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    StopCounting(start, 0000, "PreCallValidateAcquireNextImageKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    StopCounting(start, 0000, "PreCallRecordAcquireNextImageKHR");
}

void CoreChecksInstrumented::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, result);
    StopCounting(start, 0000, "PostCallRecordAcquireNextImageKHR");
}

bool CoreChecksInstrumented::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateQueuePresentKHR(queue, pPresentInfo);
    StopCounting(start, 0000, "PreCallValidateQueuePresentKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordQueuePresentKHR(queue, pPresentInfo);
    StopCounting(start, 0000, "PreCallRecordQueuePresentKHR");
}


bool CoreChecksInstrumented::PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    StopCounting(start, 0000, "PreCallValidateGetDeviceGroupPresentCapabilitiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    StopCounting(start, 0000, "PreCallRecordGetDeviceGroupPresentCapabilitiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities, result);
    StopCounting(start, 0000, "PostCallRecordGetDeviceGroupPresentCapabilitiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    StopCounting(start, 0000, "PreCallValidateGetDeviceGroupSurfacePresentModesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    StopCounting(start, 0000, "PreCallRecordGetDeviceGroupSurfacePresentModesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes, result);
    StopCounting(start, 0000, "PostCallRecordGetDeviceGroupSurfacePresentModesKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDevicePresentRectanglesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDevicePresentRectanglesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDevicePresentRectanglesKHR");
}

bool CoreChecksInstrumented::PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    StopCounting(start, 0000, "PreCallValidateAcquireNextImage2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    StopCounting(start, 0000, "PreCallRecordAcquireNextImage2KHR");
}

void CoreChecksInstrumented::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, result);
    StopCounting(start, 0000, "PostCallRecordAcquireNextImage2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    StopCounting(start, 0000, "PreCallValidateGetDisplayPlaneSupportedDisplaysKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    StopCounting(start, 0000, "PreCallRecordGetDisplayPlaneSupportedDisplaysKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays, result);
    StopCounting(start, 0000, "PostCallRecordGetDisplayPlaneSupportedDisplaysKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetDisplayModePropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetDisplayModePropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetDisplayModePropertiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    StopCounting(start, 0000, "PreCallValidateCreateDisplayModeKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    StopCounting(start, 0000, "PreCallRecordCreateDisplayModeKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode, result);
    StopCounting(start, 0000, "PostCallRecordCreateDisplayModeKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    StopCounting(start, 0000, "PreCallValidateGetDisplayPlaneCapabilitiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    StopCounting(start, 0000, "PreCallRecordGetDisplayPlaneCapabilitiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities, result);
    StopCounting(start, 0000, "PostCallRecordGetDisplayPlaneCapabilitiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateDisplayPlaneSurfaceKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateDisplayPlaneSurfaceKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateDisplayPlaneSurfaceKHR");
}

bool CoreChecksInstrumented::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    StopCounting(start, 0000, "PreCallValidateCreateSharedSwapchainsKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    StopCounting(start, 0000, "PreCallRecordCreateSharedSwapchainsKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains, result);
    StopCounting(start, 0000, "PostCallRecordCreateSharedSwapchainsKHR");
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool CoreChecksInstrumented::PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateXlibSurfaceKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateXlibSurfaceKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateXlibSurfaceKHR");
}

#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR");
}

#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool CoreChecksInstrumented::PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateXcbSurfaceKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateXcbSurfaceKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateXcbSurfaceKHR");
}

#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR");
}

#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool CoreChecksInstrumented::PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateWaylandSurfaceKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateWaylandSurfaceKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateWaylandSurfaceKHR");
}

#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR");
}

#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecksInstrumented::PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateAndroidSurfaceKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateAndroidSurfaceKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateAndroidSurfaceKHR");
}

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateWin32SurfaceKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateWin32SurfaceKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateWin32SurfaceKHR");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceFeatures2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceFeatures2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceFeatures2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceFormatProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceFormatProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceFormatProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceMemoryProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceMemoryProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceMemoryProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    StopCounting(start, 0000, "PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    StopCounting(start, 0000, "PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    StopCounting(start, 0000, "PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    StopCounting(start, 0000, "PreCallValidateCmdSetDeviceMaskKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    StopCounting(start, 0000, "PreCallRecordCmdSetDeviceMaskKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    StopCounting(start, 0000, "PostCallRecordCmdSetDeviceMaskKHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PreCallValidateCmdDispatchBaseKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PreCallRecordCmdDispatchBaseKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    StopCounting(start, 0000, "PostCallRecordCmdDispatchBaseKHR");
}

bool CoreChecksInstrumented::PreCallValidateTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateTrimCommandPoolKHR(device, commandPool, flags);
    StopCounting(start, 0000, "PreCallValidateTrimCommandPoolKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordTrimCommandPoolKHR(device, commandPool, flags);
    StopCounting(start, 0000, "PreCallRecordTrimCommandPoolKHR");
}

void CoreChecksInstrumented::PostCallRecordTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordTrimCommandPoolKHR(device, commandPool, flags);
    StopCounting(start, 0000, "PostCallRecordTrimCommandPoolKHR");
}

bool CoreChecksInstrumented::PreCallValidateEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    StopCounting(start, 0000, "PreCallValidateEnumeratePhysicalDeviceGroupsKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    StopCounting(start, 0000, "PreCallRecordEnumeratePhysicalDeviceGroupsKHR");
}

void CoreChecksInstrumented::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties, result);
    StopCounting(start, 0000, "PostCallRecordEnumeratePhysicalDeviceGroupsKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    StopCounting(start, 0000, "PreCallValidateGetMemoryWin32HandleKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    StopCounting(start, 0000, "PreCallRecordGetMemoryWin32HandleKHR");
}

void CoreChecksInstrumented::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    StopCounting(start, 0000, "PostCallRecordGetMemoryWin32HandleKHR");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    StopCounting(start, 0000, "PreCallValidateGetMemoryWin32HandlePropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    StopCounting(start, 0000, "PreCallRecordGetMemoryWin32HandlePropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetMemoryWin32HandlePropertiesKHR");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetMemoryFdKHR(device, pGetFdInfo, pFd);
    StopCounting(start, 0000, "PreCallValidateGetMemoryFdKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd);
    StopCounting(start, 0000, "PreCallRecordGetMemoryFdKHR");
}

void CoreChecksInstrumented::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd, result);
    StopCounting(start, 0000, "PostCallRecordGetMemoryFdKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    StopCounting(start, 0000, "PreCallValidateGetMemoryFdPropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    StopCounting(start, 0000, "PreCallRecordGetMemoryFdPropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetMemoryFdPropertiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    StopCounting(start, 0000, "PreCallValidateImportSemaphoreWin32HandleKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    StopCounting(start, 0000, "PreCallRecordImportSemaphoreWin32HandleKHR");
}

void CoreChecksInstrumented::PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo, result);
    StopCounting(start, 0000, "PostCallRecordImportSemaphoreWin32HandleKHR");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    StopCounting(start, 0000, "PreCallValidateGetSemaphoreWin32HandleKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    StopCounting(start, 0000, "PreCallRecordGetSemaphoreWin32HandleKHR");
}

void CoreChecksInstrumented::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    StopCounting(start, 0000, "PostCallRecordGetSemaphoreWin32HandleKHR");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    StopCounting(start, 0000, "PreCallValidateImportSemaphoreFdKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    StopCounting(start, 0000, "PreCallRecordImportSemaphoreFdKHR");
}

void CoreChecksInstrumented::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo, result);
    StopCounting(start, 0000, "PostCallRecordImportSemaphoreFdKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    StopCounting(start, 0000, "PreCallValidateGetSemaphoreFdKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    StopCounting(start, 0000, "PreCallRecordGetSemaphoreFdKHR");
}

void CoreChecksInstrumented::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd, result);
    StopCounting(start, 0000, "PostCallRecordGetSemaphoreFdKHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    StopCounting(start, 0000, "PreCallValidateCmdPushDescriptorSetKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    StopCounting(start, 0000, "PreCallRecordCmdPushDescriptorSetKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    StopCounting(start, 0000, "PostCallRecordCmdPushDescriptorSetKHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    StopCounting(start, 0000, "PreCallValidateCmdPushDescriptorSetWithTemplateKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    StopCounting(start, 0000, "PreCallRecordCmdPushDescriptorSetWithTemplateKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    StopCounting(start, 0000, "PostCallRecordCmdPushDescriptorSetWithTemplateKHR");
}

bool CoreChecksInstrumented::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    StopCounting(start, 0000, "PreCallValidateCreateDescriptorUpdateTemplateKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    StopCounting(start, 0000, "PreCallRecordCreateDescriptorUpdateTemplateKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, result);
    StopCounting(start, 0000, "PostCallRecordCreateDescriptorUpdateTemplateKHR");
}

bool CoreChecksInstrumented::PreCallValidateDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyDescriptorUpdateTemplateKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyDescriptorUpdateTemplateKHR");
}

void CoreChecksInstrumented::PostCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyDescriptorUpdateTemplateKHR");
}

bool CoreChecksInstrumented::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    StopCounting(start, 0000, "PreCallValidateUpdateDescriptorSetWithTemplateKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    StopCounting(start, 0000, "PreCallRecordUpdateDescriptorSetWithTemplateKHR");
}

void CoreChecksInstrumented::PostCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    StopCounting(start, 0000, "PostCallRecordUpdateDescriptorSetWithTemplateKHR");
}

bool CoreChecksInstrumented::PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    StopCounting(start, 0000, "PreCallValidateCreateRenderPass2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    StopCounting(start, 0000, "PreCallRecordCreateRenderPass2KHR");
}

void CoreChecksInstrumented::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass, result);
    StopCounting(start, 0000, "PostCallRecordCreateRenderPass2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    StopCounting(start, 0000, "PreCallValidateCmdBeginRenderPass2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    StopCounting(start, 0000, "PreCallRecordCmdBeginRenderPass2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    StopCounting(start, 0000, "PostCallRecordCmdBeginRenderPass2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    StopCounting(start, 0000, "PreCallValidateCmdNextSubpass2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    StopCounting(start, 0000, "PreCallRecordCmdNextSubpass2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    StopCounting(start, 0000, "PostCallRecordCmdNextSubpass2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    StopCounting(start, 0000, "PreCallValidateCmdEndRenderPass2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    StopCounting(start, 0000, "PreCallRecordCmdEndRenderPass2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    StopCounting(start, 0000, "PostCallRecordCmdEndRenderPass2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetSwapchainStatusKHR(device, swapchain);
    StopCounting(start, 0000, "PreCallValidateGetSwapchainStatusKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetSwapchainStatusKHR(device, swapchain);
    StopCounting(start, 0000, "PreCallRecordGetSwapchainStatusKHR");
}

void CoreChecksInstrumented::PostCallRecordGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetSwapchainStatusKHR(device, swapchain, result);
    StopCounting(start, 0000, "PostCallRecordGetSwapchainStatusKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    StopCounting(start, 0000, "PreCallValidateImportFenceWin32HandleKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    StopCounting(start, 0000, "PreCallRecordImportFenceWin32HandleKHR");
}

void CoreChecksInstrumented::PostCallRecordImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo, result);
    StopCounting(start, 0000, "PostCallRecordImportFenceWin32HandleKHR");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    StopCounting(start, 0000, "PreCallValidateGetFenceWin32HandleKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    StopCounting(start, 0000, "PreCallRecordGetFenceWin32HandleKHR");
}

void CoreChecksInstrumented::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle, result);
    StopCounting(start, 0000, "PostCallRecordGetFenceWin32HandleKHR");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo);
    StopCounting(start, 0000, "PreCallValidateImportFenceFdKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordImportFenceFdKHR(device, pImportFenceFdInfo);
    StopCounting(start, 0000, "PreCallRecordImportFenceFdKHR");
}

void CoreChecksInstrumented::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordImportFenceFdKHR(device, pImportFenceFdInfo, result);
    StopCounting(start, 0000, "PostCallRecordImportFenceFdKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetFenceFdKHR(device, pGetFdInfo, pFd);
    StopCounting(start, 0000, "PreCallValidateGetFenceFdKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd);
    StopCounting(start, 0000, "PreCallRecordGetFenceFdKHR");
}

void CoreChecksInstrumented::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd, result);
    StopCounting(start, 0000, "PostCallRecordGetFenceFdKHR");
}

bool CoreChecksInstrumented::PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    StopCounting(start, 0000, "PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    StopCounting(start, 0000, "PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
}

void CoreChecksInstrumented::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions, result);
    StopCounting(start, 0000, "PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
}

bool CoreChecksInstrumented::PreCallValidateAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAcquireProfilingLockKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateAcquireProfilingLockKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAcquireProfilingLockKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordAcquireProfilingLockKHR");
}

void CoreChecksInstrumented::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAcquireProfilingLockKHR(device, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordAcquireProfilingLockKHR");
}

bool CoreChecksInstrumented::PreCallValidateReleaseProfilingLockKHR(VkDevice device) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateReleaseProfilingLockKHR(device);
    StopCounting(start, 0000, "PreCallValidateReleaseProfilingLockKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordReleaseProfilingLockKHR(VkDevice device) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordReleaseProfilingLockKHR(device);
    StopCounting(start, 0000, "PreCallRecordReleaseProfilingLockKHR");
}

void CoreChecksInstrumented::PostCallRecordReleaseProfilingLockKHR(VkDevice device) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordReleaseProfilingLockKHR(device);
    StopCounting(start, 0000, "PostCallRecordReleaseProfilingLockKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceDisplayProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceDisplayProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceDisplayProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetDisplayModeProperties2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetDisplayModeProperties2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetDisplayModeProperties2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    StopCounting(start, 0000, "PreCallValidateGetDisplayPlaneCapabilities2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    StopCounting(start, 0000, "PreCallRecordGetDisplayPlaneCapabilities2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities, result);
    StopCounting(start, 0000, "PostCallRecordGetDisplayPlaneCapabilities2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetImageMemoryRequirements2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetImageMemoryRequirements2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetImageMemoryRequirements2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetBufferMemoryRequirements2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetBufferMemoryRequirements2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetBufferMemoryRequirements2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetImageSparseMemoryRequirements2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetImageSparseMemoryRequirements2KHR");
}

void CoreChecksInstrumented::PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetImageSparseMemoryRequirements2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    StopCounting(start, 0000, "PreCallValidateCreateSamplerYcbcrConversionKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    StopCounting(start, 0000, "PreCallRecordCreateSamplerYcbcrConversionKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion, result);
    StopCounting(start, 0000, "PostCallRecordCreateSamplerYcbcrConversionKHR");
}

bool CoreChecksInstrumented::PreCallValidateDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroySamplerYcbcrConversionKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroySamplerYcbcrConversionKHR");
}

void CoreChecksInstrumented::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroySamplerYcbcrConversionKHR");
}

bool CoreChecksInstrumented::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallValidateBindBufferMemory2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallRecordBindBufferMemory2KHR");
}

void CoreChecksInstrumented::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos, result);
    StopCounting(start, 0000, "PostCallRecordBindBufferMemory2KHR");
}

bool CoreChecksInstrumented::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallValidateBindImageMemory2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallRecordBindImageMemory2KHR");
}

void CoreChecksInstrumented::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, result);
    StopCounting(start, 0000, "PostCallRecordBindImageMemory2KHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    StopCounting(start, 0000, "PreCallValidateGetDescriptorSetLayoutSupportKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    StopCounting(start, 0000, "PreCallRecordGetDescriptorSetLayoutSupportKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    StopCounting(start, 0000, "PostCallRecordGetDescriptorSetLayoutSupportKHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndirectCountKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndirectCountKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndirectCountKHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndexedIndirectCountKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndexedIndirectCountKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndexedIndirectCountKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    StopCounting(start, 0000, "PreCallValidateGetSemaphoreCounterValueKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    StopCounting(start, 0000, "PreCallRecordGetSemaphoreCounterValueKHR");
}

void CoreChecksInstrumented::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue, result);
    StopCounting(start, 0000, "PostCallRecordGetSemaphoreCounterValueKHR");
}

bool CoreChecksInstrumented::PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateWaitSemaphoresKHR(device, pWaitInfo, timeout);
    StopCounting(start, 0000, "PreCallValidateWaitSemaphoresKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout);
    StopCounting(start, 0000, "PreCallRecordWaitSemaphoresKHR");
}

void CoreChecksInstrumented::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout, result);
    StopCounting(start, 0000, "PostCallRecordWaitSemaphoresKHR");
}

bool CoreChecksInstrumented::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSignalSemaphoreKHR(device, pSignalInfo);
    StopCounting(start, 0000, "PreCallValidateSignalSemaphoreKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSignalSemaphoreKHR(device, pSignalInfo);
    StopCounting(start, 0000, "PreCallRecordSignalSemaphoreKHR");
}

void CoreChecksInstrumented::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSignalSemaphoreKHR(device, pSignalInfo, result);
    StopCounting(start, 0000, "PostCallRecordSignalSemaphoreKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetBufferDeviceAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetBufferDeviceAddressKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetBufferDeviceAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetBufferDeviceAddressKHR");
}

void CoreChecksInstrumented::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetBufferDeviceAddressKHR(device, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordGetBufferDeviceAddressKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetBufferOpaqueCaptureAddressKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetBufferOpaqueCaptureAddressKHR");
}

void CoreChecksInstrumented::PostCallRecordGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PostCallRecordGetBufferOpaqueCaptureAddressKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR");
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    StopCounting(start, 0000, "PreCallValidateCreateDeferredOperationKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    StopCounting(start, 0000, "PreCallRecordCreateDeferredOperationKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation, result);
    StopCounting(start, 0000, "PostCallRecordCreateDeferredOperationKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyDeferredOperationKHR(device, operation, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyDeferredOperationKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyDeferredOperationKHR");
}

void CoreChecksInstrumented::PostCallRecordDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyDeferredOperationKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeferredOperationMaxConcurrencyKHR(device, operation);
    StopCounting(start, 0000, "PreCallValidateGetDeferredOperationMaxConcurrencyKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
    StopCounting(start, 0000, "PreCallRecordGetDeferredOperationMaxConcurrencyKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
    StopCounting(start, 0000, "PostCallRecordGetDeferredOperationMaxConcurrencyKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeferredOperationResultKHR(device, operation);
    StopCounting(start, 0000, "PreCallValidateGetDeferredOperationResultKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeferredOperationResultKHR(device, operation);
    StopCounting(start, 0000, "PreCallRecordGetDeferredOperationResultKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeferredOperationResultKHR(device, operation, result);
    StopCounting(start, 0000, "PostCallRecordGetDeferredOperationResultKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDeferredOperationJoinKHR(device, operation);
    StopCounting(start, 0000, "PreCallValidateDeferredOperationJoinKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDeferredOperationJoinKHR(device, operation);
    StopCounting(start, 0000, "PreCallRecordDeferredOperationJoinKHR");
}

void CoreChecksInstrumented::PostCallRecordDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDeferredOperationJoinKHR(device, operation, result);
    StopCounting(start, 0000, "PostCallRecordDeferredOperationJoinKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice                        device, const VkPipelineInfoKHR*        pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPipelineExecutablePropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPipelineExecutablePropertiesKHR(VkDevice                        device, const VkPipelineInfoKHR*        pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPipelineExecutablePropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPipelineExecutablePropertiesKHR(VkDevice                        device, const VkPipelineInfoKHR*        pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPipelineExecutablePropertiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    StopCounting(start, 0000, "PreCallValidateGetPipelineExecutableStatisticsKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPipelineExecutableStatisticsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    StopCounting(start, 0000, "PreCallRecordGetPipelineExecutableStatisticsKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPipelineExecutableStatisticsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics, result);
    StopCounting(start, 0000, "PostCallRecordGetPipelineExecutableStatisticsKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    StopCounting(start, 0000, "PreCallValidateGetPipelineExecutableInternalRepresentationsKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    StopCounting(start, 0000, "PreCallRecordGetPipelineExecutableInternalRepresentationsKHR");
}

void CoreChecksInstrumented::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(VkDevice                        device, const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations, result);
    StopCounting(start, 0000, "PostCallRecordGetPipelineExecutableInternalRepresentationsKHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    StopCounting(start, 0000, "PreCallValidateCmdCopyBuffer2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    StopCounting(start, 0000, "PreCallRecordCmdCopyBuffer2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    StopCounting(start, 0000, "PostCallRecordCmdCopyBuffer2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    StopCounting(start, 0000, "PreCallValidateCmdCopyImage2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    StopCounting(start, 0000, "PreCallRecordCmdCopyImage2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    StopCounting(start, 0000, "PostCallRecordCmdCopyImage2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    StopCounting(start, 0000, "PreCallValidateCmdCopyBufferToImage2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    StopCounting(start, 0000, "PreCallRecordCmdCopyBufferToImage2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    StopCounting(start, 0000, "PostCallRecordCmdCopyBufferToImage2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    StopCounting(start, 0000, "PreCallValidateCmdCopyImageToBuffer2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    StopCounting(start, 0000, "PreCallRecordCmdCopyImageToBuffer2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    StopCounting(start, 0000, "PostCallRecordCmdCopyImageToBuffer2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    StopCounting(start, 0000, "PreCallValidateCmdBlitImage2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    StopCounting(start, 0000, "PreCallRecordCmdBlitImage2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    StopCounting(start, 0000, "PostCallRecordCmdBlitImage2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    StopCounting(start, 0000, "PreCallValidateCmdResolveImage2KHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    StopCounting(start, 0000, "PreCallRecordCmdResolveImage2KHR");
}

void CoreChecksInstrumented::PostCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    StopCounting(start, 0000, "PostCallRecordCmdResolveImage2KHR");
}

bool CoreChecksInstrumented::PreCallValidateCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    StopCounting(start, 0000, "PreCallValidateCreateDebugReportCallbackEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    StopCounting(start, 0000, "PreCallRecordCreateDebugReportCallbackEXT");
}

void CoreChecksInstrumented::PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback, result);
    StopCounting(start, 0000, "PostCallRecordCreateDebugReportCallbackEXT");
}

bool CoreChecksInstrumented::PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyDebugReportCallbackEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyDebugReportCallbackEXT");
}

void CoreChecksInstrumented::PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyDebugReportCallbackEXT");
}

bool CoreChecksInstrumented::PreCallValidateDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    StopCounting(start, 0000, "PreCallValidateDebugReportMessageEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    StopCounting(start, 0000, "PreCallRecordDebugReportMessageEXT");
}

void CoreChecksInstrumented::PostCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    StopCounting(start, 0000, "PostCallRecordDebugReportMessageEXT");
}

bool CoreChecksInstrumented::PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDebugMarkerSetObjectTagEXT(device, pTagInfo);
    StopCounting(start, 0000, "PreCallValidateDebugMarkerSetObjectTagEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo);
    StopCounting(start, 0000, "PreCallRecordDebugMarkerSetObjectTagEXT");
}

void CoreChecksInstrumented::PostCallRecordDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo, result);
    StopCounting(start, 0000, "PostCallRecordDebugMarkerSetObjectTagEXT");
}

bool CoreChecksInstrumented::PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDebugMarkerSetObjectNameEXT(device, pNameInfo);
    StopCounting(start, 0000, "PreCallValidateDebugMarkerSetObjectNameEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo);
    StopCounting(start, 0000, "PreCallRecordDebugMarkerSetObjectNameEXT");
}

void CoreChecksInstrumented::PostCallRecordDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo, result);
    StopCounting(start, 0000, "PostCallRecordDebugMarkerSetObjectNameEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PreCallValidateCmdDebugMarkerBeginEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PreCallRecordCmdDebugMarkerBeginEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PostCallRecordCmdDebugMarkerBeginEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDebugMarkerEndEXT(commandBuffer);
    StopCounting(start, 0000, "PreCallValidateCmdDebugMarkerEndEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDebugMarkerEndEXT(commandBuffer);
    StopCounting(start, 0000, "PreCallRecordCmdDebugMarkerEndEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDebugMarkerEndEXT(commandBuffer);
    StopCounting(start, 0000, "PostCallRecordCmdDebugMarkerEndEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PreCallValidateCmdDebugMarkerInsertEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PreCallRecordCmdDebugMarkerInsertEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PostCallRecordCmdDebugMarkerInsertEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    StopCounting(start, 0000, "PreCallValidateCmdBindTransformFeedbackBuffersEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    StopCounting(start, 0000, "PreCallRecordCmdBindTransformFeedbackBuffersEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    StopCounting(start, 0000, "PostCallRecordCmdBindTransformFeedbackBuffersEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    StopCounting(start, 0000, "PreCallValidateCmdBeginTransformFeedbackEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    StopCounting(start, 0000, "PreCallRecordCmdBeginTransformFeedbackEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    StopCounting(start, 0000, "PostCallRecordCmdBeginTransformFeedbackEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    StopCounting(start, 0000, "PreCallValidateCmdEndTransformFeedbackEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    StopCounting(start, 0000, "PreCallRecordCmdEndTransformFeedbackEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    StopCounting(start, 0000, "PostCallRecordCmdEndTransformFeedbackEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    StopCounting(start, 0000, "PreCallValidateCmdBeginQueryIndexedEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    StopCounting(start, 0000, "PreCallRecordCmdBeginQueryIndexedEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    StopCounting(start, 0000, "PostCallRecordCmdBeginQueryIndexedEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    StopCounting(start, 0000, "PreCallValidateCmdEndQueryIndexedEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    StopCounting(start, 0000, "PreCallRecordCmdEndQueryIndexedEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    StopCounting(start, 0000, "PostCallRecordCmdEndQueryIndexedEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndirectByteCountEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndirectByteCountEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndirectByteCountEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageViewHandleNVX(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetImageViewHandleNVX");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageViewHandleNVX(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetImageViewHandleNVX");
}

void CoreChecksInstrumented::PostCallRecordGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageViewHandleNVX(device, pInfo);
    StopCounting(start, 0000, "PostCallRecordGetImageViewHandleNVX");
}

bool CoreChecksInstrumented::PreCallValidateGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageViewAddressNVX(device, imageView, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetImageViewAddressNVX");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageViewAddressNVX(device, imageView, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetImageViewAddressNVX");
}

void CoreChecksInstrumented::PostCallRecordGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageViewAddressNVX(device, imageView, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetImageViewAddressNVX");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndirectCountAMD");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndirectCountAMD");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndirectCountAMD");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawIndexedIndirectCountAMD");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawIndexedIndirectCountAMD");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawIndexedIndirectCountAMD");
}

bool CoreChecksInstrumented::PreCallValidateGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetShaderInfoAMD");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetShaderInfoAMD");
}

void CoreChecksInstrumented::PostCallRecordGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordGetShaderInfoAMD");
}

#ifdef VK_USE_PLATFORM_GGP
bool CoreChecksInstrumented::PreCallValidateCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateStreamDescriptorSurfaceGGP");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateStreamDescriptorSurfaceGGP");
}

void CoreChecksInstrumented::PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateStreamDescriptorSurfaceGGP");
}

#endif // VK_USE_PLATFORM_GGP
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    StopCounting(start, 0000, "PreCallValidateGetMemoryWin32HandleNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    StopCounting(start, 0000, "PreCallRecordGetMemoryWin32HandleNV");
}

void CoreChecksInstrumented::PostCallRecordGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle, result);
    StopCounting(start, 0000, "PostCallRecordGetMemoryWin32HandleNV");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
bool CoreChecksInstrumented::PreCallValidateCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateViSurfaceNN");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateViSurfaceNN");
}

void CoreChecksInstrumented::PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateViSurfaceNN");
}

#endif // VK_USE_PLATFORM_VI_NN
bool CoreChecksInstrumented::PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    StopCounting(start, 0000, "PreCallValidateCmdBeginConditionalRenderingEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    StopCounting(start, 0000, "PreCallRecordCmdBeginConditionalRenderingEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    StopCounting(start, 0000, "PostCallRecordCmdBeginConditionalRenderingEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdEndConditionalRenderingEXT(commandBuffer);
    StopCounting(start, 0000, "PreCallValidateCmdEndConditionalRenderingEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
    StopCounting(start, 0000, "PreCallRecordCmdEndConditionalRenderingEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
    StopCounting(start, 0000, "PostCallRecordCmdEndConditionalRenderingEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    StopCounting(start, 0000, "PreCallValidateCmdSetViewportWScalingNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    StopCounting(start, 0000, "PreCallRecordCmdSetViewportWScalingNV");
}

void CoreChecksInstrumented::PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    StopCounting(start, 0000, "PostCallRecordCmdSetViewportWScalingNV");
}

bool CoreChecksInstrumented::PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateReleaseDisplayEXT(physicalDevice, display);
    StopCounting(start, 0000, "PreCallValidateReleaseDisplayEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordReleaseDisplayEXT(physicalDevice, display);
    StopCounting(start, 0000, "PreCallRecordReleaseDisplayEXT");
}

void CoreChecksInstrumented::PostCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordReleaseDisplayEXT(physicalDevice, display, result);
    StopCounting(start, 0000, "PostCallRecordReleaseDisplayEXT");
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool CoreChecksInstrumented::PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    StopCounting(start, 0000, "PreCallValidateAcquireXlibDisplayEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    StopCounting(start, 0000, "PreCallRecordAcquireXlibDisplayEXT");
}

void CoreChecksInstrumented::PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display, result);
    StopCounting(start, 0000, "PostCallRecordAcquireXlibDisplayEXT");
}

#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool CoreChecksInstrumented::PreCallValidateGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    StopCounting(start, 0000, "PreCallValidateGetRandROutputDisplayEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    StopCounting(start, 0000, "PreCallRecordGetRandROutputDisplayEXT");
}

void CoreChecksInstrumented::PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay, result);
    StopCounting(start, 0000, "PostCallRecordGetRandROutputDisplayEXT");
}

#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT");
}

bool CoreChecksInstrumented::PreCallValidateDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    StopCounting(start, 0000, "PreCallValidateDisplayPowerControlEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    StopCounting(start, 0000, "PreCallRecordDisplayPowerControlEXT");
}

void CoreChecksInstrumented::PostCallRecordDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo, result);
    StopCounting(start, 0000, "PostCallRecordDisplayPowerControlEXT");
}

bool CoreChecksInstrumented::PreCallValidateRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    StopCounting(start, 0000, "PreCallValidateRegisterDeviceEventEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    StopCounting(start, 0000, "PreCallRecordRegisterDeviceEventEXT");
}

void CoreChecksInstrumented::PostCallRecordRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence, result);
    StopCounting(start, 0000, "PostCallRecordRegisterDeviceEventEXT");
}

bool CoreChecksInstrumented::PreCallValidateRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    StopCounting(start, 0000, "PreCallValidateRegisterDisplayEventEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    StopCounting(start, 0000, "PreCallRecordRegisterDisplayEventEXT");
}

void CoreChecksInstrumented::PostCallRecordRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence, result);
    StopCounting(start, 0000, "PostCallRecordRegisterDisplayEventEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    StopCounting(start, 0000, "PreCallValidateGetSwapchainCounterEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    StopCounting(start, 0000, "PreCallRecordGetSwapchainCounterEXT");
}

void CoreChecksInstrumented::PostCallRecordGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue, result);
    StopCounting(start, 0000, "PostCallRecordGetSwapchainCounterEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    StopCounting(start, 0000, "PreCallValidateGetRefreshCycleDurationGOOGLE");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    StopCounting(start, 0000, "PreCallRecordGetRefreshCycleDurationGOOGLE");
}

void CoreChecksInstrumented::PostCallRecordGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetRefreshCycleDurationGOOGLE");
}

bool CoreChecksInstrumented::PreCallValidateGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    StopCounting(start, 0000, "PreCallValidateGetPastPresentationTimingGOOGLE");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    StopCounting(start, 0000, "PreCallRecordGetPastPresentationTimingGOOGLE");
}

void CoreChecksInstrumented::PostCallRecordGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings, result);
    StopCounting(start, 0000, "PostCallRecordGetPastPresentationTimingGOOGLE");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    StopCounting(start, 0000, "PreCallValidateCmdSetDiscardRectangleEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    StopCounting(start, 0000, "PreCallRecordCmdSetDiscardRectangleEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    StopCounting(start, 0000, "PostCallRecordCmdSetDiscardRectangleEXT");
}

bool CoreChecksInstrumented::PreCallValidateSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    StopCounting(start, 0000, "PreCallValidateSetHdrMetadataEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    StopCounting(start, 0000, "PreCallRecordSetHdrMetadataEXT");
}

void CoreChecksInstrumented::PostCallRecordSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    StopCounting(start, 0000, "PostCallRecordSetHdrMetadataEXT");
}

#ifdef VK_USE_PLATFORM_IOS_MVK
bool CoreChecksInstrumented::PreCallValidateCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateIOSSurfaceMVK");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateIOSSurfaceMVK");
}

void CoreChecksInstrumented::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateIOSSurfaceMVK");
}

#endif // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool CoreChecksInstrumented::PreCallValidateCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateMacOSSurfaceMVK");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateMacOSSurfaceMVK");
}

void CoreChecksInstrumented::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateMacOSSurfaceMVK");
}

#endif // VK_USE_PLATFORM_MACOS_MVK
bool CoreChecksInstrumented::PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo);
    StopCounting(start, 0000, "PreCallValidateSetDebugUtilsObjectNameEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
    StopCounting(start, 0000, "PreCallRecordSetDebugUtilsObjectNameEXT");
}

void CoreChecksInstrumented::PostCallRecordSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo, result);
    StopCounting(start, 0000, "PostCallRecordSetDebugUtilsObjectNameEXT");
}

bool CoreChecksInstrumented::PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo);
    StopCounting(start, 0000, "PreCallValidateSetDebugUtilsObjectTagEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo);
    StopCounting(start, 0000, "PreCallRecordSetDebugUtilsObjectTagEXT");
}

void CoreChecksInstrumented::PostCallRecordSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo, result);
    StopCounting(start, 0000, "PostCallRecordSetDebugUtilsObjectTagEXT");
}

bool CoreChecksInstrumented::PreCallValidateQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    StopCounting(start, 0000, "PreCallValidateQueueBeginDebugUtilsLabelEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    StopCounting(start, 0000, "PreCallRecordQueueBeginDebugUtilsLabelEXT");
}

void CoreChecksInstrumented::PostCallRecordQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    StopCounting(start, 0000, "PostCallRecordQueueBeginDebugUtilsLabelEXT");
}

bool CoreChecksInstrumented::PreCallValidateQueueEndDebugUtilsLabelEXT(VkQueue queue) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateQueueEndDebugUtilsLabelEXT(queue);
    StopCounting(start, 0000, "PreCallValidateQueueEndDebugUtilsLabelEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordQueueEndDebugUtilsLabelEXT(queue);
    StopCounting(start, 0000, "PreCallRecordQueueEndDebugUtilsLabelEXT");
}

void CoreChecksInstrumented::PostCallRecordQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordQueueEndDebugUtilsLabelEXT(queue);
    StopCounting(start, 0000, "PostCallRecordQueueEndDebugUtilsLabelEXT");
}

bool CoreChecksInstrumented::PreCallValidateQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    StopCounting(start, 0000, "PreCallValidateQueueInsertDebugUtilsLabelEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    StopCounting(start, 0000, "PreCallRecordQueueInsertDebugUtilsLabelEXT");
}

void CoreChecksInstrumented::PostCallRecordQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    StopCounting(start, 0000, "PostCallRecordQueueInsertDebugUtilsLabelEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    StopCounting(start, 0000, "PreCallValidateCmdBeginDebugUtilsLabelEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    StopCounting(start, 0000, "PreCallRecordCmdBeginDebugUtilsLabelEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    StopCounting(start, 0000, "PostCallRecordCmdBeginDebugUtilsLabelEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer);
    StopCounting(start, 0000, "PreCallValidateCmdEndDebugUtilsLabelEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    StopCounting(start, 0000, "PreCallRecordCmdEndDebugUtilsLabelEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    StopCounting(start, 0000, "PostCallRecordCmdEndDebugUtilsLabelEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    StopCounting(start, 0000, "PreCallValidateCmdInsertDebugUtilsLabelEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    StopCounting(start, 0000, "PreCallRecordCmdInsertDebugUtilsLabelEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    StopCounting(start, 0000, "PostCallRecordCmdInsertDebugUtilsLabelEXT");
}

bool CoreChecksInstrumented::PreCallValidateCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    StopCounting(start, 0000, "PreCallValidateCreateDebugUtilsMessengerEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    StopCounting(start, 0000, "PreCallRecordCreateDebugUtilsMessengerEXT");
}

void CoreChecksInstrumented::PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger, result);
    StopCounting(start, 0000, "PostCallRecordCreateDebugUtilsMessengerEXT");
}

bool CoreChecksInstrumented::PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyDebugUtilsMessengerEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyDebugUtilsMessengerEXT");
}

void CoreChecksInstrumented::PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyDebugUtilsMessengerEXT");
}

bool CoreChecksInstrumented::PreCallValidateSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    StopCounting(start, 0000, "PreCallValidateSubmitDebugUtilsMessageEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    StopCounting(start, 0000, "PreCallRecordSubmitDebugUtilsMessageEXT");
}

void CoreChecksInstrumented::PostCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    StopCounting(start, 0000, "PostCallRecordSubmitDebugUtilsMessageEXT");
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecksInstrumented::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetAndroidHardwareBufferPropertiesANDROID");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetAndroidHardwareBufferPropertiesANDROID");
}

void CoreChecksInstrumented::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetAndroidHardwareBufferPropertiesANDROID");
}

#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecksInstrumented::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    StopCounting(start, 0000, "PreCallValidateGetMemoryAndroidHardwareBufferANDROID");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    StopCounting(start, 0000, "PreCallRecordGetMemoryAndroidHardwareBufferANDROID");
}

void CoreChecksInstrumented::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer, result);
    StopCounting(start, 0000, "PostCallRecordGetMemoryAndroidHardwareBufferANDROID");
}

#endif // VK_USE_PLATFORM_ANDROID_KHR
bool CoreChecksInstrumented::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    StopCounting(start, 0000, "PreCallValidateCmdSetSampleLocationsEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    StopCounting(start, 0000, "PreCallRecordCmdSetSampleLocationsEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    StopCounting(start, 0000, "PostCallRecordCmdSetSampleLocationsEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetImageDrmFormatModifierPropertiesEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetImageDrmFormatModifierPropertiesEXT");
}

void CoreChecksInstrumented::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetImageDrmFormatModifierPropertiesEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    StopCounting(start, 0000, "PreCallValidateCmdBindShadingRateImageNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    StopCounting(start, 0000, "PreCallRecordCmdBindShadingRateImageNV");
}

void CoreChecksInstrumented::PostCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    StopCounting(start, 0000, "PostCallRecordCmdBindShadingRateImageNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    StopCounting(start, 0000, "PreCallValidateCmdSetViewportShadingRatePaletteNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    StopCounting(start, 0000, "PreCallRecordCmdSetViewportShadingRatePaletteNV");
}

void CoreChecksInstrumented::PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    StopCounting(start, 0000, "PostCallRecordCmdSetViewportShadingRatePaletteNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    StopCounting(start, 0000, "PreCallValidateCmdSetCoarseSampleOrderNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    StopCounting(start, 0000, "PreCallRecordCmdSetCoarseSampleOrderNV");
}

void CoreChecksInstrumented::PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    StopCounting(start, 0000, "PostCallRecordCmdSetCoarseSampleOrderNV");
}

bool CoreChecksInstrumented::PreCallValidateCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    StopCounting(start, 0000, "PreCallValidateCreateAccelerationStructureNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    StopCounting(start, 0000, "PreCallRecordCreateAccelerationStructureNV");
}

void CoreChecksInstrumented::PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
    StopCounting(start, 0000, "PostCallRecordCreateAccelerationStructureNV");
}

bool CoreChecksInstrumented::PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyAccelerationStructureKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyAccelerationStructureKHR");
}

void CoreChecksInstrumented::PostCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyAccelerationStructureKHR");
}

bool CoreChecksInstrumented::PreCallValidateDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyAccelerationStructureNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyAccelerationStructureNV");
}

void CoreChecksInstrumented::PostCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyAccelerationStructureNV");
}

bool CoreChecksInstrumented::PreCallValidateGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetAccelerationStructureMemoryRequirementsNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetAccelerationStructureMemoryRequirementsNV");
}

void CoreChecksInstrumented::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetAccelerationStructureMemoryRequirementsNV");
}

bool CoreChecksInstrumented::PreCallValidateBindAccelerationStructureMemoryKHR(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallValidateBindAccelerationStructureMemoryKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBindAccelerationStructureMemoryKHR(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallRecordBindAccelerationStructureMemoryKHR");
}

void CoreChecksInstrumented::PostCallRecordBindAccelerationStructureMemoryKHR(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos, result);
    StopCounting(start, 0000, "PostCallRecordBindAccelerationStructureMemoryKHR");
}

bool CoreChecksInstrumented::PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallValidateBindAccelerationStructureMemoryNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    StopCounting(start, 0000, "PreCallRecordBindAccelerationStructureMemoryNV");
}

void CoreChecksInstrumented::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, result);
    StopCounting(start, 0000, "PostCallRecordBindAccelerationStructureMemoryNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureKHR dst, VkAccelerationStructureKHR src, VkBuffer scratch, VkDeviceSize scratchOffset) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    StopCounting(start, 0000, "PreCallValidateCmdBuildAccelerationStructureNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureKHR dst, VkAccelerationStructureKHR src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    StopCounting(start, 0000, "PreCallRecordCmdBuildAccelerationStructureNV");
}

void CoreChecksInstrumented::PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureKHR dst, VkAccelerationStructureKHR src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    StopCounting(start, 0000, "PostCallRecordCmdBuildAccelerationStructureNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureKHR dst, VkAccelerationStructureKHR src, VkCopyAccelerationStructureModeKHR mode) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    StopCounting(start, 0000, "PreCallValidateCmdCopyAccelerationStructureNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureKHR dst, VkAccelerationStructureKHR src, VkCopyAccelerationStructureModeKHR mode) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    StopCounting(start, 0000, "PreCallRecordCmdCopyAccelerationStructureNV");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureKHR dst, VkAccelerationStructureKHR src, VkCopyAccelerationStructureModeKHR mode) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    StopCounting(start, 0000, "PostCallRecordCmdCopyAccelerationStructureNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    StopCounting(start, 0000, "PreCallValidateCmdTraceRaysNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    StopCounting(start, 0000, "PreCallRecordCmdTraceRaysNV");
}

void CoreChecksInstrumented::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    StopCounting(start, 0000, "PostCallRecordCmdTraceRaysNV");
}

bool CoreChecksInstrumented::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    StopCounting(start, 0000, "PreCallValidateCreateRayTracingPipelinesNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    StopCounting(start, 0000, "PreCallRecordCreateRayTracingPipelinesNV");
}

void CoreChecksInstrumented::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, extra_data);
    StopCounting(start, 0000, "PostCallRecordCreateRayTracingPipelinesNV");
}

bool CoreChecksInstrumented::PreCallValidateGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    StopCounting(start, 0000, "PreCallValidateGetRayTracingShaderGroupHandlesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    StopCounting(start, 0000, "PreCallRecordGetRayTracingShaderGroupHandlesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    StopCounting(start, 0000, "PostCallRecordGetRayTracingShaderGroupHandlesKHR");
}

bool CoreChecksInstrumented::PreCallValidateGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    StopCounting(start, 0000, "PreCallValidateGetRayTracingShaderGroupHandlesNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    StopCounting(start, 0000, "PreCallRecordGetRayTracingShaderGroupHandlesNV");
}

void CoreChecksInstrumented::PostCallRecordGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    StopCounting(start, 0000, "PostCallRecordGetRayTracingShaderGroupHandlesNV");
}

bool CoreChecksInstrumented::PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureKHR accelerationStructure, size_t dataSize, void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    StopCounting(start, 0000, "PreCallValidateGetAccelerationStructureHandleNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureKHR accelerationStructure, size_t dataSize, void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    StopCounting(start, 0000, "PreCallRecordGetAccelerationStructureHandleNV");
}

void CoreChecksInstrumented::PostCallRecordGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureKHR accelerationStructure, size_t dataSize, void* pData, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData, result);
    StopCounting(start, 0000, "PostCallRecordGetAccelerationStructureHandleNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    StopCounting(start, 0000, "PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    StopCounting(start, 0000, "PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    StopCounting(start, 0000, "PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR");
}

bool CoreChecksInstrumented::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    StopCounting(start, 0000, "PreCallValidateCmdWriteAccelerationStructuresPropertiesNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    StopCounting(start, 0000, "PreCallRecordCmdWriteAccelerationStructuresPropertiesNV");
}

void CoreChecksInstrumented::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    StopCounting(start, 0000, "PostCallRecordCmdWriteAccelerationStructuresPropertiesNV");
}

bool CoreChecksInstrumented::PreCallValidateCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCompileDeferredNV(device, pipeline, shader);
    StopCounting(start, 0000, "PreCallValidateCompileDeferredNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCompileDeferredNV(device, pipeline, shader);
    StopCounting(start, 0000, "PreCallRecordCompileDeferredNV");
}

void CoreChecksInstrumented::PostCallRecordCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCompileDeferredNV(device, pipeline, shader, result);
    StopCounting(start, 0000, "PostCallRecordCompileDeferredNV");
}

bool CoreChecksInstrumented::PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    StopCounting(start, 0000, "PreCallValidateGetMemoryHostPointerPropertiesEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    StopCounting(start, 0000, "PreCallRecordGetMemoryHostPointerPropertiesEXT");
}

void CoreChecksInstrumented::PostCallRecordGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetMemoryHostPointerPropertiesEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    StopCounting(start, 0000, "PreCallValidateCmdWriteBufferMarkerAMD");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    StopCounting(start, 0000, "PreCallRecordCmdWriteBufferMarkerAMD");
}

void CoreChecksInstrumented::PostCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    StopCounting(start, 0000, "PostCallRecordCmdWriteBufferMarkerAMD");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    StopCounting(start, 0000, "PreCallValidateGetCalibratedTimestampsEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    StopCounting(start, 0000, "PreCallRecordGetCalibratedTimestampsEXT");
}

void CoreChecksInstrumented::PostCallRecordGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation, result);
    StopCounting(start, 0000, "PostCallRecordGetCalibratedTimestampsEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    StopCounting(start, 0000, "PreCallValidateCmdDrawMeshTasksNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    StopCounting(start, 0000, "PreCallRecordCmdDrawMeshTasksNV");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    StopCounting(start, 0000, "PostCallRecordCmdDrawMeshTasksNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawMeshTasksIndirectNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawMeshTasksIndirectNV");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawMeshTasksIndirectNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallValidateCmdDrawMeshTasksIndirectCountNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PreCallRecordCmdDrawMeshTasksIndirectCountNV");
}

void CoreChecksInstrumented::PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    StopCounting(start, 0000, "PostCallRecordCmdDrawMeshTasksIndirectCountNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    StopCounting(start, 0000, "PreCallValidateCmdSetExclusiveScissorNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    StopCounting(start, 0000, "PreCallRecordCmdSetExclusiveScissorNV");
}

void CoreChecksInstrumented::PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    StopCounting(start, 0000, "PostCallRecordCmdSetExclusiveScissorNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    StopCounting(start, 0000, "PreCallValidateCmdSetCheckpointNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    StopCounting(start, 0000, "PreCallRecordCmdSetCheckpointNV");
}

void CoreChecksInstrumented::PostCallRecordCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    StopCounting(start, 0000, "PostCallRecordCmdSetCheckpointNV");
}

bool CoreChecksInstrumented::PreCallValidateGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    StopCounting(start, 0000, "PreCallValidateGetQueueCheckpointDataNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    StopCounting(start, 0000, "PreCallRecordGetQueueCheckpointDataNV");
}

void CoreChecksInstrumented::PostCallRecordGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    StopCounting(start, 0000, "PostCallRecordGetQueueCheckpointDataNV");
}

bool CoreChecksInstrumented::PreCallValidateInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateInitializePerformanceApiINTEL(device, pInitializeInfo);
    StopCounting(start, 0000, "PreCallValidateInitializePerformanceApiINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo);
    StopCounting(start, 0000, "PreCallRecordInitializePerformanceApiINTEL");
}

void CoreChecksInstrumented::PostCallRecordInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo, result);
    StopCounting(start, 0000, "PostCallRecordInitializePerformanceApiINTEL");
}

bool CoreChecksInstrumented::PreCallValidateUninitializePerformanceApiINTEL(VkDevice device) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateUninitializePerformanceApiINTEL(device);
    StopCounting(start, 0000, "PreCallValidateUninitializePerformanceApiINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordUninitializePerformanceApiINTEL(VkDevice device) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordUninitializePerformanceApiINTEL(device);
    StopCounting(start, 0000, "PreCallRecordUninitializePerformanceApiINTEL");
}

void CoreChecksInstrumented::PostCallRecordUninitializePerformanceApiINTEL(VkDevice device) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordUninitializePerformanceApiINTEL(device);
    StopCounting(start, 0000, "PostCallRecordUninitializePerformanceApiINTEL");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PreCallValidateCmdSetPerformanceMarkerINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PreCallRecordCmdSetPerformanceMarkerINTEL");
}

void CoreChecksInstrumented::PostCallRecordCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo, result);
    StopCounting(start, 0000, "PostCallRecordCmdSetPerformanceMarkerINTEL");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PreCallValidateCmdSetPerformanceStreamMarkerINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    StopCounting(start, 0000, "PreCallRecordCmdSetPerformanceStreamMarkerINTEL");
}

void CoreChecksInstrumented::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo, result);
    StopCounting(start, 0000, "PostCallRecordCmdSetPerformanceStreamMarkerINTEL");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    StopCounting(start, 0000, "PreCallValidateCmdSetPerformanceOverrideINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    StopCounting(start, 0000, "PreCallRecordCmdSetPerformanceOverrideINTEL");
}

void CoreChecksInstrumented::PostCallRecordCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo, result);
    StopCounting(start, 0000, "PostCallRecordCmdSetPerformanceOverrideINTEL");
}

bool CoreChecksInstrumented::PreCallValidateAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo, VkPerformanceConfigurationINTEL* pConfiguration) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    StopCounting(start, 0000, "PreCallValidateAcquirePerformanceConfigurationINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo, VkPerformanceConfigurationINTEL* pConfiguration) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    StopCounting(start, 0000, "PreCallRecordAcquirePerformanceConfigurationINTEL");
}

void CoreChecksInstrumented::PostCallRecordAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo, VkPerformanceConfigurationINTEL* pConfiguration, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration, result);
    StopCounting(start, 0000, "PostCallRecordAcquirePerformanceConfigurationINTEL");
}

bool CoreChecksInstrumented::PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateReleasePerformanceConfigurationINTEL(device, configuration);
    StopCounting(start, 0000, "PreCallValidateReleasePerformanceConfigurationINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordReleasePerformanceConfigurationINTEL(device, configuration);
    StopCounting(start, 0000, "PreCallRecordReleasePerformanceConfigurationINTEL");
}

void CoreChecksInstrumented::PostCallRecordReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordReleasePerformanceConfigurationINTEL(device, configuration, result);
    StopCounting(start, 0000, "PostCallRecordReleasePerformanceConfigurationINTEL");
}

bool CoreChecksInstrumented::PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateQueueSetPerformanceConfigurationINTEL(queue, configuration);
    StopCounting(start, 0000, "PreCallValidateQueueSetPerformanceConfigurationINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration);
    StopCounting(start, 0000, "PreCallRecordQueueSetPerformanceConfigurationINTEL");
}

void CoreChecksInstrumented::PostCallRecordQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration, result);
    StopCounting(start, 0000, "PostCallRecordQueueSetPerformanceConfigurationINTEL");
}

bool CoreChecksInstrumented::PreCallValidateGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL* pValue) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPerformanceParameterINTEL(device, parameter, pValue);
    StopCounting(start, 0000, "PreCallValidateGetPerformanceParameterINTEL");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL* pValue) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPerformanceParameterINTEL(device, parameter, pValue);
    StopCounting(start, 0000, "PreCallRecordGetPerformanceParameterINTEL");
}

void CoreChecksInstrumented::PostCallRecordGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL* pValue, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPerformanceParameterINTEL(device, parameter, pValue, result);
    StopCounting(start, 0000, "PostCallRecordGetPerformanceParameterINTEL");
}

bool CoreChecksInstrumented::PreCallValidateSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    StopCounting(start, 0000, "PreCallValidateSetLocalDimmingAMD");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    StopCounting(start, 0000, "PreCallRecordSetLocalDimmingAMD");
}

void CoreChecksInstrumented::PostCallRecordSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    StopCounting(start, 0000, "PostCallRecordSetLocalDimmingAMD");
}

#ifdef VK_USE_PLATFORM_FUCHSIA
bool CoreChecksInstrumented::PreCallValidateCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateImagePipeSurfaceFUCHSIA");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateImagePipeSurfaceFUCHSIA");
}

void CoreChecksInstrumented::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateImagePipeSurfaceFUCHSIA");
}

#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
bool CoreChecksInstrumented::PreCallValidateCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateMetalSurfaceEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateMetalSurfaceEXT");
}

void CoreChecksInstrumented::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateMetalSurfaceEXT");
}

#endif // VK_USE_PLATFORM_METAL_EXT
bool CoreChecksInstrumented::PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetBufferDeviceAddressEXT(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetBufferDeviceAddressEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetBufferDeviceAddressEXT(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetBufferDeviceAddressEXT");
}

void CoreChecksInstrumented::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo, VkDeviceAddress result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetBufferDeviceAddressEXT(device, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordGetBufferDeviceAddressEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolPropertiesEXT* pToolProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceToolPropertiesEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolPropertiesEXT* pToolProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceToolPropertiesEXT");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolPropertiesEXT* pToolProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceToolPropertiesEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV");
}

bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes, result);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateAcquireFullScreenExclusiveModeEXT(device, swapchain);
    StopCounting(start, 0000, "PreCallValidateAcquireFullScreenExclusiveModeEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain);
    StopCounting(start, 0000, "PreCallRecordAcquireFullScreenExclusiveModeEXT");
}

void CoreChecksInstrumented::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain, result);
    StopCounting(start, 0000, "PostCallRecordAcquireFullScreenExclusiveModeEXT");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateReleaseFullScreenExclusiveModeEXT(device, swapchain);
    StopCounting(start, 0000, "PreCallValidateReleaseFullScreenExclusiveModeEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain);
    StopCounting(start, 0000, "PreCallRecordReleaseFullScreenExclusiveModeEXT");
}

void CoreChecksInstrumented::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain, result);
    StopCounting(start, 0000, "PostCallRecordReleaseFullScreenExclusiveModeEXT");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR* pModes) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    StopCounting(start, 0000, "PreCallValidateGetDeviceGroupSurfacePresentModes2EXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR* pModes) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    StopCounting(start, 0000, "PreCallRecordGetDeviceGroupSurfacePresentModes2EXT");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR* pModes, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes, result);
    StopCounting(start, 0000, "PostCallRecordGetDeviceGroupSurfacePresentModes2EXT");
}

#endif // VK_USE_PLATFORM_WIN32_KHR
bool CoreChecksInstrumented::PreCallValidateCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateHeadlessSurfaceEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateHeadlessSurfaceEXT");
}

void CoreChecksInstrumented::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateHeadlessSurfaceEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    StopCounting(start, 0000, "PreCallValidateCmdSetLineStippleEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    StopCounting(start, 0000, "PreCallRecordCmdSetLineStippleEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    StopCounting(start, 0000, "PostCallRecordCmdSetLineStippleEXT");
}

bool CoreChecksInstrumented::PreCallValidateResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PreCallValidateResetQueryPoolEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PreCallRecordResetQueryPoolEXT");
}

void CoreChecksInstrumented::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    StopCounting(start, 0000, "PostCallRecordResetQueryPoolEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetCullModeEXT(commandBuffer, cullMode);
    StopCounting(start, 0000, "PreCallValidateCmdSetCullModeEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
    StopCounting(start, 0000, "PreCallRecordCmdSetCullModeEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
    StopCounting(start, 0000, "PostCallRecordCmdSetCullModeEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetFrontFaceEXT(commandBuffer, frontFace);
    StopCounting(start, 0000, "PreCallValidateCmdSetFrontFaceEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
    StopCounting(start, 0000, "PreCallRecordCmdSetFrontFaceEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
    StopCounting(start, 0000, "PostCallRecordCmdSetFrontFaceEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    StopCounting(start, 0000, "PreCallValidateCmdSetPrimitiveTopologyEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    StopCounting(start, 0000, "PreCallRecordCmdSetPrimitiveTopologyEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    StopCounting(start, 0000, "PostCallRecordCmdSetPrimitiveTopologyEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    StopCounting(start, 0000, "PreCallValidateCmdSetViewportWithCountEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    StopCounting(start, 0000, "PreCallRecordCmdSetViewportWithCountEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    StopCounting(start, 0000, "PostCallRecordCmdSetViewportWithCountEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    StopCounting(start, 0000, "PreCallValidateCmdSetScissorWithCountEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    StopCounting(start, 0000, "PreCallRecordCmdSetScissorWithCountEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    StopCounting(start, 0000, "PostCallRecordCmdSetScissorWithCountEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    StopCounting(start, 0000, "PreCallValidateCmdBindVertexBuffers2EXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    StopCounting(start, 0000, "PreCallRecordCmdBindVertexBuffers2EXT");
}

void CoreChecksInstrumented::PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    StopCounting(start, 0000, "PostCallRecordCmdBindVertexBuffers2EXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    StopCounting(start, 0000, "PreCallValidateCmdSetDepthTestEnableEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    StopCounting(start, 0000, "PreCallRecordCmdSetDepthTestEnableEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    StopCounting(start, 0000, "PostCallRecordCmdSetDepthTestEnableEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    StopCounting(start, 0000, "PreCallValidateCmdSetDepthWriteEnableEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    StopCounting(start, 0000, "PreCallRecordCmdSetDepthWriteEnableEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    StopCounting(start, 0000, "PostCallRecordCmdSetDepthWriteEnableEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    StopCounting(start, 0000, "PreCallValidateCmdSetDepthCompareOpEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    StopCounting(start, 0000, "PreCallRecordCmdSetDepthCompareOpEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    StopCounting(start, 0000, "PostCallRecordCmdSetDepthCompareOpEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    StopCounting(start, 0000, "PreCallValidateCmdSetDepthBoundsTestEnableEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    StopCounting(start, 0000, "PreCallRecordCmdSetDepthBoundsTestEnableEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    StopCounting(start, 0000, "PostCallRecordCmdSetDepthBoundsTestEnableEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    StopCounting(start, 0000, "PreCallValidateCmdSetStencilTestEnableEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    StopCounting(start, 0000, "PreCallRecordCmdSetStencilTestEnableEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    StopCounting(start, 0000, "PostCallRecordCmdSetStencilTestEnableEXT");
}

bool CoreChecksInstrumented::PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    StopCounting(start, 0000, "PreCallValidateCmdSetStencilOpEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    StopCounting(start, 0000, "PreCallRecordCmdSetStencilOpEXT");
}

void CoreChecksInstrumented::PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    StopCounting(start, 0000, "PostCallRecordCmdSetStencilOpEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetGeneratedCommandsMemoryRequirementsNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetGeneratedCommandsMemoryRequirementsNV");
}

void CoreChecksInstrumented::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetGeneratedCommandsMemoryRequirementsNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    StopCounting(start, 0000, "PreCallValidateCmdPreprocessGeneratedCommandsNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    StopCounting(start, 0000, "PreCallRecordCmdPreprocessGeneratedCommandsNV");
}

void CoreChecksInstrumented::PostCallRecordCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    StopCounting(start, 0000, "PostCallRecordCmdPreprocessGeneratedCommandsNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    StopCounting(start, 0000, "PreCallValidateCmdExecuteGeneratedCommandsNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    StopCounting(start, 0000, "PreCallRecordCmdExecuteGeneratedCommandsNV");
}

void CoreChecksInstrumented::PostCallRecordCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    StopCounting(start, 0000, "PostCallRecordCmdExecuteGeneratedCommandsNV");
}

bool CoreChecksInstrumented::PreCallValidateCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    StopCounting(start, 0000, "PreCallValidateCmdBindPipelineShaderGroupNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    StopCounting(start, 0000, "PreCallRecordCmdBindPipelineShaderGroupNV");
}

void CoreChecksInstrumented::PostCallRecordCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    StopCounting(start, 0000, "PostCallRecordCmdBindPipelineShaderGroupNV");
}

bool CoreChecksInstrumented::PreCallValidateCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    StopCounting(start, 0000, "PreCallValidateCreateIndirectCommandsLayoutNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    StopCounting(start, 0000, "PreCallRecordCreateIndirectCommandsLayoutNV");
}

void CoreChecksInstrumented::PostCallRecordCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout, result);
    StopCounting(start, 0000, "PostCallRecordCreateIndirectCommandsLayoutNV");
}

bool CoreChecksInstrumented::PreCallValidateDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyIndirectCommandsLayoutNV");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyIndirectCommandsLayoutNV");
}

void CoreChecksInstrumented::PostCallRecordDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyIndirectCommandsLayoutNV");
}

bool CoreChecksInstrumented::PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlotEXT* pPrivateDataSlot) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    StopCounting(start, 0000, "PreCallValidateCreatePrivateDataSlotEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlotEXT* pPrivateDataSlot) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    StopCounting(start, 0000, "PreCallRecordCreatePrivateDataSlotEXT");
}

void CoreChecksInstrumented::PostCallRecordCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlotEXT* pPrivateDataSlot, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot, result);
    StopCounting(start, 0000, "PostCallRecordCreatePrivateDataSlotEXT");
}

bool CoreChecksInstrumented::PreCallValidateDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlotEXT privateDataSlot, const VkAllocationCallbacks* pAllocator) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    StopCounting(start, 0000, "PreCallValidateDestroyPrivateDataSlotEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlotEXT privateDataSlot, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    StopCounting(start, 0000, "PreCallRecordDestroyPrivateDataSlotEXT");
}

void CoreChecksInstrumented::PostCallRecordDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlotEXT privateDataSlot, const VkAllocationCallbacks* pAllocator) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    StopCounting(start, 0000, "PostCallRecordDestroyPrivateDataSlotEXT");
}

bool CoreChecksInstrumented::PreCallValidateSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t data) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    StopCounting(start, 0000, "PreCallValidateSetPrivateDataEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    StopCounting(start, 0000, "PreCallRecordSetPrivateDataEXT");
}

void CoreChecksInstrumented::PostCallRecordSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t data, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data, result);
    StopCounting(start, 0000, "PostCallRecordSetPrivateDataEXT");
}

bool CoreChecksInstrumented::PreCallValidateGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    StopCounting(start, 0000, "PreCallValidateGetPrivateDataEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    StopCounting(start, 0000, "PreCallRecordGetPrivateDataEXT");
}

void CoreChecksInstrumented::PostCallRecordGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlotEXT privateDataSlot, uint64_t* pData) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    StopCounting(start, 0000, "PostCallRecordGetPrivateDataEXT");
}

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool CoreChecksInstrumented::PreCallValidateCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallValidateCreateDirectFBSurfaceEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    StopCounting(start, 0000, "PreCallRecordCreateDirectFBSurfaceEXT");
}

void CoreChecksInstrumented::PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface, result);
    StopCounting(start, 0000, "PostCallRecordCreateDirectFBSurfaceEXT");
}

#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool CoreChecksInstrumented::PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, IDirectFB* dfb) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    StopCounting(start, 0000, "PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, IDirectFB* dfb) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    StopCounting(start, 0000, "PreCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT");
}

void CoreChecksInstrumented::PostCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, IDirectFB* dfb) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    StopCounting(start, 0000, "PostCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT");
}

#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCreateAccelerationStructureKHR(VkDevice                                           device, const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, const VkAllocationCallbacks*       pAllocator, VkAccelerationStructureKHR*                        pAccelerationStructure) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    StopCounting(start, 0000, "PreCallValidateCreateAccelerationStructureKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateAccelerationStructureKHR(VkDevice                                           device, const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, const VkAllocationCallbacks*       pAllocator, VkAccelerationStructureKHR*                        pAccelerationStructure) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    StopCounting(start, 0000, "PreCallRecordCreateAccelerationStructureKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateAccelerationStructureKHR(VkDevice                                           device, const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, const VkAllocationCallbacks*       pAllocator, VkAccelerationStructureKHR*                        pAccelerationStructure, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure, result);
    StopCounting(start, 0000, "PostCallRecordCreateAccelerationStructureKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateGetAccelerationStructureMemoryRequirementsKHR(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallValidateGetAccelerationStructureMemoryRequirementsKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetAccelerationStructureMemoryRequirementsKHR(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PreCallRecordGetAccelerationStructureMemoryRequirementsKHR");
}

void CoreChecksInstrumented::PostCallRecordGetAccelerationStructureMemoryRequirementsKHR(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    StopCounting(start, 0000, "PostCallRecordGetAccelerationStructureMemoryRequirementsKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCmdBuildAccelerationStructureKHR(VkCommandBuffer                                    commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos);
    StopCounting(start, 0000, "PreCallValidateCmdBuildAccelerationStructureKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBuildAccelerationStructureKHR(VkCommandBuffer                                    commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos);
    StopCounting(start, 0000, "PreCallRecordCmdBuildAccelerationStructureKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdBuildAccelerationStructureKHR(VkCommandBuffer                                    commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos);
    StopCounting(start, 0000, "PostCallRecordCmdBuildAccelerationStructureKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCmdBuildAccelerationStructureIndirectKHR(VkCommandBuffer                  commandBuffer, const VkAccelerationStructureBuildGeometryInfoKHR* pInfo, VkBuffer                                           indirectBuffer, VkDeviceSize                                       indirectOffset, uint32_t                                           indirectStride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride);
    StopCounting(start, 0000, "PreCallValidateCmdBuildAccelerationStructureIndirectKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdBuildAccelerationStructureIndirectKHR(VkCommandBuffer                  commandBuffer, const VkAccelerationStructureBuildGeometryInfoKHR* pInfo, VkBuffer                                           indirectBuffer, VkDeviceSize                                       indirectOffset, uint32_t                                           indirectStride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride);
    StopCounting(start, 0000, "PreCallRecordCmdBuildAccelerationStructureIndirectKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdBuildAccelerationStructureIndirectKHR(VkCommandBuffer                  commandBuffer, const VkAccelerationStructureBuildGeometryInfoKHR* pInfo, VkBuffer                                           indirectBuffer, VkDeviceSize                                       indirectOffset, uint32_t                                           indirectStride) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdBuildAccelerationStructureIndirectKHR(commandBuffer, pInfo, indirectBuffer, indirectOffset, indirectStride);
    StopCounting(start, 0000, "PostCallRecordCmdBuildAccelerationStructureIndirectKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateBuildAccelerationStructureKHR(VkDevice                                           device, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos);
    StopCounting(start, 0000, "PreCallValidateBuildAccelerationStructureKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordBuildAccelerationStructureKHR(VkDevice                                           device, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos);
    StopCounting(start, 0000, "PreCallRecordBuildAccelerationStructureKHR");
}

void CoreChecksInstrumented::PostCallRecordBuildAccelerationStructureKHR(VkDevice                                           device, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos, result);
    StopCounting(start, 0000, "PostCallRecordBuildAccelerationStructureKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCopyAccelerationStructureKHR(VkDevice device, const VkCopyAccelerationStructureInfoKHR* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCopyAccelerationStructureKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateCopyAccelerationStructureKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCopyAccelerationStructureKHR(VkDevice device, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCopyAccelerationStructureKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordCopyAccelerationStructureKHR");
}

void CoreChecksInstrumented::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, const VkCopyAccelerationStructureInfoKHR* pInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCopyAccelerationStructureKHR(device, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordCopyAccelerationStructureKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCopyAccelerationStructureToMemoryKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateCopyAccelerationStructureToMemoryKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCopyAccelerationStructureToMemoryKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordCopyAccelerationStructureToMemoryKHR");
}

void CoreChecksInstrumented::PostCallRecordCopyAccelerationStructureToMemoryKHR(VkDevice device, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCopyAccelerationStructureToMemoryKHR(device, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordCopyAccelerationStructureToMemoryKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCopyMemoryToAccelerationStructureKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateCopyMemoryToAccelerationStructureKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCopyMemoryToAccelerationStructureKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordCopyMemoryToAccelerationStructureKHR");
}

void CoreChecksInstrumented::PostCallRecordCopyMemoryToAccelerationStructureKHR(VkDevice device, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCopyMemoryToAccelerationStructureKHR(device, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordCopyMemoryToAccelerationStructureKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType  queryType, size_t       dataSize, void* pData, size_t stride) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    StopCounting(start, 0000, "PreCallValidateWriteAccelerationStructuresPropertiesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType  queryType, size_t       dataSize, void* pData, size_t stride) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    StopCounting(start, 0000, "PreCallRecordWriteAccelerationStructuresPropertiesKHR");
}

void CoreChecksInstrumented::PostCallRecordWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType  queryType, size_t       dataSize, void* pData, size_t stride, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride, result);
    StopCounting(start, 0000, "PostCallRecordWriteAccelerationStructuresPropertiesKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PreCallValidateCmdCopyAccelerationStructureKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PreCallRecordCmdCopyAccelerationStructureKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PostCallRecordCmdCopyAccelerationStructureKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PreCallValidateCmdCopyAccelerationStructureToMemoryKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PreCallRecordCmdCopyAccelerationStructureToMemoryKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PostCallRecordCmdCopyAccelerationStructureToMemoryKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PreCallValidateCmdCopyMemoryToAccelerationStructureKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PreCallRecordCmdCopyMemoryToAccelerationStructureKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    StopCounting(start, 0000, "PostCallRecordCmdCopyMemoryToAccelerationStructureKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable, const VkStridedBufferRegionKHR* pMissShaderBindingTable, const VkStridedBufferRegionKHR* pHitShaderBindingTable, const VkStridedBufferRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    StopCounting(start, 0000, "PreCallValidateCmdTraceRaysKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable, const VkStridedBufferRegionKHR* pMissShaderBindingTable, const VkStridedBufferRegionKHR* pHitShaderBindingTable, const VkStridedBufferRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    StopCounting(start, 0000, "PreCallRecordCmdTraceRaysKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable, const VkStridedBufferRegionKHR* pMissShaderBindingTable, const VkStridedBufferRegionKHR* pHitShaderBindingTable, const VkStridedBufferRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    StopCounting(start, 0000, "PostCallRecordCmdTraceRaysKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    StopCounting(start, 0000, "PreCallValidateCreateRayTracingPipelinesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, extra_data);
    StopCounting(start, 0000, "PreCallRecordCreateRayTracingPipelinesKHR");
}

void CoreChecksInstrumented::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result, void* extra_data) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, result, extra_data);
    StopCounting(start, 0000, "PostCallRecordCreateRayTracingPipelinesKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallValidateGetAccelerationStructureDeviceAddressKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    StopCounting(start, 0000, "PreCallRecordGetAccelerationStructureDeviceAddressKHR");
}

void CoreChecksInstrumented::PostCallRecordGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo, VkDeviceAddress result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo, result);
    StopCounting(start, 0000, "PostCallRecordGetAccelerationStructureDeviceAddressKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    StopCounting(start, 0000, "PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    StopCounting(start, 0000, "PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR");
}

void CoreChecksInstrumented::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData, result);
    StopCounting(start, 0000, "PostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable, const VkStridedBufferRegionKHR* pMissShaderBindingTable, const VkStridedBufferRegionKHR* pHitShaderBindingTable, const VkStridedBufferRegionKHR* pCallableShaderBindingTable, VkBuffer buffer, VkDeviceSize offset) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset);
    StopCounting(start, 0000, "PreCallValidateCmdTraceRaysIndirectKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable, const VkStridedBufferRegionKHR* pMissShaderBindingTable, const VkStridedBufferRegionKHR* pHitShaderBindingTable, const VkStridedBufferRegionKHR* pCallableShaderBindingTable, VkBuffer buffer, VkDeviceSize offset) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset);
    StopCounting(start, 0000, "PreCallRecordCmdTraceRaysIndirectKHR");
}

void CoreChecksInstrumented::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedBufferRegionKHR* pRaygenShaderBindingTable, const VkStridedBufferRegionKHR* pMissShaderBindingTable, const VkStridedBufferRegionKHR* pHitShaderBindingTable, const VkStridedBufferRegionKHR* pCallableShaderBindingTable, VkBuffer buffer, VkDeviceSize offset) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, buffer, offset);
    StopCounting(start, 0000, "PostCallRecordCmdTraceRaysIndirectKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
bool CoreChecksInstrumented::PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionKHR* version) const {
    auto start = StartCounting();
    auto result = CoreChecks::PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(device, version);
    StopCounting(start, 0000, "PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR");
    return result;
}

void CoreChecksInstrumented::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionKHR* version) {
    auto start = StartCounting();
    CoreChecks::PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, version);
    StopCounting(start, 0000, "PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR");
}

void CoreChecksInstrumented::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionKHR* version, VkResult result) {
    auto start = StartCounting();
    CoreChecks::PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, version, result);
    StopCounting(start, 0000, "PostCallRecordGetDeviceAccelerationStructureCompatibilityKHR");
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#endif // INSTRUMENT_CORECHECKS
