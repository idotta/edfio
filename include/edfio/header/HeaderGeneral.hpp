//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../core/DataFormat.hpp"
#include "../core/Field.hpp"

#include <string>
#include <tuple>

namespace edfio
{

	struct HeaderGeneralFields
	{
		Field<8> m_version;
		Field<80> m_patient;
		Field<80> m_recording;
		Field<8> m_startDate;
		Field<8> m_startTime;
		Field<8> m_headerSize;
		Field<44> m_reserved;
		Field<8> m_datarecordsFile;
		Field<8> m_datarecordDuration;
		Field<4> m_totalSignals;
	};

	namespace detail
	{
		struct HeaderGeneralDetail
		{
			unsigned int m_recordSize = 0;
			double m_fileDuration = 0;

			std::string m_patientCode;
			std::string m_gender;
			std::string m_birthdate;
			std::string m_patientName;
			std::string m_patientAdditional;
			std::string m_admincode;
			std::string m_technician;
			std::string m_equipment;
			std::string m_recordingAdditional;
		};
	}

	struct HeaderGeneral
	{
		// Field values
		DataFormat m_version = DataFormat::Invalid;
		std::string m_patient;
		std::string m_recording;
		std::tuple<int, int, int> m_startDate;
		std::tuple<int, int, int> m_startTime;
		int m_headerSize = 0;
		std::string m_reserved;
		long long m_datarecordsFile = 0;
		double m_datarecordDuration = 0;
		int m_totalSignals = 0;
		// Extra values
		detail::HeaderGeneralDetail m_detail;
	};

}
