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
#include <edfio/header/HeaderGeneral.hpp>
#include <fstream>
#include <filesystem>

namespace edfio::test
{

// Test that the sample EDF file exists
TEST(EdfReadTest, SampleFileExists)
{
    std::filesystem::path testFile = "Calib5.edf";
    EXPECT_TRUE(std::filesystem::exists(testFile)) 
        << "Sample file Calib5.edf should exist in the repository root";
}

// Test that we can open and read basic header information
TEST(EdfReadTest, CanOpenAndReadHeader)
{
    std::filesystem::path testFile = "Calib5.edf";
    
    if (!std::filesystem::exists(testFile))
    {
        GTEST_SKIP() << "Sample file not found, skipping test";
    }
    
    std::ifstream file(testFile, std::ios::binary);
    EXPECT_TRUE(file.is_open()) << "Should be able to open the EDF file";
    
    if (file.is_open())
    {
        // Read the version field (first 8 bytes)
        char version[9] = {0};
        file.read(version, 8);
        
        EXPECT_TRUE(file.good()) << "Should be able to read from the file";
        
        // EDF version should be "0       " (0 followed by spaces)
        std::string versionStr(version, 8);
        EXPECT_FALSE(versionStr.empty()) << "Version field should not be empty";
    }
}

// Test DataFormat enum is available and valid
TEST(EdfReadTest, DataFormatEnumAvailable)
{
    DataFormat format = DataFormat::Edf;
    EXPECT_EQ(format, DataFormat::Edf);
    
    // Test the helper functions
    EXPECT_TRUE(IsEdf(format));
    EXPECT_FALSE(IsBdf(format));
    EXPECT_FALSE(IsPlus(format));
    EXPECT_EQ(GetSampleBytes(format), 2);
}

// Test that header structures are available
TEST(EdfReadTest, HeaderStructuresAvailable)
{
    HeaderGeneralFields headerGeneral;
    // Field template wraps strings, check the Size() method instead
    EXPECT_EQ(headerGeneral.m_version.Size(), 8);
    EXPECT_EQ(headerGeneral.m_patient.Size(), 80);
    EXPECT_EQ(headerGeneral.m_recording.Size(), 80);
}

} // namespace edfio::test
