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
	std::ifstream is("test_generator_2.edf", std::ios::binary);

	if (!is)
		return -1;

	edfio::HeaderExam header;

	edfio::HeaderGeneralFields examFields;
	std::vector<edfio::HeaderSignalFields> signalFields;

	edfio::ReaderHeaderGeneral readerExam(is);
	edfio::ReaderHeaderSignal readerSignals(is);

	edfio::ProcessorHeaderGeneral processorGeneral;
	edfio::ProcessorHeaderSignal processorSignals;
	edfio::ProcessorHeaderExam processorExam(is);

	readerExam(examFields);
	processorGeneral(examFields, header.m_general);
	
	signalFields.resize(header.m_general.m_totalSignals);
	readerSignals(signalFields);	

	processorSignals(signalFields, header);
	processorExam(header, header);

	return 0;
}

