//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../Utils.hpp"
#include "../core/DataFormat.hpp"
#include "../header/HeaderGeneral.hpp"
#include "detail/ProcessorUtils.hpp"

#include <sstream>

namespace edfio
{

	struct ProcessorHeaderGeneralFields
	{
		HeaderGeneral operator ()(HeaderGeneralFields in);
	};

	inline HeaderGeneral ProcessorHeaderGeneralFields::operator()(HeaderGeneralFields in)
	{
		HeaderGeneral out;

		if (/*detail::CheckFormatErrors(in.m_version())
			||*/ detail::CheckFormatErrors(in.m_patient())
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
			{
				int day{}, month{}, year{};
				auto [p1, e1] = std::from_chars(startdate.data(), startdate.data() + 2, day);
				auto [p2, e2] = std::from_chars(startdate.data() + 3, startdate.data() + 5, month);
				auto [p3, e3] = std::from_chars(startdate.data() + 6, startdate.data() + 8, year);
				if (e1 != std::errc{} || e2 != std::errc{} || e3 != std::errc{}
					|| day < 1 || day > 31 || month < 1 || month > 12)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
				year += year > 84 ? 1900 : 2000;
				out.m_startDate = std::make_tuple(day, month, year);
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
			{
				int hour{}, minute{}, second{};
				auto [p1, e1] = std::from_chars(starttime.data(), starttime.data() + 2, hour);
				auto [p2, e2] = std::from_chars(starttime.data() + 3, starttime.data() + 5, minute);
				auto [p3, e3] = std::from_chars(starttime.data() + 6, starttime.data() + 8, second);
				if (e1 != std::errc{} || e2 != std::errc{} || e3 != std::errc{}
					|| hour > 23 || minute > 59 || second > 59)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
				out.m_startTime = std::make_tuple(hour, minute, second);
			}
		}
		// Header Size
		{
			out.m_headerSize = detail::ParseInt(in.m_headerSize(), detail::GetError(FileErrc::FileContainsFormatErrors));
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
			out.m_datarecordsFile = detail::ParseLongLong(in.m_datarecordsFile(), detail::GetError(FileErrc::FileContainsFormatErrors));
			if (out.m_datarecordsFile < 0)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}
		// Datarecord Duration
		{
			double duration = detail::ParseDouble(in.m_datarecordDuration(), detail::GetError(FileErrc::FileContainsFormatErrors));
			out.m_datarecordDuration = duration;
			if (duration < 0)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			out.m_detail.m_fileDuration = out.m_datarecordDuration * out.m_datarecordsFile;
		}
		// Number of signals
		{
			int signals = detail::ParseInt(in.m_totalSignals(), detail::GetError(FileErrc::FileContainsFormatErrors));
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
								{
									int day{}, year{};
									auto [p1, e1] = std::from_chars(str.data(), str.data() + 2, day);
									auto [p2, e2] = std::from_chars(str.data() + 7, str.data() + 11, year);
									int month = detail::GetMonthFromString(std::string_view{str}.substr(3, 3));
									if (e1 != std::errc{} || e2 != std::errc{} || month == 0)
									{
										throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
									}
									out.m_startDate = std::make_tuple(day, month, year);
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

		return out;
	}

}
