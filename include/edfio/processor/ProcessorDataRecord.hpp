//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "ProcessorBase.hpp"
#include "../core/DataFormat.hpp"
#include "../core/Record.hpp"
#include "../header/HeaderSignal.hpp"

#include <vector>

namespace edfio
{

	struct ProcessorDataRecord : ProcessorBase<RecordField, std::vector<RecordField>>
	{
		ProcessorDataRecord(const std::vector<HeaderSignal> &signals, DataFormat format)
			: m_signals(signals)
			, m_sampleSize(edfio::GetSampleBytes(format)) {}

		TypeOu operator ()(TypeIn in);

	private:
		const std::vector<HeaderSignal> &m_signals;
		const int m_sampleSize;
	};

}

#include "impl/ProcessorDataRecord.ipp"
