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

	ProcessorHeaderGeneral::TypeO ProcessorHeaderGeneral::operator << (TypeI in)
	{
		TypeO ou;

		if (detail::CheckFormatErrors(in.m_version())
			|| detail::CheckFormatErrors(in.m_patient())
			|| detail::CheckFormatErrors(in.m_recording())
			|| detail::CheckFormatErrors(in.m_startDate())
			|| detail::CheckFormatErrors(in.m_startTime())
			|| detail::CheckFormatErrors(in.m_headerSize())
			|| detail::CheckFormatErrors(in.m_reserved())
			|| detail::CheckFormatErrors(in.m_datarecordsFile())
			|| detail::CheckFormatErrors(in.m_datarecordDuration())
			|| detail::CheckFormatErrors(in.m_totalSignals()))
		{
			throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
		}


		// Version
		{
			auto &version = in.m_version();
			if (version.front() == -1)   // BDF-file
			{
				if (version.substr(1) != "BIOSEMI")
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
				ou.m_version = DataFormat::Bdf;
			}
			else    // EDF-file
			{
				if (version != "0       ")
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
				ou.m_version = DataFormat::Edf;
			}
		}
		// Patient Name
		{
			ou.m_patient = in.m_patient();
		}
		// Recording
		{
			ou.m_recording = in.m_recording();
		}
		// Start Date
		{
			auto& startdate = in.m_startDate();
			if ((startdate[2] != '.') || (startdate[5] != '.')
				|| !std::isdigit(startdate[0])
				|| !std::isdigit(startdate[1])
				|| !std::isdigit(startdate[3])
				|| !std::isdigit(startdate[4])
				|| !std::isdigit(startdate[6])
				|| !std::isdigit(startdate[7]))
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			try
			{
				int day = std::stoi(startdate);
				int month = std::stoi(startdate.substr(3));
				int year = std::stoi(startdate.substr(6));

				if (day < 1 || day > 31 || month < 1 || month > 12)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
				year += year > 84 ? 1900 : 2000;

				ou.m_startDate = std::make_tuple(day, month, year);
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}
		// Start Time
		{
			auto& starttime = in.m_startTime();
			if ((starttime[2] != '.') || (starttime[5] != '.')
				|| !std::isdigit(starttime[0])
				|| !std::isdigit(starttime[1])
				|| !std::isdigit(starttime[3])
				|| !std::isdigit(starttime[4])
				|| !std::isdigit(starttime[6])
				|| !std::isdigit(starttime[7]))
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			try
			{
				int hour = std::stoi(starttime);
				int minute = std::stoi(starttime.substr(3));
				int second = std::stoi(starttime.substr(6));

				if (hour > 23 || minute > 59 || second > 59)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
				ou.m_startTime = std::make_tuple(hour, minute, second);
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}
		// Header Size
		{
			try
			{
				ou.m_headerSize = std::stoi(in.m_headerSize());
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}
		// Reserved
		{
			auto &reserved = in.m_reserved();
			if (IsEdf(ou.m_version))
			{
				if (reserved.find("EDF+C") != std::string::npos)
				{
					ou.m_version = DataFormat::EdfPlusC;
				}
				else if (reserved.find("EDF+D") != std::string::npos)
				{
					ou.m_version = DataFormat::EdfPlusD;
				}
			}
			else if (IsBdf(ou.m_version))
			{
				if (reserved.find("BDF+C") != std::string::npos)
				{
					ou.m_version = DataFormat::BdfPlusC;
				}
				else if (reserved.find("BDF+D") != std::string::npos)
				{
					ou.m_version = DataFormat::BdfPlusD;
				}
			}
		}
		// Datarecords in File
		{
			try
			{
				ou.m_datarecordsFile = std::stoll(in.m_datarecordsFile());
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			if (ou.m_datarecordsFile < 1)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}
		// Datarecord Duration
		{
			double duration = -1;
			try
			{
				duration = std::stod(in.m_datarecordDuration());
				ou.m_datarecordDuration = static_cast<long long>(std::fabs(duration) * static_cast<double>(TIME_DIMENSION));
				ou.m_detail.m_datarecordDuration = duration;
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			if (duration < -0.000001)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			ou.m_detail.m_fileDuration = ou.m_datarecordDuration * ou.m_datarecordsFile;
		}
		// Number of signals
		{
			int signals = 0;
			try
			{
				signals = std::stoi(in.m_totalSignals());
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			if (signals < 1 || signals > MAX_SIGNALS
				|| (signals * 256 + 256) != ou.m_headerSize)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			ou.m_totalSignals = signals;
		}
		// Plus Fields
		if (IsPlus(ou.m_version))
		{
			// Patient Name
			{
				std::vector<std::string> fields;
				std::string field;
				std::istringstream f(ou.m_patient);
				while (std::getline(f, field, ' '))
				{
					fields.push_back(field);
				}
				if (fields.size() < 4)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}

				ou.m_detail.m_patientAdditional.clear();

				for (size_t i = 0; i < fields.size(); i++)
				{
					auto& str = fields[i];
					std::replace(str.begin(), str.end(), '_', ' '); // replace all '_' to ' '
					switch (i)
					{
					case 0: // The code by which the patient is known in the hospital administration.
						ou.m_detail.m_patientCode = str;
						break;
					case 1: // Sex
						if (str == "F" || str == "M" || str == "X")
						{
							ou.m_detail.m_gender = str;
						}
						else
						{
							throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
						}
						break;
					case 2: // Birthdate in dd-MMM-yyyy format using the English 3-character abbreviations of the month in capitals. 02-AUG-1951 is OK, while 2-AUG-1951 is not.
						ou.m_detail.m_birthdate = str;
						break;
					case 3: // The patients name.
						ou.m_detail.m_patientName = str;
						break;
					default: // Additional information.
						if (!ou.m_detail.m_patientAdditional.empty())
						{
							ou.m_detail.m_patientAdditional += detail::ADDITIONAL_SEPARATOR;
						}
						ou.m_detail.m_patientAdditional.append(str);
						break;
					}
				}
			}
			// Recording
			{
				std::vector<std::string> fields;
				std::string field;
				std::istringstream f(ou.m_recording);
				while (std::getline(f, field, ' '))
				{
					fields.push_back(field);
				}

				if (fields.size() < 5)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}

				ou.m_detail.m_recordingAdditional.clear();

				for (size_t i = 0; i < fields.size(); i++)
				{
					auto& str = fields[i];
					std::replace(str.begin(), str.end(), '_', ' '); // replace all '_' to ' '
					if (str != "X")
					{
						switch (i)
						{
						case 0: // The text 'Startdate'.
							if (str != "Startdate")
							{
								throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
							}
							break;
						case 1: // The startdate itself in dd-MMM-yyyy format using the English 3-character abbreviations of the month in capitals: dd-MMM-yyyy (MMM = 'JAN' | 'FEV' | ...)
							if (str.size() == 11 && str[2] == '-' && str[6] == '-')
							{
								try
								{
									int day = std::stoi(str.substr(0, 2));
									int year = std::stoi(str.substr(7, 4));
									int month = detail::GetMonthFromString(str.substr(3, 3));
									if (month == 0)
									{
										throw std::exception("Invalid Month");
									}
									ou.m_startDate = std::make_tuple(day, month, year);
								}
								catch (...)
								{
									throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
								}
							}
							else
							{
								throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
							}
							break;
						case 2: // The hospital administration code of the investigation, i.e. EEG number or PSG number.
							ou.m_detail.m_admincode = str;
							break;
						case 3: // A code specifying the responsible investigator or technician.
							ou.m_detail.m_technician = str;
							break;
						case 4: // A code specifying the used equipment.
							ou.m_detail.m_equipment = str;
							break;
						default: // Additional information.
							if (!ou.m_detail.m_recordingAdditional.empty())
							{
								ou.m_detail.m_recordingAdditional += detail::ADDITIONAL_SEPARATOR;
							}
							ou.m_detail.m_recordingAdditional.append(str);
							break;
						}
					}
				}
			}
		}

		ou.m_patient = detail::ReduceString(ou.m_patient);
		ou.m_recording = detail::ReduceString(ou.m_recording);
		ou.m_reserved = detail::ReduceString(ou.m_reserved);
		ou.m_detail.m_patientCode = detail::ReduceString(ou.m_detail.m_patientCode);
		ou.m_detail.m_gender = detail::ReduceString(ou.m_detail.m_gender);
		ou.m_detail.m_birthdate = detail::ReduceString(ou.m_detail.m_birthdate);
		ou.m_detail.m_patientName = detail::ReduceString(ou.m_detail.m_patientName);
		ou.m_detail.m_patientAdditional = detail::ReduceString(ou.m_detail.m_patientAdditional);
		ou.m_detail.m_admincode = detail::ReduceString(ou.m_detail.m_admincode);
		ou.m_detail.m_technician = detail::ReduceString(ou.m_detail.m_technician);
		ou.m_detail.m_equipment = detail::ReduceString(ou.m_detail.m_equipment);
		ou.m_detail.m_recordingAdditional = detail::ReduceString(ou.m_detail.m_recordingAdditional);

		return std::move(ou);
	}

	ProcessorHeaderGeneral::TypeI ProcessorHeaderGeneral::operator >> (TypeO in)
	{
		TypeI ou;

		// Version
		{
			if (IsEdf(in.m_version))
			{
				ou.m_version() = "0       ";
			}
			else if (IsBdf(in.m_version))
			{
				ou.m_version() = " BIOSEMI";
			}
		}
		// Patient Name
		{
			ou.m_patient() = in.m_patient;
		}
		// Recording
		{
			ou.m_recording() = in.m_recording;
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
			ou.m_startDate() = oss.str();
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
			ou.m_startTime() = oss.str();
		}
		// Header Size
		{
			ou.m_headerSize() = std::to_string(in.m_headerSize);
		}
		// Reserved
		{
			if (!in.m_reserved.empty())
			{
				ou.m_reserved() = in.m_reserved;
			}
			else if (IsPlus(in.m_version))
			{
				ou.m_reserved() = detail::GetFormatName(in.m_version);
			}
		}
		// Datarecords in File
		{
			ou.m_datarecordsFile() = std::to_string(in.m_datarecordsFile);
		}
		// Datarecord Duration
		{
			ou.m_datarecordDuration() = detail::to_string_decimal(in.m_detail.m_datarecordDuration);
		}
		// Number of signals
		{
			ou.m_totalSignals() = std::to_string(in.m_totalSignals);
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
				ou.m_patient() = patient;
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
				ou.m_recording() = recording;
			}
		}
		return std::move(ou);
	}
}
