//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../Defs.hpp"

namespace edfio
{

	FileErrc ProcessorHeaderExam::operator()(const TypeIn &in, TypeOu &ou)
	{
		// File size
		{
			// get length of file:
			m_is.seekg(0, m_is.end);
			size_t length = m_is.tellg();
			m_is.seekg(0, m_is.beg);

			if (length != (in.m_general.m_detail.m_recordSize * in.m_general.m_datarecordsFile + in.m_general.m_headerSize))
			{
				FileErrc::FileContainsFormatErrors;
			}
		}
		return FileErrc::NoError;
	}

}
