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

	ProcessorDataRecord::TypeOu ProcessorDataRecord::operator()(TypeIn in)
	{
		auto datarecord = std::move(in());

		TypeOu fields;
		fields.reserve(m_signals.size());

		auto it = datarecord.begin();

		for (auto& sig : m_signals)
		{
			size_t fieldSize = sig.m_samplesInDataRecord * m_sampleSize;
			RecordField field(it, it + fieldSize);
			fields.emplace_back(std::move(field));
			it += fieldSize;
		}

		return fields;
	}

}

