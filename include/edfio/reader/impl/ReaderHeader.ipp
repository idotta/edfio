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
#include "../../header/HeaderGeneral.hpp"
#include "../../header/HeaderSignal.hpp"

#include <vector>

namespace edfio
{

	FileErrc ReaderHeaderGeneral::operator ()(HeaderGeneralFields &hdr)
	{
		if (!m_stream || !m_stream.is_open())
			return FileErrc::FileDoesNotOpen;
		m_stream.clear();
		m_stream.seekg(0, std::ios::beg);

		try
		{
			m_stream >> hdr.m_version;
			m_stream >> hdr.m_patient;
			m_stream >> hdr.m_recording;
			m_stream >> hdr.m_startDate;
			m_stream >> hdr.m_startTime;
			m_stream >> hdr.m_headerSize;
			m_stream >> hdr.m_reserved;
			m_stream >> hdr.m_datarecordsFile;
			m_stream >> hdr.m_datarecordDuration;
			m_stream >> hdr.m_totalSignals;
		}
		catch (std::exception e)
		{
			return FileErrc::FileReadError;
		}
		return FileErrc::NoError;
	}

	FileErrc ReaderHeaderSignal::operator ()(std::vector<HeaderSignalFields> &signals)
	{
		if (!m_stream || !m_stream.is_open())
			return FileErrc::FileDoesNotOpen;
		m_stream.clear();
		m_stream.seekg(256, std::ios::beg);

		try
		{
			for (auto &s : signals)
				m_stream >> s.m_label;
			for (auto &s : signals)
				m_stream >> s.m_transducer;
			for (auto &s : signals)
				m_stream >> s.m_physDimension;
			for (auto &s : signals)
				m_stream >> s.m_physicalMin;
			for (auto &s : signals)
				m_stream >> s.m_physicalMax;
			for (auto &s : signals)
				m_stream >> s.m_digitalMin;
			for (auto &s : signals)
				m_stream >> s.m_digitalMax;
			for (auto &s : signals)
				m_stream >> s.m_prefilter;
			for (auto &s : signals)
				m_stream >> s.m_samplesInDataRecord;
			for (auto &s : signals)
				m_stream >> s.m_reserved;
		}
		catch (std::exception e)
		{
			return FileErrc::FileReadError;
		}
		return edfio::FileErrc::NoError;
	}

}
