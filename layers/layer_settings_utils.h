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

#pragma once

#include "layer_options.h"
#include <cstdlib>
#include <cassert>

static inline std::string GetNextToken(std::string *token_list, const std::string &delimiter, size_t *pos) {
    assert(!delimiter.empty());

    std::string token;
    *pos = token_list->find(delimiter);
    if (*pos != std::string::npos) {
        token = token_list->substr(0, *pos);
    } else {
        *pos = token_list->length() - delimiter.length();
        token = *token_list;
    }
    token_list->erase(0, *pos + delimiter.length());

    // Remove quotes from quoted strings
    if ((token.length() > 0) && (token[0] == '\"')) {
        token.erase(token.begin());
        if ((token.length() > 0) && (token[token.length() - 1] == '\"')) {
            token.erase(--token.end());
        }
    }
    return token;
}

static inline std::string FindDelimiter(const std::string &setting_values) {
    if (setting_values.find(";")) {
        return ";"; // Typically WIN32 env var
    } else if (setting_values.find(":")) {
        return ":"; // Typically UNIX env var
    } else {
        return ",";
    }
}

static inline uint32_t TokenToUint(const std::string &token) {
    assert(!token.empty());

    uint32_t int_id = 0;
    if ((token.find("0x") == 0) || token.find("0X") == 0) {  // Handle hex format
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}
