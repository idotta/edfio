//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#include <edfio/header/HeaderExam.hpp>
#include <edfio/reader/ReaderHeader.hpp>
#include <edfio/processor/ProcessorHeaderGeneral.hpp>
#include <edfio/processor/ProcessorHeaderSignal.hpp>
#include <edfio/processor/ProcessorHeaderExam.hpp>
#include <edfio/reader/ReaderRecord.hpp>
#include <edfio/core/Record.hpp>
#include <edfio/processor/ProcessorDataRecord.hpp>

#include <fstream>

int main()
{
	std::ifstream is("E:/Projetos/edfio/Calib5.edf", std::ios::binary);

	if (!is)
		return -1;

	edfio::HeaderExam header;

	// Read general fields
	edfio::ReaderHeaderGeneral readerGeneral;
	auto generalFields = readerGeneral(is);
	// Process general fields
	edfio::ProcessorHeaderGeneral processorGeneral;
	header.m_general = processorGeneral(std::move(generalFields));

	// Read signal fields
	edfio::ReaderHeaderSignal readerSignals(header.m_general.m_totalSignals);
	auto signalFields = readerSignals(is);
	// Process signal fields
	edfio::ProcessorHeaderSignal processorSignals(header.m_general);
	header.m_signals = processorSignals(std::move(signalFields));

	// Process exam header
	edfio::ProcessorHeaderExam processorExam(is);
	header = processorExam(std::move(header));

	is.seekg(header.m_general.m_headerSize, is.beg);

	// Read one data record
	size_t sz = header.m_general.m_detail.m_recordSize;
	edfio::ReaderRecord readerDataRecord(sz);
	auto recordField = readerDataRecord(is);
	edfio::ProcessorDataRecord procDataRecord(header.m_signals, edfio::GetSampleSize(header.m_general.m_version));
	auto signalRecordFields = procDataRecord(recordField);

	return 0;
}
