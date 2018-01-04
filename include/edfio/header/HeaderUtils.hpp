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
#include "HeaderGeneral.hpp"
#include "HeaderSignal.hpp"

#include <vector>
#include <string>

namespace edfio
{

	namespace detail
	{

		static HeaderGeneral CreateHeaderGeneral(
			DataFormat version,
			std::string patient,
			std::string recording,
			int startDateD,
			int startDateM,
			int startDateY,
			int startTimeH,
			int startTimeM,
			int startTimeS,
			int headerSize,
			std::string reserved,
			long long datarecordsFile,
			double datarecordDuration,
			int totalSignals
		)
		{
			HeaderGeneral header;
			header.m_version = version;
			header.m_patient = patient;
			header.m_recording = recording;
			header.m_startDate = std::make_tuple(startDateD, startDateM, startDateY);
			header.m_startTime = std::make_tuple(startTimeH, startTimeM, startTimeS);
			header.m_headerSize = headerSize;
			header.m_reserved = reserved;
			header.m_datarecordsFile = datarecordsFile;
			header.m_datarecordDuration = datarecordDuration;
			header.m_totalSignals = totalSignals;

			return std::move(header);
		}

		static HeaderGeneral CreateHeaderGeneralPlus(
			DataFormat version,
			std::string patientCode,
			std::string gender,
			std::string birthdate,
			std::string patientName,
			std::string patientAdditional,
			std::string admincode,
			std::string technician,
			std::string equipment,
			std::string recordingAdditional,
			int startDateD,
			int startDateM,
			int startDateY,
			int startTimeH,
			int startTimeM,
			int startTimeS,
			int headerSize,
			std::string reserved,
			long long datarecordsFile,
			double datarecordDuration,
			int totalSignals
		)
		{
			auto header = std::move(CreateHeaderGeneral(
				version, "", "", startDateD, startDateM, startDateY, startTimeH, startTimeM, startTimeS,
				headerSize, reserved, datarecordsFile, datarecordDuration, totalSignals
			));

			header.m_detail.m_patientCode = patientCode;
			header.m_detail.m_gender = gender;
			header.m_detail.m_birthdate = birthdate;
			header.m_detail.m_patientName = patientName;
			header.m_detail.m_patientAdditional = patientAdditional;
			header.m_detail.m_admincode = admincode;
			header.m_detail.m_technician = technician;
			header.m_detail.m_equipment = equipment;
			header.m_detail.m_recordingAdditional = recordingAdditional;

			return std::move(header);
		}

		static HeaderSignal CreateHeaderSignal(
			std::string label,
			int samplesInDataRecord,
			double physicalMin,
			double physicalMax,
			int digitalMin,
			int digitalMax,
			long bufferOffset = 0,
			bool annotation = false,
			std::string transducer = "",
			std::string physDimension = "",
			std::string prefilter = "",
			std::string reserved = ""
		)
		{
			HeaderSignal signal;

			signal.m_label = label;
			signal.m_transducer = transducer;
			signal.m_physDimension = physDimension;
			signal.m_physicalMin = physicalMin;
			signal.m_physicalMax = physicalMax;
			signal.m_digitalMin = digitalMin;
			signal.m_digitalMax = digitalMax;
			signal.m_prefilter = prefilter;
			signal.m_samplesInDataRecord = samplesInDataRecord;
			signal.m_reserved = reserved;

			signal.m_detail.m_bufferOffset = bufferOffset;
			signal.m_detail.m_scaling = (signal.m_physicalMax - signal.m_physicalMin) / (signal.m_digitalMax - signal.m_digitalMin);
			signal.m_detail.m_offset = signal.m_physicalMin - signal.m_detail.m_scaling * signal.m_digitalMin;
			signal.m_detail.m_isAnnotation = annotation;

			return std::move(signal);
		}

	}

}
