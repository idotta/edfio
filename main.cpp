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

#include <fstream>

int main()
{
	std::ifstream is("E:/Projetos/edfio/Calib5.edf", std::ios::binary);

	if (!is)
		return -1;

	edfio::HeaderExam header;

	edfio::HeaderGeneralFields generalFields;
	std::vector<edfio::HeaderSignalFields> signalFields;

	edfio::ReaderHeaderGeneral readerExam(is);
	edfio::ReaderHeaderSignal readerSignals(is);

	edfio::ProcessorHeaderGeneral processorGeneral;
	edfio::ProcessorHeaderSignal processorSignals(header.m_general);
	edfio::ProcessorHeaderExam processorExam(is);

	// Read general fields
	generalFields = readerExam();
	// Process general fields
	header.m_general = processorGeneral(std::move(generalFields));
	
	// Read signal fields
	signalFields = readerSignals(header.m_general.m_totalSignals);
	// Process signal fields
	header.m_signals = processorSignals(std::move(signalFields));
	
	// Process exam header
	header = processorExam(std::move(header));

	return 0;
}
