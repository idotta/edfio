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
#include "../ExamHeader.hpp"
#include "../SignalHeader.hpp"

#include <vector>

namespace edfio
{

	FileErrc ExamHeaderReader::operator ()(ExamHeader &hdr)
	{
		if (!m_is || !m_is.is_open())
			return FileErrc::FileDoesNotOpen;
		m_is.clear();
		m_is.seekg(0, std::ios::beg);

		try
		{
			m_is >> hdr.m_version;
			m_is >> hdr.m_patient;
			m_is >> hdr.m_recording;
			m_is >> hdr.m_startDate;
			m_is >> hdr.m_startTime;
			m_is >> hdr.m_headerSize;
			m_is >> hdr.m_reserved;
			m_is >> hdr.m_datarecordsFile;
			m_is >> hdr.m_datarecordDuration;
			m_is >> hdr.m_totalSignals;
		}
		catch (std::exception e)
		{
			return FileErrc::FileReadError;
		}
		return edfio::FileErrc::NoError;
	}

	FileErrc SignalHeaderReader::operator ()(std::vector<SignalHeader> &signals)
	{
		if (!m_is || !m_is.is_open())
			return FileErrc::FileDoesNotOpen;
		m_is.clear();
		m_is.seekg(256, std::ios::beg);

		try
		{
			for (auto &s : signals)
				m_is >> s.m_label;
			for (auto &s : signals)
				m_is >> s.m_transducer;
			for (auto &s : signals)
				m_is >> s.m_physDimension;
			for (auto &s : signals)
				m_is >> s.m_physicalMin;
			for (auto &s : signals)
				m_is >> s.m_physicalMax;
			for (auto &s : signals)
				m_is >> s.m_digitalMin;
			for (auto &s : signals)
				m_is >> s.m_digitalMax;
			for (auto &s : signals)
				m_is >> s.m_prefilter;
			for (auto &s : signals)
				m_is >> s.m_samplesInDataRecord;
			for (auto &s : signals)
				m_is >> s.m_reserved;
		}
		catch (std::exception e)
		{
			return FileErrc::FileReadError;
		}
		return edfio::FileErrc::NoError;
	}

}
