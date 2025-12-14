//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#include <gtest/gtest.h>
#include <edfio/Config.hpp>

namespace edfio::test
{

// Test ProcessorErrorCheck enum values
TEST(ConfigTest, ProcessorErrorCheckEnum)
{
    EXPECT_EQ(ProcessorErrorCheck::Strict, ProcessorErrorCheck::Strict);
    EXPECT_EQ(ProcessorErrorCheck::Permissive, ProcessorErrorCheck::Permissive);
    EXPECT_NE(ProcessorErrorCheck::Strict, ProcessorErrorCheck::Permissive);
}

// Test default configuration
TEST(ConfigTest, DefaultConfiguration)
{
    EXPECT_EQ(config::PROCESSOR_ERROR_CHECKING, ProcessorErrorCheck::Strict);
}

} // namespace edfio::test
