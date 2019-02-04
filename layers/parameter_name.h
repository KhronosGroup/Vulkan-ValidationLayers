/* Copyright (c) 2016-2019 The Khronos Group Inc.
 * Copyright (c) 2016-2019 Valve Corporation
 * Copyright (c) 2016-2019 LunarG, Inc.
 * Copyright (c) 2016-2019 Google Inc.
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
#ifndef PARAMETER_NAME_H
#define PARAMETER_NAME_H

#include <cassert>
#include <sstream>
#include <string>
#include <vector>

/**
 * Parameter name string supporting deferred formatting for array subscripts.
 *
 * Custom parameter name class with support for deferred formatting of names containing array subscripts.  The class stores
 * a format string and a pointer to an array of index values, and performs string formatting when an accessor function is called to
 * retrieve the name string.  This class was primarily designed to be used with validation functions that receive a parameter name
 * string and value as arguments, and print an error message that includes the parameter name when the value fails a validation
 * test.  Using standard strings with these validation functions requires that parameter names containing array subscripts be
 * formatted before each validation function is called, performing the string formatting even when the value passes validation
 * and the string is not used:
 *         sprintf(name, "pCreateInfo[%d].sType", i);
 *         validate_stype(name, pCreateInfo[i].sType);
 *
 * With the ParameterName class, a format string and a pointer to an array of format values are stored by the ParameterName object
 * that is provided to the validation function.  String formatting is then performed only when the validation function retrieves the
 * name string from the ParameterName object:
 *         validate_stype(ParameterName("pCreateInfo[%i].sType", IndexVector{ i }), pCreateInfo[i].sType);
 *
 * Since the IndexVector is not copied into the object, the lifetime of the ParameterName should not outlast the lifetime of
 * the IndexVector, but that's fine given how it is used in parameter validation.
 */
class ParameterName {
   public:
    /// Container for index values to be used with parameter name string formatting.
    typedef std::initializer_list<size_t> IndexVector;

    /// Format specifier for the parameter name string, to be replaced by an index value.  The parameter name string must contain
    /// one format specifier for each index value specified.
    const char *const IndexFormatSpecifier = "%i";

   public:
    /**
     * Construct a ParameterName object from a string literal, without formatting.
     *
     * @param source Paramater name string without format specifiers.
     *
     * @pre The source string must not contain the %i format specifier.
     */
    ParameterName(const char *source) : source_(source), num_indices_(0) { assert(IsValid()); }

    /**
     * Construct a ParameterName object from a string literal, with formatting.
     *
     * @param source Paramater name string with format specifiers.
     * @param args Array index values to be used for formatting.
     *
     * @pre The number of %i format specifiers contained by the source string must match the number of elements contained
     *      by the index vector.
     */
    ParameterName(const char *source, const IndexVector &args)
        : source_(source), args_(args.size() ? args.begin() : (const size_t *)nullptr), num_indices_(args.size()) {
        assert(IsValid());
    }

    /// Retrive the formatted name string.
    std::string get_name() const { return (num_indices_ == 0) ? std::string(source_) : Format(); }

   private:
    /// Replace the %i format specifiers in the source string with the values from the index vector.
    std::string Format() const {
        std::string::size_type current = 0;
        std::string::size_type last = 0;
        std::stringstream format;

        std::string source(source_);

        for (size_t i = 0; i < num_indices_; ++i) {
            auto index = args_[i];
            current = source.find(IndexFormatSpecifier, last);
            if (current == std::string::npos) {
                break;
            }
            format << source.substr(last, (current - last)) << index;
            last = current + strlen(IndexFormatSpecifier);
        }

        format << source.substr(last, std::string::npos);

        return format.str();
    }

    /// Check that the number of %i format specifiers in the source string matches the number of elements in the index vector.
    bool IsValid() {
        // Count the number of occurances of the format specifier
        uint32_t count = 0;

        std::string source(source_);

        std::string::size_type pos = source.find(IndexFormatSpecifier);

        while (pos != std::string::npos) {
            ++count;
            pos = source.find(IndexFormatSpecifier, pos + 1);
        }

        return (count == num_indices_);
    }

   private:
    const char *source_;  ///< Format string.
    const size_t *args_;  ///< Array index values for formatting.
    size_t num_indices_;  ///< Number of array index values.
};

#endif  // PARAMETER_NAME_H
