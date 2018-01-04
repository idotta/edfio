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
		return false;
		/*return format == DataFormat::EdfPlusC || format == DataFormat::EdfPlusD
			|| format == DataFormat::BdfPlusC || format == DataFormat::BdfPlusD;*/
	}

	static const bool IsEdf(DataFormat format)
	{
		return format == DataFormat::Edf || format == DataFormat::EdfPlusC || format == DataFormat::EdfPlusD;
	}

	static const bool IsBdf(DataFormat format)
	{
		return format == DataFormat::Bdf || format == DataFormat::BdfPlusC || format == DataFormat::BdfPlusD;
	}

	static const int GetSampleBytes(DataFormat format)
	{
		if (IsEdf(format))
			return 2;
		else if (IsBdf(format))
			return 3;
		return -1;
	}

}
