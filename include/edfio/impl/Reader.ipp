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

#include <sstream>
#include <cctype>
#include <algorithm>

namespace edfio
{
	static const char ADDITIONAL_SEPARATOR = '|';

	static bool CheckFormatErrors(const std::string &str)
	{
		for (auto& c : str)
		{
			if (!std::isprint(c))
			{
				return true;
			}
		}
		return false;
	}

	static bool CheckFormatErrors(const std::vector<char> &str)
	{
		for (auto& c : str)
		{
			if (!std::isprint(c))
			{
				return true;
			}
		}
		return false;
	}

	static int GetMonthFromString(const std::string &str)
	{
		static const std::vector<std::string> months = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
		for (size_t idx = 0; idx < months.size(); idx++)
		{
			if (str == months[idx])
			{
				return idx + 1;
			}
		}
		return 0;
	}

	FileErrc Reader::CheckExamHeader()
	{
		auto& is = m_stream;
		auto& header = m_header;

		if (!is.is_open())
			return FileErrc::FileDoesNotOpen;

		is.clear();
		is.seekg(0, std::ios::beg);

		std::vector<char> hdrChar(256);
		auto iter = hdrChar.begin();

		is.read(hdrChar.data(), hdrChar.size());

		if (!is)
			return FileErrc::FileReadError;

		if (CheckFormatErrors(hdrChar))
		{
			return FileErrc::FileContainsFormatErrors;
		}

		// Version
		{
			std::string version(iter, iter + 8);
			iter += 8;
			if (version.front() == -1)   // BDF-file
			{
				if (version.substr(1) != "BIOSEMI")
				{
					return FileErrc::FileContainsFormatErrors;
				}
				m_format = DataFormat::Bdf;
			}
			else    // EDF-file
			{
				if (version != "0       ")
				{
					return FileErrc::FileContainsFormatErrors;
				}
				m_format = DataFormat::Edf;
			}
		}
		// Patient Name
		{
			std::string patient(iter, iter + 80);
			iter += 80;
			header.m_patient = patient;
		}
		// Recording
		{
			std::string recording(iter, iter + 80);
			iter += 80;
			header.m_recording = recording;
		}
		// Start Date
		{
			std::string startdate(iter, iter + 8);
			iter += 8;
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
				header.m_startDay = std::stoi(startdate);
				header.m_startMonth = std::stoi(startdate.substr(3));
				header.m_startYear = std::stoi(startdate.substr(6));
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (header.m_startDay < 1 || header.m_startDay > 31
				|| header.m_startMonth < 1 || header.m_startMonth > 12)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (header.m_startYear > 84)
			{
				header.m_startYear += 1900;
			}
			else
			{
				header.m_startYear += 2000;
			}
		}
		// Start Time
		{
			std::string starttime(iter, iter + 8);
			iter += 8;
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
				header.m_startHour = std::stoi(starttime);
				header.m_startMinute = std::stoi(starttime.substr(3));
				header.m_startSecond = std::stoi(starttime.substr(6));
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}

			if (header.m_startHour > 23
				|| header.m_startMinute > 59
				|| header.m_startSecond > 59)
			{
				return FileErrc::FileContainsFormatErrors;
			}
		}
		// Header Size
		{
			std::string numbytes(iter, iter + 8);
			iter += 8;
			try
			{
				header.m_detail.m_headerSize = std::stoi(numbytes);
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
		}
		// Reserved
		{
			std::string reserved(iter, iter + 44);
			iter += 44;
			if (IsEdf(m_format))
			{
				if (reserved.find("EDF+C") != std::string::npos)
				{
					m_format = DataFormat::EdfPlusC;
				}
				else if (reserved.find("EDF+D") != std::string::npos)
				{
					m_format = DataFormat::EdfPlusD;
				}
			}
			else if (IsBdf(m_format))
			{
				if (reserved.find("BDF+C") != std::string::npos)
				{
					m_format = DataFormat::BdfPlusC;
				}
				else if (reserved.find("BDF+D") != std::string::npos)
				{
					m_format = DataFormat::BdfPlusD;
				}
			}
		}
		// Datarecord in File
		{
			std::string datarecords(iter, iter + 8);
			iter += 8;
			try
			{
				header.m_datarecordsFile = std::stoll(datarecords);
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (header.m_datarecordsFile < 1)
			{
				return FileErrc::FileContainsFormatErrors;
			}
		}
		// Datarecord Duration
		{
			std::string datarecordDuration(iter, iter + 8);
			iter += 8;
			double duration = -1;
			try
			{
				duration = std::stod(datarecordDuration);
				header.m_datarecordDuration = static_cast<long long>(std::fabs(duration) * static_cast<double>(TIME_DIMENSION));
				header.m_detail.m_datarecordDuration = duration;
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (duration < -0.000001)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			header.m_fileDuration = header.m_datarecordDuration * header.m_datarecordsFile;
		}
		// Number of signals
		{
			std::string numsignals(iter, iter + 4);
			int signals = 0;
			try
			{
				signals = std::stoi(numsignals);
			}
			catch (std::exception& e)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (signals < 1 || signals > MAX_SIGNALS
				|| (signals * 256 + 256) != header.m_detail.m_headerSize)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			header.m_totalSignals = signals;
		}
		// Plus Fields
		if (IsPlus(m_format))
		{
			// Patient Name
			{
				std::vector<std::string> fields;
				std::string field;
				std::istringstream f(header.m_patient);
				while (std::getline(f, field, ' '))
				{
					fields.push_back(field);
				}
				if (fields.size() < 4)
				{
					return FileErrc::FileContainsFormatErrors;
				}

				header.m_patientAdditional.clear();

				for (size_t i = 0; i < fields.size(); i++)
				{
					auto& str = fields[i];
					std::replace(str.begin(), str.end(), '_', ' '); // replace all '_' to ' '
					switch (i)
					{
					case 0: // The code by which the patient is known in the hospital administration.
						header.m_patientCode = str;
						break;
					case 1: // Sex
						if (str == "F" || str == "M" || str == "X")
						{
							header.m_gender = str;
						}
						else
						{
							return FileErrc::FileContainsFormatErrors;
						}
						break;
					case 2: // Birthdate in dd-MMM-yyyy format using the English 3-character abbreviations of the month in capitals. 02-AUG-1951 is OK, while 2-AUG-1951 is not.
						header.m_birthdate = str;
						break;
					case 3: // The patients name.
						header.m_patientName = str;
						break;
					default: // Additional information.
						if (!header.m_patientAdditional.empty())
						{
							header.m_patientAdditional += ADDITIONAL_SEPARATOR;
						}
						header.m_patientAdditional.append(str);
						break;
					}
				}
			}
			// Recording
			{
				std::vector<std::string> fields;
				std::string field;
				std::istringstream f(header.m_recording);
				while (std::getline(f, field, ' '))
				{
					fields.push_back(field);
				}

				if (fields.size() < 5)
				{
					return FileErrc::FileContainsFormatErrors;
				}

				header.m_recording.clear();

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
									header.m_startDay = std::stoi(str.substr(0, 2));
									header.m_startYear = std::stoi(str.substr(7, 4));
								}
								catch (std::exception& e)
								{
									return FileErrc::FileContainsFormatErrors;
								}
								header.m_startMonth = GetMonthFromString(str.substr(3, 3));
								if (header.m_startMonth == 0)
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
							header.m_admincode = str;
							break;
						case 3: // A code specifying the responsible investigator or technician.
							header.m_technician = str;
							break;
						case 4: // A code specifying the used equipment.
							header.m_equipment = str;
							break;
						default: // Additional information.
							if (!header.m_recording.empty())
							{
								header.m_recording += ADDITIONAL_SEPARATOR;
							}
							header.m_recording.append(str);
							break;
						}
					}
				}
			}
		}

		return FileErrc::NoError;
	}

	FileErrc Reader::CheckSignalHeaders()
	{
		auto& is = m_stream;
		auto& header = m_header;
		auto& signals = m_signals;

		if (!is.is_open())
			return FileErrc::FileDoesNotOpen;

		signals.clear();
		signals.resize(header.m_totalSignals);

		is.clear();
		is.seekg(256, is.beg);

		std::vector<char> hdrChar(signals.size() * 256);
		auto iter = hdrChar.begin();

		is.read(hdrChar.data(), hdrChar.size());

		if (!is)
			return FileErrc::FileReadError;

		if (CheckFormatErrors(hdrChar))
		{
			return FileErrc::FileContainsFormatErrors;
		}

		// Labels
		{
			size_t totalAnnotationChannels = 0;
			for (auto &signal : signals)
			{
				std::string label(iter, iter + 16);
				iter += 16;
				if (IsPlus(m_format))
				{
					if (label.find("Annotation") != std::string::npos)
					{
						totalAnnotationChannels++;
						signal.m_detail.m_isAnnotation = true;
					}
					else
					{
						signal.m_detail.m_isAnnotation = false;
					}
				}
				else
				{
					signal.m_detail.m_isAnnotation = false;
				}
				signal.m_label = label;
			}
			if (IsPlus(m_format) && totalAnnotationChannels == 0)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (signals.size() != totalAnnotationChannels || !IsPlus(m_format))
			{
				if (header.m_datarecordDuration < 1)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
		}
		// Transducers Types
		{
			for (auto &signal : signals)
			{
				std::string transducer(iter, iter + 80);
				iter += 80;
				signal.m_transducer = transducer;

				if (signal.m_detail.m_isAnnotation)
				{
					if (transducer.find_first_not_of(' ') != std::string::npos)
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
			}
		}
		// Physical Dimensions
		{
			for (auto &signal : signals)
			{
				std::string physDimension(iter, iter + 8);
				iter += 8;
				signal.m_physDimension = physDimension;
			}
		}
		// Physical Minima
		{
			for (auto &signal : signals)
			{
				std::string physMin(iter, iter + 8);
				iter += 8;

				try
				{
					signal.m_PhysicalMin = std::stod(physMin);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
		}
		// Physical Maxima
		{
			for (auto &signal : signals)
			{
				std::string physMax(iter, iter + 8);
				iter += 8;

				try
				{
					signal.m_physicalMax = std::stod(physMax);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
		}
		// Digital Minima
		{
			for (auto &signal : signals)
			{
				std::string digMin(iter, iter + 8);
				iter += 8;

				int n = 0;
				try
				{
					n = std::stoi(digMin);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}

				if (signal.m_detail.m_isAnnotation)
				{
					if (IsEdf(m_format) && IsPlus(m_format))
					{
						if (n != -32768)
						{
							return FileErrc::FileContainsFormatErrors;
						}
					}
					else if (IsBdf(m_format) && IsPlus(m_format))
					{
						if (n != -8388608)
						{
							return FileErrc::FileContainsFormatErrors;
						}
					}
				}
				else if (IsEdf(m_format))
				{
					if ((n > 32767) || (n < -32768))
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
				else if (IsBdf(m_format))
				{
					if ((n > 8388607) || (n < -8388608))
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
				signal.m_digitalMin = n;
			}
		}
		// Digital Maxima
		{
			for (auto &signal : signals)
			{
				std::string digMax(iter, iter + 8);
				iter += 8;

				int n = 0;
				try
				{
					n = std::stoi(digMax);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}

				if (signal.m_detail.m_isAnnotation)
				{
					if (IsEdf(m_format) && IsPlus(m_format))
					{
						if (n != 32767)
						{
							return FileErrc::FileContainsFormatErrors;
						}
					}
					else if (IsBdf(m_format) && IsPlus(m_format))
					{
						if (n != 8388607)
						{
							return FileErrc::FileContainsFormatErrors;
						}
					}
				}
				else if (IsEdf(m_format))
				{
					if ((n > 32767) || (n < -32768))
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
				else if (IsBdf(m_format))
				{
					if ((n > 8388607) || (n < -8388608))
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
				signal.m_digitalMax = n;
				if (signal.m_digitalMax < (signal.m_digitalMin + 1))
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
		}
		// Prefilter
		{
			for (auto &signal : signals)
			{
				std::string prefilter(iter, iter + 80);
				iter += 80;
				signal.m_prefilter = prefilter;

				if (signal.m_detail.m_isAnnotation)
				{
					if (prefilter.find_first_not_of(' ') != std::string::npos)
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
			}
		}
		// Samples in each datarecord
		{
			size_t recordsize = 0;

			for (auto &signal : signals)
			{
				std::string nrSamples(iter, iter + 8);
				iter += 8;

				int n = 0;
				try
				{
					n = std::stoi(nrSamples);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}

				if (n < 1)
				{
					return FileErrc::FileContainsFormatErrors;
				}
				signal.m_samplesDatarecord = n;
				signal.m_samplesFile = n * header.m_datarecordsFile;
				recordsize += n;
			}

			if (IsBdf(m_format))
			{
				recordsize *= 3;
				if (recordsize > 0xF00000)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
			else
			{
				recordsize *= 2;
				if (recordsize > 0xA00000)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
			header.m_detail.m_recordSize = recordsize;
		}
		// Reserved
		{
			for (auto &signal : signals)
			{
				std::string reserved(iter, iter + 32);
				iter += 32;
				signal.m_detail.m_reserved = reserved;
			}
		}
		// Details
		{
			size_t n = 0;
			for (auto &signal : signals)
			{
				signal.m_detail.m_bufferOffset = n;
				if (IsBdf(m_format))
					n += signal.m_samplesDatarecord * 3;
				else  if (IsEdf(m_format))
					n += signal.m_samplesDatarecord * 2;

				signal.m_detail.m_bitValue = (signal.m_physicalMax - signal.m_PhysicalMin) / (signal.m_digitalMax - signal.m_digitalMin);
				signal.m_detail.m_offset = signal.m_physicalMax / signal.m_detail.m_bitValue - signal.m_digitalMax;
			}
		}
		// File size
		{
			// get length of file:
			is.seekg(0, is.end);
			size_t length = is.tellg();
			is.seekg(0, is.beg);

			if (length != (header.m_detail.m_recordSize * header.m_datarecordsFile + header.m_detail.m_headerSize))
			{
				FileErrc::FileContainsFormatErrors;
			}
		}

		return FileErrc::NoError;
	}

	FileErrc Reader::Open(const std::string &path)
	{
		if (m_stream.is_open())
			m_stream.close();

		m_stream.open(path, std::ifstream::binary);

		if (!m_stream.is_open())
			return FileErrc::FileDoesNotOpen;

		auto errc = CheckExamHeader();
		if (errc != FileErrc::NoError)
			return errc;

		errc = CheckSignalHeaders();
		if (errc != FileErrc::NoError)
			return errc;

		return FileErrc::NoError;
	}

	FileErrc Reader::Close()
	{
		if (m_stream.is_open())
			m_stream.close();

		return FileErrc::NoError;
	}
}
