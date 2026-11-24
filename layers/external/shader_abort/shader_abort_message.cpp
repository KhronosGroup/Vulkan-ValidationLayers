// SPDX-FileCopyrightText: 2026 Spencer Fricke spencer@lunarg.com
// SPDX-License-Identifier: MIT

#include "shader_abort_message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t align_up(uint64_t value, uint64_t alignment) { return (value + alignment - 1) & ~(alignment - 1); }

bool shader_abort_next_message(const void* pMessageData, uint64_t messageDataSize, uint64_t* pOffset,
                               ShaderAbortMessage* pMessage) {
    if (pMessageData == NULL || pOffset == NULL || pMessage == NULL) {
        return false;
    }

    uint64_t offset = *pOffset;
    if (offset > messageDataSize || messageDataSize - offset < sizeof(uint64_t)) {
        return false;
    }

    // memcpy as the data is only guaranteed to be aligned if the application allocated it that way
    const uint8_t* pBytes = (const uint8_t*)pMessageData;
    uint64_t payload_size = 0;
    memcpy(&payload_size, pBytes + offset, sizeof(payload_size));
    offset += sizeof(uint64_t);

    // Truncated or corrupt, don't look at anything after it. (A size of 0 is a valid, empty message.)
    if (payload_size > messageDataSize - offset) {
        *pOffset = messageDataSize;
        return false;
    }

    pMessage->size = payload_size;
    pMessage->pPayload = pBytes + offset;

    // Every (size, payload) pair starts at the next 8-byte aligned offset
    *pOffset = align_up(offset + payload_size, 8);
    return true;
}

bool shader_abort_first_message(const void* pMessageData, uint64_t messageDataSize, ShaderAbortMessage* pMessage) {
    uint64_t offset = 0;
    return shader_abort_next_message(pMessageData, messageDataSize, &offset, pMessage);
}

const char* shader_abort_message_text(const ShaderAbortMessage* pMessage) {
    if (pMessage == NULL || pMessage->pPayload == NULL) {
        return NULL;
    }

    const char* pString = (const char*)pMessage->pPayload;
    for (uint64_t i = 0; i < pMessage->size; ++i) {
        if (pString[i] == '\0') {
            return pString;
        }
        if (pString[i] == '%') {
            return NULL;
        }
    }
    return NULL;  // no null terminator, not a string
}

// snprintf() style output, the length is always counted even when the buffer is full
typedef struct MessageWriter {
    char* pOut;
    size_t capacity;
    size_t length;
} MessageWriter;

static void writer_append(MessageWriter* pWriter, const char* pText, size_t count) {
    if (pWriter->pOut != NULL && pWriter->length < pWriter->capacity) {
        size_t space = pWriter->capacity - pWriter->length;
        size_t copy = count < space ? count : space;
        memcpy(pWriter->pOut + pWriter->length, pText, copy);
    }
    pWriter->length += count;
}

// Reads the arguments that follow the format string in the payload
typedef struct MessageArgReader {
    const uint8_t* pPayload;
    uint64_t size;
    uint64_t offset;
} MessageArgReader;

// Reads one 1, 2, 4 or 8 byte scalar, aligned to its own size (scalar layout).
// The bits are returned zero-extended.
// Returns false if it doesn't fit in the payload.
static bool read_scalar(MessageArgReader* pReader, uint64_t size, uint64_t* pBits) {
    uint64_t offset = align_up(pReader->offset, size);
    if (offset > pReader->size || pReader->size - offset < size) {
        return false;
    }
    // Little-endian: the bytes are the low bytes of the value
    *pBits = 0;
    for (uint64_t i = 0; i < size; ++i) {
        *pBits |= (uint64_t)pReader->pPayload[offset + i] << (8 * i);
    }
    pReader->offset = offset + size;
    return true;
}

static int64_t sign_extend(uint64_t bits, uint64_t size) {
    if (size == sizeof(int64_t)) {
        int64_t value;
        memcpy(&value, &bits, sizeof(value));
        return value;
    }
    const uint64_t sign_bit = 1ull << (size * 8 - 1);
    return (bits & sign_bit) ? (int64_t)bits - (int64_t)(sign_bit << 1) : (int64_t)bits;
}

