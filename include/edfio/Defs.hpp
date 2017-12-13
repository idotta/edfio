//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

namespace edfio
{

	static const double TIME_DIMENSION = 1e7;
	static const size_t MAX_SIGNALS = 512;

	enum class DataFormat
	{
		Edf,
		EdfPlusC,
		EdfPlusD,
		Bdf,
		BdfPlusC,
		BdfPlusD,
		Invalid
	};

	static const bool IsPlus(DataFormat format)
	{
		return format == DataFormat::EdfPlusC || format == DataFormat::EdfPlusD
			|| format == DataFormat::BdfPlusC || format == DataFormat::BdfPlusD;
	}

	static const bool IsEdf(DataFormat format)
	{
		return format == DataFormat::Edf || format == DataFormat::EdfPlusC || format == DataFormat::EdfPlusD;
	}

	static const bool IsBdf(DataFormat format)
	{
		return format == DataFormat::Bdf || format == DataFormat::BdfPlusC || format == DataFormat::BdfPlusD;
	}

	enum SampleSize
	{
		EdfSize = 2, // 16 bits
		BdfSize = 3	 // 24 bits
	};

	enum class FileErrc 
	{
		FileDoesNotOpen,
		FileNotOpened,
		FileReadError,
		FileContainsFormatErrors
	};

	static const char* GetError(FileErrc err)
	{
		if (err == FileErrc::FileDoesNotOpen) 
		{
			return "Error: file does not open";
		}
		else if (err == FileErrc::FileNotOpened)
		{
			return "Error: file not opened";
		}
		else if (err == FileErrc::FileReadError)
		{
			return "Error: can't read file";
		}
		else if (err == FileErrc::FileContainsFormatErrors)
		{
			return "Error: file contains format errors";
		}
		return "Unspecified error";
	}

	enum class SeekType 
	{
		Set,
		Cur,
		End
	};

}
