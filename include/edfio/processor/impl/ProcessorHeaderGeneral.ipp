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
#include "../../core/DataFormat.hpp"
#include "../detail/ProcessorUtils.hpp"

#include <sstream>
#include <iomanip>

namespace edfio
{

	inline HeaderGeneralFields ProcessorHeaderGeneral::operator()(HeaderGeneral in)
	{
		HeaderGeneralFields out;

		// Version
		{
			if (IsEdf(in.m_version))
			{
				out.m_version("0       ");
			}
			else if (IsBdf(in.m_version))
			{
				out.m_version(" BIOSEMI");
			}
		}
		// Patient Name
		{
			out.m_patient(in.m_patient);
		}
		// Recording
		{
			out.m_recording(in.m_recording);
		}
		// Start Date
		{
			int day = std::get<0>(in.m_startDate);
			int month = std::get<1>(in.m_startDate);
			int year = std::get<2>(in.m_startDate);
			year -= year > 1999 ? 2000 : 1900;
			std::ostringstream oss;
			oss << std::setw(2) << std::setfill('0') << day << ".";
			oss << std::setw(2) << std::setfill('0') << month << ".";
			oss << std::setw(2) << std::setfill('0') << year;
			out.m_startDate(oss.str());
		}
		// Start Time
		{
			int hour = std::get<0>(in.m_startTime);
			int minute = std::get<1>(in.m_startTime);
			int second = std::get<2>(in.m_startTime);
			std::ostringstream oss;
			oss << std::setw(2) << std::setfill('0') << hour << ".";
			oss << std::setw(2) << std::setfill('0') << minute << ".";
			oss << std::setw(2) << std::setfill('0') << second;
			out.m_startTime(oss.str());
		}
		// Header Size
		{
			out.m_headerSize(std::to_string(in.m_headerSize));
		}
		// Reserved
		{
			if (!in.m_reserved.empty())
			{
				out.m_reserved(in.m_reserved);
			}
			else if (IsPlus(in.m_version))
			{
				out.m_reserved(detail::GetFormatName(in.m_version));
			}
		}
		// Datarecords in File
		{
			out.m_datarecordsFile(std::to_string(in.m_datarecordsFile));
		}
		// Datarecord Duration
		{
			out.m_datarecordDuration(detail::to_string_decimal(in.m_datarecordDuration));
		}
		// Number of signals
		{
			out.m_totalSignals(std::to_string(in.m_totalSignals));
		}
		// Plus Fields
		if (IsPlus(in.m_version))
		{
			// Patient Name
			{
				std::vector<std::string> fields;
				// Code
				fields.push_back(in.m_detail.m_patientCode.empty() ? "X" : in.m_detail.m_patientCode);
				// Sex
				fields.push_back(in.m_detail.m_gender.empty() ? "X" : in.m_detail.m_gender);
				// Birthdate in dd-MMM-yyyy format using the English 3-character abbreviations of the month in capitals. 02-AUG-1951 is OK, while 2-AUG-1951 is not.
				fields.push_back(in.m_detail.m_birthdate.empty() ? "X" : in.m_detail.m_birthdate);
				// The patients name.
				fields.push_back(in.m_detail.m_patientName.empty() ? "X" : in.m_detail.m_patientName);
				// Additional information.
				std::string info;
				std::istringstream f(in.m_detail.m_patientAdditional);
				while (std::getline(f, info, detail::ADDITIONAL_SEPARATOR))
				{
					fields.push_back(info);
				}
				std::string patient = "";
				for (auto& field : fields)
				{
					std::replace(field.begin(), field.end(), ' ', '_'); // replace all ' ' to '_'
					patient += field + " ";
				}
				patient.pop_back(); // remove last " "
				out.m_patient(patient);
			}
			// Recording
			{
				std::vector<std::string> fields;
				// The text 'Startdate'
				fields.push_back("Startdate");
				// The startdate itself in dd-MMM-yyyy format using the English 3-character abbreviations of the month in capitals: dd-MMM-yyyy (MMM = 'JAN' | 'FEV' | ...)
				std::ostringstream oss;
				int day = std::get<0>(in.m_startDate);
				int month = std::get<1>(in.m_startDate);
				int year = std::get<2>(in.m_startDate);
				oss << std::setw(2) << std::setfill('0') << (day ? day : 1) << "-" << detail::GetStringFromMonth(month) << "-" << (year ? year : 1984);
				fields.push_back(oss.str());
				// The hospital administration code of the investigation, i.e. EEG number or PSG number.
				fields.push_back(in.m_detail.m_admincode.empty() ? "X" : in.m_detail.m_admincode);
				// A code specifying the responsible investigator or technician.
				fields.push_back(in.m_detail.m_technician.empty() ? "X" : in.m_detail.m_technician);
				// A code specifying the used equipment.
				fields.push_back(in.m_detail.m_equipment.empty() ? "X" : in.m_detail.m_equipment);
				// Additional information.
				std::string info;
				std::istringstream f(in.m_detail.m_recordingAdditional);
				while (std::getline(f, info, detail::ADDITIONAL_SEPARATOR))
				{
					fields.push_back(info);
				}
				std::string recording = "";
				for (auto& field : fields)
				{
					std::replace(field.begin(), field.end(), ' ', '_'); // replace all '_' to ' '
					recording += field + " ";
				}
				recording.pop_back(); // remove last " "
				out.m_recording(recording);
			}
		}
		return std::move(out);
	}
}