// IEEE 754 half to float, without needing <math.h>
static float half_to_float(uint16_t half) {
    const uint32_t sign = (uint32_t)(half & 0x8000) << 16;
    uint32_t exponent = (half >> 10) & 0x1F;
    uint32_t mantissa = half & 0x3FF;
    uint32_t bits;
    if (exponent == 0x1F) {
        bits = sign | 0x7F800000 | (mantissa << 13);  // infinity, NaN
    } else if (exponent != 0) {
        bits = sign | ((exponent + 127 - 15) << 23) | (mantissa << 13);
    } else if (mantissa == 0) {
        bits = sign;  // zero
    } else {
        // Subnormal half, it is a normal float once the mantissa is shifted up
        exponent = 127 - 15 + 1;
        while ((mantissa & 0x400) == 0) {
            mantissa <<= 1;
            exponent--;
        }
        bits = sign | (exponent << 23) | ((mantissa & 0x3FF) << 13);
    }
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// One parsed "%[flags][width][.precision][vN][hh|h|l|ll|ul]conversion"
typedef struct MessageSpecifier {
    char flags[6];    // the subset of "-+ #0" that was used, each once, null-terminated
    int width;        // 0 if not given
    int precision;    // -1 if not given
    int components;   // 1, or 2 to 4 for "%v3f" style vectors
    uint64_t size;    // bytes of each argument: 1 (hh), 2 (h), 4, or 8 (l, ll, ul, p)
    char conversion;  // one of kConversions, or something else if this isn't a specifier we know
} MessageSpecifier;

static const char kConversions[] = "diuxXoeEfFgGaAp";

// Width and precision are clamped so a single value always fits in format_scalar()'s buffer
// (the longest is a 64-bit float with %f, 309 digits + the precision)
enum { kMaxWidthOrPrecision = 100 };

static const char* parse_number(const char* p, int* pValue) {
    int value = 0;
    while (*p >= '0' && *p <= '9') {
        if (value < kMaxWidthOrPrecision) {
            value = value * 10 + (*p - '0');
        }
        p++;
    }
    *pValue = value < kMaxWidthOrPrecision ? value : kMaxWidthOrPrecision;
    return p;
}

// |p| points at the character after the '%'.
// Relies on the string being null-terminated, the parsing stops there.
// Returns where the specifier ends.
static const char* parse_specifier(const char* p, MessageSpecifier* pSpec) {
    memset(pSpec, 0, sizeof(*pSpec));
    pSpec->precision = -1;
    pSpec->components = 1;
    pSpec->size = 4;

    size_t flag_count = 0;
    while (*p != '\0' && strchr("-+ #0", *p) != NULL) {
        if (strchr(pSpec->flags, *p) == NULL) {
            pSpec->flags[flag_count++] = *p;
        }
        p++;
    }
    p = parse_number(p, &pSpec->width);
    if (*p == '.') {
        p = parse_number(p + 1, &pSpec->precision);
    }

    if (p[0] == 'v' && p[1] >= '2' && p[1] <= '4') {
        pSpec->components = p[1] - '0';
        p += 2;
    }

    // The size, like C: "hh" 8-bit, "h" 16-bit, "l" / "ll" 64-bit. Also the debugPrintf style "ul" for 64-bit.
    if (p[0] == 'u' && p[1] == 'l') {
        pSpec->size = 8;
        pSpec->conversion = 'u';
        return p + 2;
    }
    if (p[0] == 'l') {
        pSpec->size = 8;
        p += (p[1] == 'l') ? 2 : 1;
    } else if (p[0] == 'h') {
        pSpec->size = (p[1] == 'h') ? 1 : 2;
        p += (p[1] == 'h') ? 2 : 1;
    }
    pSpec->conversion = *p;
    return *p != '\0' ? p + 1 : p;
}

// Builds the snprintf() format for |pSpec|.
// Width and precision are passed as "*" arguments, so the shader's own text never goes into the format
// and only flags that are well defined for the conversion are kept.
static void make_printf_format(const MessageSpecifier* pSpec, const char* pLengthModifier, char conversion, char* pOut,
                               size_t outSize) {
    char flags[sizeof(pSpec->flags)] = {0};
    size_t count = 0;
    for (const char* f = pSpec->flags; *f != '\0'; ++f) {
        if (*f == '#' && strchr("diu", conversion) != NULL) {
            continue;  // '#' with d, i or u is undefined behavior in C
        }
        if ((*f == '+' || *f == ' ') && strchr("uxXo", conversion) != NULL) {
            continue;  // a sign only means something for a signed conversion, and not every C library ignores it
        }
        flags[count++] = *f;
    }
    snprintf(pOut, outSize, "%%%s*.*%s%c", flags, pLengthModifier, conversion);
}

// Reads one argument (a single scalar) and appends it as text.
// Returns false if the payload ran out of data.
static bool format_scalar(const MessageSpecifier* pSpec, MessageArgReader* pReader, MessageWriter* pWriter) {
    uint64_t bits = 0;
    if (!read_scalar(pReader, pSpec->size, &bits)) {
        return false;
    }

    const char conversion = pSpec->conversion;
    char format[24];
    char text[512];
    int length = 0;

    if (conversion == 'p') {
        length = snprintf(text, sizeof(text), "0x%llx", (unsigned long long)bits);
    } else if (strchr("eEfFgGaA", conversion) != NULL) {
        double value = 0.0;
        if (pSpec->size == 8) {
            memcpy(&value, &bits, sizeof(value));
        } else if (pSpec->size == 4) {
            const uint32_t bits32 = (uint32_t)bits;
            float value32 = 0.0f;
            memcpy(&value32, &bits32, sizeof(value32));
            value = (double)value32;
        } else {
            value = (double)half_to_float((uint16_t)bits);  // "%hf", there is no 8-bit float
        }
        make_printf_format(pSpec, "", conversion, format, sizeof(format));
        length = snprintf(text, sizeof(text), format, pSpec->width, pSpec->precision, value);
    } else if (conversion == 'd' || conversion == 'i') {
        const int64_t value = sign_extend(bits, pSpec->size);
        make_printf_format(pSpec, "ll", 'd', format, sizeof(format));
        length = snprintf(text, sizeof(text), format, pSpec->width, pSpec->precision, (long long)value);
    } else {
        // u, x, X, o. A smaller value was zero-extended, so printing it as 64-bit gives the same text.
        make_printf_format(pSpec, "ll", conversion, format, sizeof(format));
        length = snprintf(text, sizeof(text), format, pSpec->width, pSpec->precision, (unsigned long long)bits);
    }

    if (length > 0) {
        // Can't overflow with the clamped width and precision, but never trust that
        size_t count = (size_t)length < sizeof(text) ? (size_t)length : sizeof(text) - 1;
        writer_append(pWriter, text, count);
    }
    return true;
}

// Handles one "%..." specifier. |pFormat| points at the character after the '%'.
// Returns where to continue reading the format string from.
static const char* format_specifier(const char* pFormat, MessageArgReader* pReader, MessageWriter* pWriter) {
    if (*pFormat == '%') {
        writer_append(pWriter, "%", 1);
        return pFormat + 1;
    }

    MessageSpecifier spec;
    const char* pEnd = parse_specifier(pFormat, &spec);

    const bool is_float = spec.conversion != '\0' && strchr("eEfFgGaA", spec.conversion) != NULL;
    if (spec.conversion == '\0' || strchr(kConversions, spec.conversion) == NULL || (is_float && spec.size == 1)) {
        // Not something we know how to read (there is no 8-bit float), print it as it was written (including the '%')
        writer_append(pWriter, pFormat - 1, (size_t)(pEnd - (pFormat - 1)));
        return pEnd;
    }
    if (spec.conversion == 'p') {
        spec.size = 8;
    }

    for (int i = 0; i < spec.components; ++i) {
        if (i != 0) {
            writer_append(pWriter, ", ", 2);
        }
        if (!format_scalar(&spec, pReader, pWriter)) {
            writer_append(pWriter, "<missing arg>", 13);
            break;
        }
    }
    return pEnd;
}

size_t shader_abort_format_message(const ShaderAbortMessage* pMessage, char* pOut, size_t outSize) {
    MessageWriter writer = {pOut, outSize > 0 ? outSize - 1 : 0, 0};  // always leave room for the null terminator

    if (pMessage != NULL && pMessage->pPayload != NULL) {
        const char* pFormat = (const char*)pMessage->pPayload;

        // Find the end of the format string, without ever looking outside the payload
        size_t format_length = 0;
        while (format_length < pMessage->size && pFormat[format_length] != '\0') {
            format_length++;
        }

        if (format_length == pMessage->size) {
            // No null terminator, so this isn't a format string and there is no way to know where arguments would
            // start. Print the bytes as they are.
            writer_append(&writer, pFormat, format_length);
        } else {
            // The arguments start after the string, padded to 4 bytes
            MessageArgReader reader = {pMessage->pPayload, pMessage->size, align_up(format_length + 1, 4)};

            const char* pEnd = pFormat + format_length;
            const char* p = pFormat;
            while (p < pEnd) {
                const char* pPercent = (const char*)memchr(p, '%', (size_t)(pEnd - p));
                if (pPercent == NULL) {
                    writer_append(&writer, p, (size_t)(pEnd - p));
                    break;
                }
                writer_append(&writer, p, (size_t)(pPercent - p));
                p = format_specifier(pPercent + 1, &reader, &writer);
            }
        }
    }

    size_t total = writer.length;
    if (pOut != NULL && outSize > 0) {
        pOut[total < outSize - 1 ? total : outSize - 1] = '\0';
    }
    return total;
}

size_t shader_abort_for_each_message(const void* pMessageData, uint64_t messageDataSize, ShaderAbortMessageCallback pCallback,
                                     void* pUserData) {
    size_t count = 0;
    uint64_t offset = 0;
    ShaderAbortMessage message;
    while (shader_abort_next_message(pMessageData, messageDataSize, &offset, &message)) {
        // 512 is not from the spec, just a size big enough for the usual short message so the common case never
        // allocates. It is not a limit: shader_abort_format_message() returns the length it needs (like snprintf), so a
        // longer message is formatted again into a heap buffer of that size and is never cut off.
        char stack_buffer[512];
        char* pText = stack_buffer;
        size_t needed = shader_abort_format_message(&message, stack_buffer, sizeof(stack_buffer));

        char* pHeap = NULL;
        if (needed >= sizeof(stack_buffer)) {
            pHeap = (char*)malloc(needed + 1);
            if (pHeap != NULL) {
                shader_abort_format_message(&message, pHeap, needed + 1);
                pText = pHeap;
            }  // else just give them the truncated text
        }

        if (pCallback != NULL) {
            pCallback(pText, pUserData);
        }
        free(pHeap);
        count++;
    }
    return count;
}
