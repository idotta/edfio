//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../Errors.hpp"
#include "HeaderGeneral.hpp"
#include "HeaderSignal.hpp"

#include <cstdint>
#include <span>
#include <string>

namespace edfio {

namespace detail {

inline HeaderGeneral CreateHeaderGeneral(
    DataFormat version, std::string patient, std::string recording,
    int32_t startDateD, int32_t startDateM, int32_t startDateY,
    int32_t startTimeH, int32_t startTimeM, int32_t startTimeS,
    int32_t headerSize, std::string reserved, int64_t datarecordsFile,
    double datarecordDuration, std::span<const HeaderSignal> signals) {
  HeaderGeneral header;
  header.m_version = version;
  header.m_patient = patient;
  header.m_recording = recording;
  header.m_startDate = Date{startDateD, startDateM, startDateY};
  header.m_startTime = Time{startTimeH, startTimeM, startTimeS};
  header.m_headerSize = headerSize;
  header.m_reserved = reserved;
  header.m_datarecordsFile = datarecordsFile;
  header.m_datarecordDuration = datarecordDuration;
  header.m_totalSignals = static_cast<int32_t>(signals.size());

  // Record size
  header.m_detail.m_recordSize = 0;
  for (auto &signal : signals) {
    header.m_detail.m_recordSize += signal.m_samplesInDataRecord;
  }
  header.m_detail.m_recordSize *= GetSampleBytes(version);

  return header;
}

inline HeaderGeneral CreateHeaderGeneralPlus(
    DataFormat version, std::string patientCode, std::string gender,
    std::string birthdate, std::string patientName,
    std::string patientAdditional, std::string admincode,
    std::string technician, std::string equipment,
    std::string recordingAdditional, int32_t startDateD, int32_t startDateM,
    int32_t startDateY, int32_t startTimeH, int32_t startTimeM,
    int32_t startTimeS, int32_t headerSize, std::string reserved,
    int64_t datarecordsFile, double datarecordDuration,
    std::span<const HeaderSignal> signals) {
  auto header = CreateHeaderGeneral(
      version, "", "", startDateD, startDateM, startDateY, startTimeH,
      startTimeM, startTimeS, headerSize, reserved, datarecordsFile,
      datarecordDuration, signals);

  header.m_detail.m_patientCode = patientCode;
  header.m_detail.m_gender = gender;
  header.m_detail.m_birthdate = birthdate;
  header.m_detail.m_patientName = patientName;
  header.m_detail.m_patientAdditional = patientAdditional;
  header.m_detail.m_admincode = admincode;
  header.m_detail.m_technician = technician;
  header.m_detail.m_equipment = equipment;
  header.m_detail.m_recordingAdditional = recordingAdditional;

  return header;
}

inline HeaderSignal
CreateHeaderSignal(std::string label, int32_t samplesInDataRecord,
                   double physicalMin, double physicalMax, int32_t digitalMin,
                   int32_t digitalMax, int64_t signalOffset = 0,
                   bool annotation = false, std::string transducer = "",
                   std::string physDimension = "", std::string prefilter = "",
                   std::string reserved = "") {
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

  signal.m_detail.m_signalOffset = signalOffset;
  auto digitalRange = signal.m_digitalMax - signal.m_digitalMin;
  if (digitalRange == 0) {
    throw std::invalid_argument(
        GetError(FileErrc::FileContainsFormatErrors));
  }
  signal.m_detail.m_scaling =
      (signal.m_physicalMax - signal.m_physicalMin) / digitalRange;
  signal.m_detail.m_offset =
      signal.m_physicalMin - signal.m_detail.m_scaling * signal.m_digitalMin;
  signal.m_detail.m_isAnnotation = annotation;

  return signal;
}

} // namespace detail

} // namespace edfio
