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
#include <edfio/processor/ProcessorHeaderExam.hpp>
#include <edfio/processor/ProcessorHeaderSignal.hpp>

#include <fstream>

int main()
{
	std::ifstream is("test_generator_2.edf", std::ios::binary);

	if (!is)
		return -1;

	std::pair<edfio::HeaderExam, std::vector<edfio::HeaderSignal>> header;

	edfio::HeaderExamFields examFields;
	std::vector<edfio::HeaderSignalFields> signalFields;

	edfio::ReaderHeaderExam readerExam(is);
	edfio::ReaderHeaderSignal readerSignals(is);

	edfio::ProcessorHeaderExam processorExam;
	edfio::ProcessorHeaderSignal processorSignals;

	readerExam(examFields);
	processorExam(examFields, header.first);
	
	signalFields.resize(header.first.m_totalSignals);
	readerSignals(signalFields);	

	processorSignals(signalFields, header);

	return 0;
}

