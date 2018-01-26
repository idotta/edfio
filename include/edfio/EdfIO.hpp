//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

// Header
#include <edfio/header/HeaderExam.hpp>
// Reader
#include <edfio/reader/ReaderHeaderExam.hpp>
// Writer
#include <edfio/writer/WriterHeaderExam.hpp>
// Store
#include <edfio/store/DataRecordStore.hpp>
#include <edfio/store/SignalRecordStore.hpp>
#include <edfio/store/SignalSampleStore.hpp>
#include <edfio/store/detail/StoreUtils.hpp>
#include <edfio/store/TalStore.hpp>
#include <edfio/store/TimeStampStore.hpp>
// Sink
#include <edfio/sink/DataRecordSink.hpp>
#include <edfio/sink/detail/SinkUtils.hpp>
// Processor
#include <edfio/processor/ProcessorSampleRecord.hpp>
#include <edfio/processor/ProcessorSample.hpp>
#include <edfio/processor/ProcessorTimeStampRecord.hpp>
#include <edfio/processor/ProcessorTimeStamp.hpp>
#include <edfio/processor/ProcessorTalRecord.hpp>
#include <edfio/processor/ProcessorAnnotation.hpp>

// STL
#include <fstream>
