//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "Defs.hpp"

#include <string>
#include <tuple>

namespace edfio
{

	namespace detail
	{
		struct ExamHeaderDetail
		{
			unsigned int m_recordSize = 0;
			double m_datarecordDuration = 0;
			long long m_fileDuration = 0;
			long long m_startSubsecond = 0;
			long long m_annotationsInFile = 0;

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

	struct ExamHeader
	{
		Field<DataFormat, 8> m_version;
		Field<std::string, 80> m_patient;
		Field<std::string, 80> m_recording;
		Field<std::tuple<int, int, int>, 8> m_startDate;
		Field<std::tuple<int, int, int>, 8> m_startTime;
		Field<int, 8> m_headerSize;
		Field<std::string, 44> m_reserved;
		Field<long long, 8> m_datarecordsFile;
		Field<long long, 8> m_datarecordDuration;
		Field<int, 4> m_totalSignals;
		detail::ExamHeaderDetail m_detail;
	};

}
