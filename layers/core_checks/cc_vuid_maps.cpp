/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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
#include "cc_vuid_maps.h"
#include "error_message/error_location.h"
#include "utils/vk_layer_utils.h"

namespace vvl {

const std::string &GetCopyBufferImageDeviceVUID(const Location &loc, CopyError error) {
    static const std::map<CopyError, std::array<Entry, 4>> errors{
        {CopyError::TexelBlockSize_07975,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07975"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07975"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07975"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07975"},
         }}},
        {CopyError::MultiPlaneCompatible_07976,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07976"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07976"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07976"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07976"},
         }}},
        {CopyError::BufferOffset_07737,
         {{
             // was split up in 1.3.236 spec (internal MR 5371)
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-commandBuffer-07737"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-commandBuffer-07746"},
             {Key(Func::vkCmdCopyBufferToImage2), "VUID-vkCmdCopyBufferToImage2-commandBuffer-07737"},
             {Key(Func::vkCmdCopyImageToBuffer2), "VUID-vkCmdCopyImageToBuffer2-commandBuffer-07746"},
         }}},
        {CopyError::BufferOffset_07978,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07978"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07978"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07978"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07978"},
         }}},
        {CopyError::MemoryOverlap_00173,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-pRegions-00173"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-pRegions-00184"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-pRegions-00173"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-pRegions-00184"},
         }}},
        {CopyError::ImageExtentWidthZero_06659,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-imageExtent-06659"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-imageExtent-06659"},
             {Key(Struct::VkMemoryToImageCopyEXT), "VUID-VkMemoryToImageCopyEXT-imageExtent-06659"},
             {Key(Struct::VkImageToMemoryCopyEXT), "VUID-VkImageToMemoryCopyEXT-imageExtent-06659"},
         }}},
        {CopyError::ImageExtentHeightZero_06660,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-imageExtent-06660"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-imageExtent-06660"},
             {Key(Struct::VkMemoryToImageCopyEXT), "VUID-VkMemoryToImageCopyEXT-imageExtent-06660"},
             {Key(Struct::VkImageToMemoryCopyEXT), "VUID-VkImageToMemoryCopyEXT-imageExtent-06660"},
         }}},
        {CopyError::ImageExtentDepthZero_06661,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-imageExtent-06661"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-imageExtent-06661"},
             {Key(Struct::VkMemoryToImageCopyEXT), "VUID-VkMemoryToImageCopyEXT-imageExtent-06661"},
             {Key(Struct::VkImageToMemoryCopyEXT), "VUID-VkImageToMemoryCopyEXT-imageExtent-06661"},
         }}},
        {CopyError::ImageExtentRowLength_09101,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-bufferRowLength-09101"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-bufferRowLength-09101"},
             {Key(Struct::VkMemoryToImageCopyEXT), "VUID-VkMemoryToImageCopyEXT-memoryRowLength-09101"},
             {Key(Struct::VkImageToMemoryCopyEXT), "VUID-VkImageToMemoryCopyEXT-memoryRowLength-09101"},
         }}},
        {CopyError::ImageExtentImageHeight_09102,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-bufferImageHeight-09102"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-bufferImageHeight-09102"},
             {Key(Struct::VkMemoryToImageCopyEXT), "VUID-VkMemoryToImageCopyEXT-memoryImageHeight-09102"},
             {Key(Struct::VkImageToMemoryCopyEXT), "VUID-VkImageToMemoryCopyEXT-memoryImageHeight-09102"},
         }}},
        {CopyError::AspectMaskSingleBit_09103,
         {{
             {Key(Struct::VkBufferImageCopy), "VUID-VkBufferImageCopy-aspectMask-09103"},
             {Key(Struct::VkBufferImageCopy2), "VUID-VkBufferImageCopy2-aspectMask-09103"},
             {Key(Struct::VkMemoryToImageCopyEXT), "VUID-VkMemoryToImageCopyEXT-aspectMask-09103"},
             {Key(Struct::VkImageToMemoryCopyEXT), "VUID-VkImageToMemoryCopyEXT-aspectMask-09103"},
         }}},
    };

    // It is error prone to have every call set the struct
    // Since there are a known mapping, easier to do here when we are about to print an error message
    Struct s = loc.structure;
    Func f = loc.function;
    if (IsValueIn(loc.function, {Func::vkCmdCopyImageToBuffer, Func::vkCmdCopyBufferToImage})) {
        s = Struct::VkBufferImageCopy;
    } else if (IsValueIn(loc.function, {Func::vkCmdCopyBufferToImage2, Func::vkCmdCopyBufferToImage2KHR,
                                        Func::vkCmdCopyImageToBuffer2, Func::vkCmdCopyImageToBuffer2KHR})) {
        s = Struct::VkBufferImageCopy2;
    } else if (loc.function == Func::vkCopyImageToMemoryEXT) {
        s = Struct::VkImageToMemoryCopyEXT;
    } else if (loc.function == Func::vkCopyMemoryToImageEXT) {
        s = Struct::VkMemoryToImageCopyEXT;
    }
    const Location updated_loc(f, s, loc.field, loc.index);

    const auto &result = FindVUID(error, updated_loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-copy-buffer");
        return unhandled;
    }
    return result;
}

