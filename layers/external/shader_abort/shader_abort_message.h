// SPDX-FileCopyrightText: 2026 Spencer Fricke spencer@lunarg.com
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Every message is a [size, payload] pair
typedef struct ShaderAbortMessage {
    uint64_t size;            // Size in bytes of the payload
    const uint8_t* pPayload;  // Points into the original pMessageData (nothing is copied)
} ShaderAbortMessage;

/**
 * Walks pMessageData and returns the next ShaderAbortMessage
 *
 * Start with *pOffset = 0 and call until it returns false.
 * Truncated or corrupt data ends the iteration.
 *
 * @param pMessageData VkDeviceFaultShaderAbortMessageInfoKHR::pMessageData
 * @param messageDataSize VkDeviceFaultShaderAbortMessageInfoKHR::messageDataSize (in bytes)
 * @param pOffset In/out, where to read from, is moved to the start of the next pair
 * @param pMessage Filled in with the next message
 * @return true if a message was returned
 */
bool shader_abort_next_message(const void* pMessageData, uint64_t messageDataSize, uint64_t* pOffset, ShaderAbortMessage* pMessage);

/**
 * The first message in pMessageData, same as shader_abort_next_message() with offset of zero
 *
 * @return true if there was a message
 */
bool shader_abort_first_message(const void* pMessageData, uint64_t messageDataSize, ShaderAbortMessage* pMessage);

/**
 * For a message that is just a string like
 *     abort("case 4 was hit");
 * returns that string with no formatting and no copy.
 *
 * @return The string, or NULL if the message is not simple
 */
const char* shader_abort_message_text(const ShaderAbortMessage* pMessage);

/**
 * Formats one message, like snprintf(). The format string and arguments are both read from the payload.
 *
 * @param pMessage A message from shader_abort_next_message()
 * @param pOut Where to write the text, can be NULL if outSize is 0
 * @param outSize Size of pOut in bytes. Always null-terminated if outSize > 0, truncated if needed
 * @return The length the full text needs, not counting the null terminator. If this is >= outSize it was truncated.
 */
size_t shader_abort_format_message(const ShaderAbortMessage* pMessage, char* pOut, size_t outSize);

/**
 * Called by shader_abort_for_each_message() once per message
 *
 * @param pText The null-terminated formatted message, only valid during the callback
 * @param pUserData The pUserData that was given to shader_abort_for_each_message()
 */
typedef void (*ShaderAbortMessageCallback)(const char* pText, void* pUserData);

/**
 * Convenience: formats every message in pMessageData and calls pCallback once for each.
 *
 * @param pMessageData VkDeviceFaultShaderAbortMessageInfoKHR::pMessageData
 * @param messageDataSize VkDeviceFaultShaderAbortMessageInfoKHR::messageDataSize (in bytes)
 * @param pCallback Called with the formatted text of each message
 * @param pUserData Passed to pCallback as is
 * @return The number of messages found
 */
size_t shader_abort_for_each_message(const void* pMessageData, uint64_t messageDataSize, ShaderAbortMessageCallback pCallback,
                                     void* pUserData);

#ifdef __cplusplus
}
#endif
