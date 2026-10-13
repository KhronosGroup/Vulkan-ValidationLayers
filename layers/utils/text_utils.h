/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include <cstdarg>
#include <string>
#include <string_view>

namespace text {

// vsprintf style formatting
std::string VFormat(const char *format, va_list argptr);

// sprintf style formatting
std::string Format(const char *format, ...);

void ToLower(std::string &str);
void ToUpper(std::string &str);

// Tries to find `str` against `pattern`
// In a pattern a '*' matches any sequence of characters (including none).
// So "foo*" pattern matches against "foobar" and "foo".
// But "foobar" pattern does not match against "foo".
// See tests in layer_utils_positive.cpp for examples
bool DoesStringMatchesPattern(std::string_view str, std::string_view pattern);

}  // namespace text