const std::string &GetCopyBufferImageVUID(const Location &loc, CopyError error) {
    static const std::map<CopyError, std::array<Entry, 6>> errors{
        {CopyError::ImageOffest_07971,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-07971"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-07971"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-pRegions-06223"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-imageOffset-00197"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07971"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07971"},
         }}},
        {CopyError::ImageOffest_07972,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-07972"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-07972"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-pRegions-06224"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-imageOffset-00198"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07972"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07972"},
         }}},
        {CopyError::Image1D_07979,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07979"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07979"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07979"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07979"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07979"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07979"},
         }}},
        {CopyError::Image1D_07980,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07980"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07980"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07980"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07980"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07980"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07980"},
         }}},
        {CopyError::Image3D_07983,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07983"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07983"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07983"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07983"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07983"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07983"},
         }}},
        {CopyError::TexelBlockExtentWidth_07274,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07274"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07274"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07274"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07274"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07274"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07274"},
         }}},
        {CopyError::TexelBlockExtentHeight_07275,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07275"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07275"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07275"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07275"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07275"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07275"},
         }}},
        {CopyError::TexelBlockExtentDepth_07276,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07276"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07276"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07276"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07276"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07276"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07276"},
         }}},
        {CopyError::TexelBlockExtentWidth_00207,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-00207"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-00207"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-00207"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-00207"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-00207"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-00207"},
         }}},
        {CopyError::TexelBlockExtentHeight_00208,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-00208"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-00208"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-00208"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-00208"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-00208"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-00208"},
         }}},
        {CopyError::TexelBlockExtentDepth_00209,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-00209"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-00209"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-00209"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-00209"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-00209"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-00209"},
         }}},
        {CopyError::MultiPlaneAspectMask_07981,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-dstImage-07981"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-srcImage-07981"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-dstImage-07981"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-srcImage-07981"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-dstImage-07981"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-srcImage-07981"},
         }}},
        {CopyError::ImageOffest_09104,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageOffset-09104"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageOffset-09104"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-imageOffset-09104"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-imageOffset-09104"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-imageOffset-09104"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-imageOffset-09104"},
         }}},
        {CopyError::AspectMask_09105,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-09105"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-09105"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-imageSubresource-09105"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-imageSubresource-09105"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-09105"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-09105"},
         }}},
        {CopyError::bufferRowLength_09106,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-bufferRowLength-09106"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-bufferRowLength-09106"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-bufferRowLength-09106"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-bufferRowLength-09106"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-memoryRowLength-09106"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-memoryRowLength-09106"},
         }}},
        {CopyError::bufferImageHeight_09107,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-bufferImageHeight-09107"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-bufferImageHeight-09107"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-bufferImageHeight-09107"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-bufferImageHeight-09107"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-memoryImageHeight-09107"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-memoryImageHeight-09107"},
         }}},
        {CopyError::bufferRowLength_09108,
         {{
             {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-bufferRowLength-09108"},
             {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-bufferRowLength-09108"},
             {Key(Struct::VkCopyBufferToImageInfo2), "VUID-VkCopyBufferToImageInfo2-bufferRowLength-09108"},
             {Key(Struct::VkCopyImageToBufferInfo2), "VUID-VkCopyImageToBufferInfo2-bufferRowLength-09108"},
             {Key(Struct::VkCopyMemoryToImageInfoEXT), "VUID-VkCopyMemoryToImageInfoEXT-memoryRowLength-09108"},
             {Key(Struct::VkCopyImageToMemoryInfoEXT), "VUID-VkCopyImageToMemoryInfoEXT-memoryRowLength-09108"},
         }}},
    };

    // It is error prone to have every call set the struct
    // Since there are a known mapping, easier to do here when we are about to print an error message
    Struct s = loc.structure;
    Func f = loc.function;
    if (IsValueIn(loc.function, {Func::vkCmdCopyImageToBuffer2, Func::vkCmdCopyImageToBuffer2KHR})) {
        s = Struct::VkCopyImageToBufferInfo2;
    } else if (IsValueIn(loc.function, {Func::vkCmdCopyBufferToImage2, Func::vkCmdCopyBufferToImage2KHR})) {
        s = Struct::VkCopyBufferToImageInfo2;
    } else if (loc.function == Func::vkCopyImageToMemoryEXT) {
        s = Struct::VkCopyImageToMemoryInfoEXT;
    } else if (loc.function == Func::vkCopyMemoryToImageEXT) {
        s = Struct::VkCopyMemoryToImageInfoEXT;
    }
    const Location updated_loc(f, s, loc.field, loc.index);

    const auto &result = FindVUID(error, updated_loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-copy-buffer-image");
        return unhandled;
    }
    return result;
}

