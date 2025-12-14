//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#include <gtest/gtest.h>
#include <edfio/core/DataFormat.hpp>

namespace edfio::test
{

// Test DataFormat enum values
TEST(DataFormatTest, EnumValues)
{
    EXPECT_NE(DataFormat::Edf, DataFormat::EdfPlusC);
    EXPECT_NE(DataFormat::Edf, DataFormat::EdfPlusD);
    EXPECT_NE(DataFormat::Bdf, DataFormat::BdfPlusC);
    EXPECT_NE(DataFormat::Bdf, DataFormat::BdfPlusD);
    EXPECT_NE(DataFormat::Invalid, DataFormat::Edf);
}

// Test IsPlus function
TEST(DataFormatTest, IsPlusFunction)
{
    EXPECT_FALSE(IsPlus(DataFormat::Edf));
    EXPECT_TRUE(IsPlus(DataFormat::EdfPlusC));
    EXPECT_TRUE(IsPlus(DataFormat::EdfPlusD));
    EXPECT_FALSE(IsPlus(DataFormat::Bdf));
    EXPECT_TRUE(IsPlus(DataFormat::BdfPlusC));
    EXPECT_TRUE(IsPlus(DataFormat::BdfPlusD));
    EXPECT_FALSE(IsPlus(DataFormat::Invalid));
}

// Test IsEdf function
TEST(DataFormatTest, IsEdfFunction)
{
    EXPECT_TRUE(IsEdf(DataFormat::Edf));
    EXPECT_TRUE(IsEdf(DataFormat::EdfPlusC));
    EXPECT_TRUE(IsEdf(DataFormat::EdfPlusD));
    EXPECT_FALSE(IsEdf(DataFormat::Bdf));
    EXPECT_FALSE(IsEdf(DataFormat::BdfPlusC));
    EXPECT_FALSE(IsEdf(DataFormat::BdfPlusD));
    EXPECT_FALSE(IsEdf(DataFormat::Invalid));
}

// Test IsBdf function
TEST(DataFormatTest, IsBdfFunction)
{
    EXPECT_FALSE(IsBdf(DataFormat::Edf));
    EXPECT_FALSE(IsBdf(DataFormat::EdfPlusC));
    EXPECT_FALSE(IsBdf(DataFormat::EdfPlusD));
    EXPECT_TRUE(IsBdf(DataFormat::Bdf));
    EXPECT_TRUE(IsBdf(DataFormat::BdfPlusC));
    EXPECT_TRUE(IsBdf(DataFormat::BdfPlusD));
    EXPECT_FALSE(IsBdf(DataFormat::Invalid));
}

// Test GetSampleBytes function
TEST(DataFormatTest, GetSampleBytesFunction)
{
    // EDF formats use 2 bytes per sample
    EXPECT_EQ(GetSampleBytes(DataFormat::Edf), 2);
    EXPECT_EQ(GetSampleBytes(DataFormat::EdfPlusC), 2);
    EXPECT_EQ(GetSampleBytes(DataFormat::EdfPlusD), 2);
    
    // BDF formats use 3 bytes per sample
    EXPECT_EQ(GetSampleBytes(DataFormat::Bdf), 3);
    EXPECT_EQ(GetSampleBytes(DataFormat::BdfPlusC), 3);
    EXPECT_EQ(GetSampleBytes(DataFormat::BdfPlusD), 3);
    
    // Invalid format returns -1
    EXPECT_EQ(GetSampleBytes(DataFormat::Invalid), -1);
}

} // namespace edfio::test
