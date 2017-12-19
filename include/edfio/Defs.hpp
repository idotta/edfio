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

	enum class FileErrc
	{
		FileDoesNotOpen,
		FileNotOpened,
		FileReadError,
		FileContainsFormatErrors,
		FileContainsInvalidAnnotations,
		FileWriteError
	};

	namespace detail
	{
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
			else if (err == FileErrc::FileContainsInvalidAnnotations)
			{
				return "Error: file contains invalid annotations";
			}
			else if (err == FileErrc::FileWriteError)
			{
				return "Error: can't write on file";
			}
			return "Unspecified error";
		}

	}

}