const std::string &GetCopyImageVUID(const Location &loc, CopyError error) {
    static const std::map<CopyError, std::array<Entry, 3>> errors{
        {CopyError::SrcImage1D_00146,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-00146"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-00146"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-07979"},
         }}},
        {CopyError::DstImage1D_00152,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-00152"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-00152"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-07979"},
         }}},
        {CopyError::SrcImage1D_01785,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-01785"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-01785"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-07980"},
         }}},
        {CopyError::DstImage1D_01786,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-01786"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-01786"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-07980"},
         }}},
        {CopyError::SrcOffset_01728,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-01728"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-01728"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-00207"},
         }}},
        {CopyError::SrcOffset_01729,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-01729"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-01729"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-00208"},
         }}},
        {CopyError::SrcOffset_01730,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-01730"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-01730"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-00209"},
         }}},
        {CopyError::DstOffset_01732,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-01732"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-01732"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-00207"},
         }}},
        {CopyError::DstOffset_01733,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-01733"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-01733"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-00208"},
         }}},
        {CopyError::DstOffset_01734,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-01734"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-01734"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-00209"},
         }}},
        {CopyError::SrcImageContiguous_07966,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-07966"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-07966"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-07966"},
         }}},
        {CopyError::DstImageContiguous_07966,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-07966"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-07966"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-07966"},
         }}},
        {CopyError::SrcImageSubsampled_07969,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-07969"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-07969"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-07969"},
         }}},
        {CopyError::DstImageSubsampled_07969,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-07969"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-07969"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-07969"},
         }}},
        {CopyError::SrcOffset_07278,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07278"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07278"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-07274"},
         }}},
        {CopyError::SrcOffset_07279,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07279"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07279"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-07275"},
         }}},
        {CopyError::SrcOffset_07280,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07280"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07280"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-07276"},
         }}},
        {CopyError::DstOffset_07281,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07281"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07281"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-07274"},
         }}},
        {CopyError::DstOffset_07282,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07282"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07282"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-07275"},
         }}},
        {CopyError::DstOffset_07283,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-pRegions-07283"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-pRegions-07283"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-07276"},
         }}},
        {CopyError::SrcSubresource_00142,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-aspectMask-00142"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-aspectMask-00142"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcSubresource-09105"},
         }}},
        {CopyError::DstSubresource_00143,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-aspectMask-00143"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-aspectMask-00143"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstSubresource-09105"},
         }}},
        {CopyError::SrcOffset_00144,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcOffset-00144"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcOffset-00144"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07971"},
         }}},
        {CopyError::SrcOffset_00145,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcOffset-00145"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcOffset-00145"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07972"},
         }}},
        {CopyError::SrcOffset_00147,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcOffset-00147"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcOffset-00147"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcOffset-09104"},
         }}},
        {CopyError::DstOffset_00150,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstOffset-00150"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstOffset-00150"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07971"},
         }}},
        {CopyError::DstOffset_00151,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstOffset-00151"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstOffset-00151"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07972"},
         }}},
        {CopyError::DstOffset_00153,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstOffset-00153"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstOffset-00153"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstOffset-09104"},
         }}},
        {CopyError::SrcImage3D_04443,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-srcImage-04443"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-srcImage-04443"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-srcImage-07983"},
         }}},
        {CopyError::DstImage3D_04444,
         {{
             {Key(Func::vkCmdCopyImage), "VUID-vkCmdCopyImage-dstImage-04444"},
             {Key(Func::vkCmdCopyImage2), "VUID-VkCopyImageInfo2-dstImage-04444"},
             {Key(Func::vkCopyImageToImageEXT), "VUID-VkCopyImageToImageInfoEXT-dstImage-07983"},
         }}},
    };

    const auto &result = FindVUID(error, loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-copy-buffer");
        return unhandled;
    }
    return result;
}

