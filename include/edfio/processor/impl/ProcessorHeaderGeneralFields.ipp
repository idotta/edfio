//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../Utils.hpp"
#include "../../core/DataFormat.hpp"
#include "../detail/ProcessorUtils.hpp"

#include <sstream>
#include <iomanip>

namespace edfio
{

	ProcessorHeaderGeneralFields::Out ProcessorHeaderGeneralFields::operator()(In in)
	{
		Out out;

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
				out.m_version = DataFormat::Bdf;
			}
			else    // EDF-file
			{
				if (version != "0       ")
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
				out.m_version = DataFormat::Edf;
			}
		}
		// Patient Name
		{
			out.m_patient = in.m_patient();
		}
		// Recording
		{
			out.m_recording = in.m_recording();
		}
		// Start Date
		{
			auto& startdate = in.m_startDate();
			if ((startdate[2] != '.') || (startdate[5] != '.')
				|| !std::isdigit(startdate[0]) || !std::isdigit(startdate[1])
				|| !std::isdigit(startdate[3]) || !std::isdigit(startdate[4])
				|| !std::isdigit(startdate[6]) || !std::isdigit(startdate[7]))
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

				out.m_startDate = std::make_tuple(day, month, year);
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
				|| !std::isdigit(starttime[0]) || !std::isdigit(starttime[1])
				|| !std::isdigit(starttime[3]) || !std::isdigit(starttime[4])
				|| !std::isdigit(starttime[6]) || !std::isdigit(starttime[7]))
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
				out.m_startTime = std::make_tuple(hour, minute, second);
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
				out.m_headerSize = std::stoi(in.m_headerSize());
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}
		// Reserved
		{
			auto &reserved = in.m_reserved();
			if (IsEdf(out.m_version))
			{
				if (reserved.find("EDF+C") != std::string::npos)
				{
					out.m_version = DataFormat::EdfPlusC;
				}
				else if (reserved.find("EDF+D") != std::string::npos)
				{
					out.m_version = DataFormat::EdfPlusD;
				}
			}
			else if (IsBdf(out.m_version))
			{
				if (reserved.find("BDF+C") != std::string::npos)
				{
					out.m_version = DataFormat::BdfPlusC;
				}
				else if (reserved.find("BDF+D") != std::string::npos)
				{
					out.m_version = DataFormat::BdfPlusD;
				}
			}
		}
		// Datarecords in File
		{
			try
			{
				out.m_datarecordsFile = std::stoll(in.m_datarecordsFile());
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			if (out.m_datarecordsFile < 1)
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
				out.m_datarecordDuration = duration;
			}
			catch (...)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			if (duration < 0)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			out.m_detail.m_fileDuration = out.m_datarecordDuration * out.m_datarecordsFile;
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
			if (signals <= 0 || (signals * 256 + 256) != out.m_headerSize)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			out.m_totalSignals = signals;
		}
		// Plus Fields
		if (IsPlus(out.m_version))
		{
			// Patient Name
			{
				auto &details = out.m_detail;
				std::vector<std::string> fields;
				std::string field;
				std::istringstream f(out.m_patient);
				while (std::getline(f, field, ' '))
				{
					fields.push_back(field);
				}
				if (fields.size() < 4)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}

				details.m_patientAdditional.clear();

				for (size_t i = 0; i < fields.size(); i++)
				{
					auto& str = fields[i];
					std::replace(str.begin(), str.end(), '_', ' '); // replace all '_' to ' '
					switch (i)
					{
					case 0: // The code by which the patient is known in the hospital administration.
						details.m_patientCode = str;
						break;
					case 1: // Sex
						if (str == "F" || str == "M" || str == "X")
						{
							details.m_gender = str;
						}
						else
						{
							throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
						}
						break;
					case 2: // Birthdate in dd-MMM-yyyy format using the English 3-character abbreviations of the month in capitals. 02-AUG-1951 is OK, while 2-AUG-1951 is not.
						details.m_birthdate = str;
						break;
					case 3: // The patients name.
						details.m_patientName = str;
						break;
					default: // Additional information.
						if (!details.m_patientAdditional.empty())
						{
							details.m_patientAdditional += detail::ADDITIONAL_SEPARATOR;
						}
						details.m_patientAdditional.append(str);
						break;
					}
				}
			}
			// Recording
			{
				auto &details = out.m_detail;
				std::vector<std::string> fields;
				std::string field;
				std::istringstream f(out.m_recording);
				while (std::getline(f, field, ' '))
				{
					fields.push_back(field);
				}

				if (fields.size() < 5)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}

				details.m_recordingAdditional.clear();

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
									out.m_startDate = std::make_tuple(day, month, year);
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
							details.m_admincode = str;
							break;
						case 3: // A code specifying the responsible investigator or technician.
							details.m_technician = str;
							break;
						case 4: // A code specifying the used equipment.
							details.m_equipment = str;
							break;
						default: // Additional information.
							if (!details.m_recordingAdditional.empty())
							{
								details.m_recordingAdditional += detail::ADDITIONAL_SEPARATOR;
							}
							details.m_recordingAdditional.append(str);
							break;
						}
					}
				}
			}
		}

		out.m_patient = detail::ReduceString(out.m_patient);
		out.m_recording = detail::ReduceString(out.m_recording);
		out.m_reserved = detail::ReduceString(out.m_reserved);
		out.m_detail.m_patientCode = detail::ReduceString(out.m_detail.m_patientCode);
		out.m_detail.m_gender = detail::ReduceString(out.m_detail.m_gender);
		out.m_detail.m_birthdate = detail::ReduceString(out.m_detail.m_birthdate);
		out.m_detail.m_patientName = detail::ReduceString(out.m_detail.m_patientName);
		out.m_detail.m_patientAdditional = detail::ReduceString(out.m_detail.m_patientAdditional);
		out.m_detail.m_admincode = detail::ReduceString(out.m_detail.m_admincode);
		out.m_detail.m_technician = detail::ReduceString(out.m_detail.m_technician);
		out.m_detail.m_equipment = detail::ReduceString(out.m_detail.m_equipment);
		out.m_detail.m_recordingAdditional = detail::ReduceString(out.m_detail.m_recordingAdditional);

		return std::move(out);
	}

}
