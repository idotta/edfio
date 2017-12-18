//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#include <edfio/header/HeaderExam.hpp>
#include <edfio/core/Record.hpp>
#include <edfio/reader/ReaderHeader.hpp>
#include <edfio/reader/ReaderRecord.hpp>
#include <edfio/processor/ProcessorHeaderGeneral.hpp>
#include <edfio/processor/ProcessorHeaderSignal.hpp>
#include <edfio/processor/ProcessorHeaderExam.hpp>
#include <edfio/processor/ProcessorDataRecord.hpp>
#include <edfio/processor/ProcessorSignalRecord.hpp>
#include <edfio/processor/ProcessorAnnotationRecord.hpp>

#include <fstream>

int main()
{
	using namespace edfio;

	std::ifstream is("E:/Projetos/edfio/Calib5.edf", std::ios::binary);

	if (!is)
		return -1;

	HeaderExam header;

	// Read general fields
	ReaderHeaderGeneral readerGeneral;
	auto generalFields = readerGeneral(is);
	// Process general fields
	ProcessorHeaderGeneral processorGeneral;
	header.m_general = processorGeneral(std::move(generalFields));

	// Read signal fields
	ReaderHeaderSignal readerSignals(header.m_general.m_totalSignals);
	auto signalFields = readerSignals(is);
	// Process signal fields
	ProcessorHeaderSignal processorSignals(header.m_general);
	header.m_signals = processorSignals(std::move(signalFields));

	// Process exam header
	ProcessorHeaderExam processorExam(is);
	header = processorExam(std::move(header));

	is.seekg(header.m_general.m_headerSize, is.beg);

	// Read one data record
	size_t sz = header.m_general.m_detail.m_recordSize;
	ReaderRecord readerDataRecord(sz);
	auto recordField = readerDataRecord(is);
	// Process data record
	ProcessorDataRecord procDataRecord(header.m_signals, header.m_general.m_version);
	auto signalRecordFields = procDataRecord(recordField);

	// Process signal records
	for (size_t idx = 0; idx < signalRecordFields.size(); idx++)
	{
		if (!header.m_signals[idx].m_detail.m_isAnnotation)
		{
			ProcessorSignalRecord<SampleType::Digital> procDigital(header.m_signals[idx], header.m_general.m_version);
			auto digitalData = procDigital(signalRecordFields[idx]);

			ProcessorSignalRecord<SampleType::Physical> procPhysical(header.m_signals[idx], header.m_general.m_version);
			auto physical = procPhysical(signalRecordFields[idx]);
		}
		else
		{
			ProcessorAnnotationRecord procAnnot;
			auto annotations = procAnnot(signalRecordFields[idx]);
		}

	}

	return 0;
}
