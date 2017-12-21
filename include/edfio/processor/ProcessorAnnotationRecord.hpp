//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../Defs.hpp"
#include "ProcessorBase.hpp"
#include "../core/Record.hpp"
#include "../core/Annotation.hpp"

#include <string>

namespace edfio
{

	struct ProcessorAnnotationRecord : ProcessorBase<RecordField, std::vector<Annotation>>
	{
		ProcessorAnnotationRecord() = default;
		ProcessorAnnotationRecord(size_t recordSize) 
			: m_recordSize(recordSize)
		{
		}

		TypeO operator << (TypeI in);
		TypeI operator >> (TypeO in);
	private:
		size_t m_recordSize = 0;
	};

}

#include "impl/ProcessorAnnotationRecord.ipp"
