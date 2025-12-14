//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#include <edfio/EdfIO.hpp>
#include <iostream>
#include <filesystem>

int main()
{
    std::cout << "edfio - C++20 header-only library for EDF(+)/BDF(+) files\n";
    std::cout << "===========================================================\n\n";
    
    // Check if sample file exists
    std::filesystem::path sampleFile = "../Calib5.edf";
    
    if (std::filesystem::exists(sampleFile))
    {
        std::cout << "Sample file found: " << sampleFile << "\n";
        std::cout << "File size: " << std::filesystem::file_size(sampleFile) << " bytes\n";
    }
    else
    {
        std::cout << "Sample file not found at: " << sampleFile << "\n";
        std::cout << "Please run this example from the build directory.\n";
    }
    
    // Demonstrate DataFormat enum usage
    std::cout << "\nDataFormat capabilities:\n";
    std::cout << "- EDF sample size: " << edfio::GetSampleBytes(edfio::DataFormat::Edf) << " bytes\n";
    std::cout << "- BDF sample size: " << edfio::GetSampleBytes(edfio::DataFormat::Bdf) << " bytes\n";
    
    // Demonstrate SampleType usage
    std::cout << "\nSample conversion example:\n";
    constexpr double offset = 0.0;
    constexpr double scaling = 1.0;
    
    edfio::impl::Sample<edfio::SampleType::Digital>::type digitalValue = 100;
    auto physicalValue = edfio::impl::ConvertSample(offset, scaling, digitalValue);
    
    std::cout << "- Digital value: " << digitalValue << "\n";
    std::cout << "- Physical value: " << physicalValue << "\n";
    
    std::cout << "\nExample completed successfully!\n";
    
    return 0;
}