const std::string &GetImageMipLevelVUID(const Location &loc) {
    static const std::array<Entry, 20> errors{{
        {Key(Func::vkCmdCopyImage, Field::srcSubresource), "VUID-vkCmdCopyImage-srcSubresource-07967"},
        {Key(Func::vkCmdCopyImage, Field::dstSubresource), "VUID-vkCmdCopyImage-dstSubresource-07967"},
        {Key(Func::vkCmdCopyImage2, Field::srcSubresource), "VUID-VkCopyImageInfo2-srcSubresource-07967"},
        {Key(Func::vkCmdCopyImage2, Field::dstSubresource), "VUID-VkCopyImageInfo2-dstSubresource-07967"},
        {Key(Func::vkCopyImageToImageEXT, Field::srcSubresource), "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07967"},
        {Key(Func::vkCopyImageToImageEXT, Field::dstSubresource), "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07967"},
        {Key(Func::vkCmdBlitImage, Field::srcSubresource), "VUID-vkCmdBlitImage-srcSubresource-01705"},
        {Key(Func::vkCmdBlitImage, Field::dstSubresource), "VUID-vkCmdBlitImage-dstSubresource-01706"},
        {Key(Func::vkCmdBlitImage2, Field::srcSubresource), "VUID-VkBlitImageInfo2-srcSubresource-01705"},
        {Key(Func::vkCmdBlitImage2, Field::dstSubresource), "VUID-VkBlitImageInfo2-dstSubresource-01706"},
        {Key(Func::vkCmdResolveImage, Field::srcSubresource), "VUID-vkCmdResolveImage-srcSubresource-01709"},
        {Key(Func::vkCmdResolveImage, Field::dstSubresource), "VUID-vkCmdResolveImage-dstSubresource-01710"},
        {Key(Func::vkCmdResolveImage2, Field::srcSubresource), "VUID-VkResolveImageInfo2-srcSubresource-01709"},
        {Key(Func::vkCmdResolveImage2, Field::dstSubresource), "VUID-VkResolveImageInfo2-dstSubresource-01710"},
        {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-07967"},
        {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-imageSubresource-07967"},
        {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-07967"},
        {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-imageSubresource-07967"},
        {Key(Func::vkCopyImageToMemoryEXT), "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07967"},
        {Key(Func::vkCopyMemoryToImageEXT), "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07967"},
    }};

    const auto &result = FindVUID(loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-mip-level");
        return unhandled;
    }
    return result;
}

const std::string &GetImageArrayLayerRangeVUID(const Location &loc) {
    static const std::array<Entry, 20> errors{{
        {Key(Func::vkCmdCopyImage, Field::srcSubresource), "VUID-vkCmdCopyImage-srcSubresource-07968"},
        {Key(Func::vkCmdCopyImage, Field::dstSubresource), "VUID-vkCmdCopyImage-dstSubresource-07968"},
        {Key(Func::vkCmdCopyImage2, Field::srcSubresource), "VUID-VkCopyImageInfo2-srcSubresource-07968"},
        {Key(Func::vkCmdCopyImage2, Field::dstSubresource), "VUID-VkCopyImageInfo2-dstSubresource-07968"},
        {Key(Func::vkCopyImageToImageEXT, Field::srcSubresource), "VUID-VkCopyImageToImageInfoEXT-srcSubresource-07968"},
        {Key(Func::vkCopyImageToImageEXT, Field::dstSubresource), "VUID-VkCopyImageToImageInfoEXT-dstSubresource-07968"},
        {Key(Func::vkCmdBlitImage, Field::srcSubresource), "VUID-vkCmdBlitImage-srcSubresource-01707"},
        {Key(Func::vkCmdBlitImage, Field::dstSubresource), "VUID-vkCmdBlitImage-dstSubresource-01708"},
        {Key(Func::vkCmdBlitImage2, Field::srcSubresource), "VUID-VkBlitImageInfo2-srcSubresource-01707"},
        {Key(Func::vkCmdBlitImage2, Field::dstSubresource), "VUID-VkBlitImageInfo2-dstSubresource-01708"},
        {Key(Func::vkCmdResolveImage, Field::srcSubresource), "VUID-vkCmdResolveImage-srcSubresource-01711"},
        {Key(Func::vkCmdResolveImage, Field::dstSubresource), "VUID-vkCmdResolveImage-dstSubresource-01712"},
        {Key(Func::vkCmdResolveImage2, Field::srcSubresource), "VUID-VkResolveImageInfo2-srcSubresource-01711"},
        {Key(Func::vkCmdResolveImage2, Field::dstSubresource), "VUID-VkResolveImageInfo2-dstSubresource-01712"},
        {Key(Func::vkCmdCopyImageToBuffer), "VUID-vkCmdCopyImageToBuffer-imageSubresource-07968"},
        {Key(Func::vkCmdCopyImageToBuffer2), "VUID-VkCopyImageToBufferInfo2-imageSubresource-07968"},
        {Key(Func::vkCmdCopyBufferToImage), "VUID-vkCmdCopyBufferToImage-imageSubresource-07968"},
        {Key(Func::vkCmdCopyBufferToImage2), "VUID-VkCopyBufferToImageInfo2-imageSubresource-07968"},
        {Key(Func::vkCopyImageToMemoryEXT), "VUID-VkCopyImageToMemoryInfoEXT-imageSubresource-07968"},
        {Key(Func::vkCopyMemoryToImageEXT), "VUID-VkCopyMemoryToImageInfoEXT-imageSubresource-07968"},
    }};

    const auto &result = FindVUID(loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-array-layer-range");
        return unhandled;
    }
    return result;
}

const std::string &GetSubresourceRangeVUID(const Location &loc, SubresourceRangeError error) {
    static const std::map<SubresourceRangeError, std::array<Entry, 6>> errors{
        {SubresourceRangeError::BaseMip_01486,
         {{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-01486"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-01486"},
             {Key(Func::vkTransitionImageLayoutEXT), "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-01486"},
             {Key(Func::vkCmdClearColorImage), "VUID-vkCmdClearColorImage-baseMipLevel-01470"},
             {Key(Func::vkCmdClearDepthStencilImage), "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474"},
             {Key(Func::vkCreateImageView), "VUID-VkImageViewCreateInfo-subresourceRange-01478"},
         }}},
        {SubresourceRangeError::MipCount_01724,
         {{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-01724"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-01724"},
             {Key(Func::vkTransitionImageLayoutEXT), "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-01724"},
             {Key(Func::vkCmdClearColorImage), "VUID-vkCmdClearColorImage-pRanges-01692"},
             {Key(Func::vkCmdClearDepthStencilImage), "VUID-vkCmdClearDepthStencilImage-pRanges-01694"},
             {Key(Func::vkCreateImageView), "VUID-VkImageViewCreateInfo-subresourceRange-01718"},
         }}},
        {SubresourceRangeError::BaseLayer_01488,
         {{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-01488"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-01488"},
             {Key(Func::vkTransitionImageLayoutEXT), "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-01488"},
             {Key(Func::vkCmdClearColorImage), "VUID-vkCmdClearColorImage-baseArrayLayer-01472"},
             {Key(Func::vkCmdClearDepthStencilImage), "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476"},
             {Key(Func::vkCreateImageView), "VUID-VkImageViewCreateInfo-image-06724"},
         }}},
        {SubresourceRangeError::LayerCount_01725,
         {{
             {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-subresourceRange-01725"},
             {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-subresourceRange-01725"},
             {Key(Func::vkTransitionImageLayoutEXT), "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-01725"},
             {Key(Func::vkCmdClearColorImage), "VUID-vkCmdClearColorImage-pRanges-01693"},
             {Key(Func::vkCmdClearDepthStencilImage), "VUID-vkCmdClearDepthStencilImage-pRanges-01695"},
             {Key(Func::vkCreateImageView), "VUID-VkImageViewCreateInfo-subresourceRange-06725"},
         }}},
    };

    const auto &result = FindVUID(error, loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-subresource-range");
        return unhandled;
    }
    return result;
}

}  // namespace vvl