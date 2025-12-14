//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#include <gtest/gtest.h>
#include <edfio/core/SampleType.hpp>

namespace edfio::test
{

// Test SampleType enum values
TEST(SampleTypeTest, EnumValues)
{
    EXPECT_EQ(SampleType::Physical, SampleType::Physical);
    EXPECT_EQ(SampleType::Digital, SampleType::Digital);
    EXPECT_NE(SampleType::Physical, SampleType::Digital);
}

// Test Sample type traits
TEST(SampleTypeTest, SampleTypeTraits)
{
    using PhysicalType = impl::Sample<SampleType::Physical>::type;
    using DigitalType = impl::Sample<SampleType::Digital>::type;
    
    EXPECT_TRUE((std::is_same_v<PhysicalType, double>));
    EXPECT_TRUE((std::is_same_v<DigitalType, int>));
}

// Test sample conversion from physical to digital
TEST(SampleTypeTest, ConvertPhysicalToDigital)
{
    constexpr double offset = 0.0;
    constexpr double scaling = 1.0;
    
    impl::Sample<SampleType::Physical>::type physicalSample = 100.0;
    auto digitalSample = impl::ConvertSample(offset, scaling, physicalSample);
    
    EXPECT_EQ(digitalSample, 100);
}

// Test sample conversion from digital to physical
TEST(SampleTypeTest, ConvertDigitalToPhysical)
{
    constexpr double offset = 0.0;
    constexpr double scaling = 1.0;
    
    impl::Sample<SampleType::Digital>::type digitalSample = 100;
    auto physicalSample = impl::ConvertSample(offset, scaling, digitalSample);
    
    EXPECT_DOUBLE_EQ(physicalSample, 100.0);
}

// Test sample conversion with offset and scaling
TEST(SampleTypeTest, ConvertWithOffsetAndScaling)
{
    constexpr double offset = 10.0;
    constexpr double scaling = 0.5;
    
    // Digital to Physical: result = scaling * digital + offset
    impl::Sample<SampleType::Digital>::type digitalSample = 100;
    auto physicalSample = impl::ConvertSample(offset, scaling, digitalSample);
    EXPECT_DOUBLE_EQ(physicalSample, 60.0); // 0.5 * 100 + 10 = 60
    
    // Physical to Digital: result = (physical - offset) / scaling
    impl::Sample<SampleType::Physical>::type physicalValue = 60.0;
    auto digitalValue = impl::ConvertSample(offset, scaling, physicalValue);
    EXPECT_EQ(digitalValue, 100); // (60 - 10) / 0.5 = 100
}

// Test round-trip conversion
TEST(SampleTypeTest, RoundTripConversion)
{
    constexpr double offset = 5.0;
    constexpr double scaling = 2.5;
    
    impl::Sample<SampleType::Digital>::type originalDigital = 42;
    
    // Convert digital to physical and back
    auto physical = impl::ConvertSample(offset, scaling, originalDigital);
    auto convertedDigital = impl::ConvertSample(offset, scaling, physical);
    
    EXPECT_EQ(originalDigital, convertedDigital);
}

} // namespace edfio::test
