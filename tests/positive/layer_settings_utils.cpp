
/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "layer_settings_utils.h"

class PositiveLayerSettingUtils : public VkPositiveLayerTest {};

// These test check utils in the layer without needing to create a full Vulkan instance

TEST_F(PositiveLayerSettingUtils, GetNextToken_list) {
	TEST_DESCRIPTION("Test GetNextToken");

	std::string token_list = "token1,token2";
	std::size_t pos = 0;

	ASSERT_STREQ("token1", GetNextToken(&token_list, ",", &pos).c_str());
	ASSERT_STREQ("token2", GetNextToken(&token_list, ",", &pos).c_str());
	ASSERT_TRUE(token_list.empty());

    ASSERT_STREQ("", GetNextToken(&token_list, ",", &pos).c_str()); // token_list is empty, return an empty value
}

TEST_F(PositiveLayerSettingUtils, FindDelimiter) {
    TEST_DESCRIPTION("Test FindDelimiter");

    ASSERT_STREQ(",", FindDelimiter("token1,token2").c_str());
    ASSERT_STREQ(";", FindDelimiter("token1;token2").c_str());
    ASSERT_STREQ(":", FindDelimiter("token1:token2").c_str());
    ASSERT_STREQ(",", FindDelimiter("token1").c_str()); // If the token doesn't have a known seprator, we return "," by default
    ASSERT_STREQ(",", FindDelimiter("").c_str());
}

TEST_F(PositiveLayerSettingUtils, TokenToUint) {
    TEST_DESCRIPTION("Test TokenToUint");

    ASSERT_EQ(255, TokenToUint("0xFF"));
    ASSERT_EQ(255, TokenToUint("0XFF"));
    ASSERT_EQ(255, TokenToUint("255"));
}
