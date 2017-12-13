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
#include "../../header/HeaderExam.hpp"

#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace edfio
{

	FileErrc ProcessorHeaderExam::operator()(const HeaderExamFields &in, HeaderExam&ou)
	{
		if (CheckFormatErrors(in.m_version())
			|| CheckFormatErrors(in.m_patient())
			|| CheckFormatErrors(in.m_recording())
			|| CheckFormatErrors(in.m_startDate())
			|| CheckFormatErrors(in.m_startTime())
			|| CheckFormatErrors(in.m_headerSize())
			|| CheckFormatErrors(in.m_reserved())
			|| CheckFormatErrors(in.m_datarecordsFile())
			|| CheckFormatErrors(in.m_datarecordDuration())
			|| CheckFormatErrors(in.m_totalSignals()))
		{
			return FileErrc::FileContainsFormatErrors;
		}


		// Version
		{
			auto &version = in.m_version();
			if (version.front() == -1)   // BDF-file
			{
				if (version.substr(1) != "BIOSEMI")
				{
					return FileErrc::FileContainsFormatErrors;
				}
				ou.m_version = DataFormat::Bdf;
			}
			else    // EDF-file
			{
				if (version != "0       ")
				{
					return FileErrc::FileContainsFormatErrors;
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
				return FileErrc::FileContainsFormatErrors;
			}
			try
			{
				int day = std::stoi(startdate);
				int month = std::stoi(startdate.substr(3));
				int year = std::stoi(startdate.substr(6));

				if (day < 1 || day > 31 || month < 1 || month > 12)
				{
					return FileErrc::FileContainsFormatErrors;
				}
				year += year > 84 ? 1900 : 2000;

				ou.m_startDate = std::make_tuple(day, month, year);
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
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
				return FileErrc::FileContainsFormatErrors;
			}
			try
			{
				int hour = std::stoi(starttime);
				int minute = std::stoi(starttime.substr(3));
				int second = std::stoi(starttime.substr(6));

				if (hour > 23 || minute > 59 || second > 59)
				{
					return FileErrc::FileContainsFormatErrors;
				}
				ou.m_startTime = std::make_tuple(hour, minute, second);
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
		}
		// Header Size
		{
			try
			{
				ou.m_headerSize = std::stoi(in.m_headerSize());
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
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
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (ou.m_datarecordsFile < 1)
			{
				return FileErrc::FileContainsFormatErrors;
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
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (duration < -0.000001)
			{
				return FileErrc::FileContainsFormatErrors;
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
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (signals < 1 || signals > MAX_SIGNALS
				|| (signals * 256 + 256) != ou.m_headerSize)
			{
				return FileErrc::FileContainsFormatErrors;
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
					return FileErrc::FileContainsFormatErrors;
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
							return FileErrc::FileContainsFormatErrors;
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
							ou.m_detail.m_patientAdditional += ADDITIONAL_SEPARATOR;
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
					return FileErrc::FileContainsFormatErrors;
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
								return FileErrc::FileContainsFormatErrors;
							}
							break;
						case 1: // The startdate itself in dd-MMM-yyyy format using the English 3-character abbreviations of the month in capitals: dd-MMM-yyyy (MMM = 'JAN' | 'FEV' | ...)
							if (str.size() == 11 && str[2] == '-' && str[6] == '-')
							{
								try
								{
									int day = std::stoi(str.substr(0, 2));
									int year = std::stoi(str.substr(7, 4));
									int month = GetMonthFromString(str.substr(3, 3));
									if (month == 0)
									{
										throw std::exception("Invalid Month");
									}
									ou.m_startDate = std::make_tuple(day, month, year);
								}
								catch (std::exception& e)
								{
									return FileErrc::FileContainsFormatErrors;
								}
							}
							else
							{
								return FileErrc::FileContainsFormatErrors;
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
								ou.m_detail.m_recordingAdditional += ADDITIONAL_SEPARATOR;
							}
							ou.m_detail.m_recordingAdditional.append(str);
							break;
						}
					}
				}
			}
		}
		//// File size
		//{
		//	// get length of file:
		//	is.seekg(0, is.end);
		//	size_t length = is.tellg();
		//	is.seekg(0, is.beg);

		//	if (length != (header.m_detail.m_recordSize * header.m_datarecordsFile + header.m_detail.m_headerSize))
		//	{
		//		FileErrc::FileContainsFormatErrors;
		//	}
		//}

		ou.m_patient = ReduceString(ou.m_patient);
		ou.m_recording = ReduceString(ou.m_recording);
		ou.m_reserved = ReduceString(ou.m_reserved);
		ou.m_detail.m_patientCode = ReduceString(ou.m_detail.m_patientCode);
		ou.m_detail.m_gender = ReduceString(ou.m_detail.m_gender);
		ou.m_detail.m_birthdate = ReduceString(ou.m_detail.m_birthdate);
		ou.m_detail.m_patientName = ReduceString(ou.m_detail.m_patientName);
		ou.m_detail.m_patientAdditional = ReduceString(ou.m_detail.m_patientAdditional);
		ou.m_detail.m_admincode = ReduceString(ou.m_detail.m_admincode);
		ou.m_detail.m_technician = ReduceString(ou.m_detail.m_technician);
		ou.m_detail.m_equipment = ReduceString(ou.m_detail.m_equipment);
		ou.m_detail.m_recordingAdditional = ReduceString(ou.m_detail.m_recordingAdditional);

		return FileErrc::NoError;
	}

}
