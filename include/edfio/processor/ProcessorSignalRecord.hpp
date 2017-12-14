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
#include "../header/HeaderSignal.hpp"

#include <vector>
#include <type_traits>

namespace edfio
{
	enum class SampleType
	{
		Physical,
		Digital
	};

	template <SampleType SampleT>
	struct SampleVec
	{
		typedef void type;
	};

	template <>
	struct SampleVec<SampleType::Physical>
	{
		typedef std::vector<double> type;
	};

	template <>
	struct SampleVec<SampleType::Digital>
	{
		typedef std::vector<int> type;
	};

	template <SampleType SampleT>
	struct ProcessorSignalRecord : ProcessorBase<RecordField, SampleVec<SampleT>::type>
	{
		ProcessorSignalRecord(const HeaderSignal &signal, int sampleSize)
			: m_signal(signal)
			, m_sampleSize(sampleSize) {}

		TypeOu operator ()(TypeIn in);

	private:
		const HeaderSignal &m_signal;
		const int m_sampleSize;
	};

}

#include "impl/ProcessorSignalRecord.ipp"
