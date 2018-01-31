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
#include "header/HeaderExam.hpp"
// Reader
#include "reader/ReaderHeaderExam.hpp"
// Writer
#include "writer/WriterHeaderExam.hpp"
// Store
#include "store/DataRecordStore.hpp"
#include "store/SignalRecordStore.hpp"
#include "store/SignalSampleStore.hpp"
#include "store/detail/StoreUtils.hpp"
#include "store/TalStore.hpp"
#include "store/TimeStampStore.hpp"
// Sink
#include "sink/DataRecordSink.hpp"
#include "sink/SignalRecordSink.hpp"
#include "sink/detail/SinkUtils.hpp"
// Processor
#include "processor/ProcessorSampleRecord.hpp"
#include "processor/ProcessorSample.hpp"
#include "processor/ProcessorTimeStampRecord.hpp"
#include "processor/ProcessorTimeStamp.hpp"
#include "processor/ProcessorTalRecord.hpp"
#include "processor/ProcessorAnnotation.hpp"

// STL
#include <fstream>
