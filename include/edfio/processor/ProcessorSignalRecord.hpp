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
#include "../core/DataFormat.hpp"
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

	namespace impl
	{

		template <SampleType SampleT>
		struct SampleVec
		{
		};

		template <>
		struct SampleVec<SampleType::Physical>
		{
			using type = double;
			using vec = std::vector<double>;
		};

		template <>
		struct SampleVec<SampleType::Digital>
		{
			using type = int;
			using vec = std::vector<int>;
		};

	}

	template <SampleType SampleT>
	struct ProcessorSignalRecord : ProcessorBase<RecordField, typename impl::SampleVec<SampleT>::vec>
	{
		ProcessorSignalRecord(const HeaderSignal &signal, DataFormat format)
			: m_signal(signal)
			, m_sampleSize(edfio::GetSampleBytes(format)) {}

		TypeO operator << (TypeI in);
		TypeI operator >> (TypeO in);

	private:
		using DigiType = impl::SampleVec<SampleType::Digital>::type;
		using PhysType = impl::SampleVec<SampleType::Physical>::type;
		using ThisType = typename impl::SampleVec<SampleT>::type;

		ThisType CalcSample(DigiType sample);
		ThisType CalcSample(PhysType sample);

		const HeaderSignal &m_signal;
		const int m_sampleSize;
	};

}

#include "impl/ProcessorSignalRecord.ipp"
