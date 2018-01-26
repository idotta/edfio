//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../Utils.hpp"
#include "../../header/HeaderExam.hpp"
#include "../WriterHeaderGeneral.hpp"
#include "../WriterHeaderSignals.hpp"
#include "../../processor/ProcessorHeaderGeneral.hpp"
#include "../../processor/ProcessorHeaderSignal.hpp"

#include <stdexcept>
#include <fstream>

namespace edfio
{

	void WriterHeaderExam::operator ()(Stream &stream, HeaderExam &input)
	{
		// Process header general
		auto general = std::move(ProcessorHeaderGeneral{}(input.m_general));

		// Process signal fields
		auto signals = std::move(ProcessorHeaderSignal{}(input.m_signals));

		// Write general
		WriterHeaderGeneral{}(stream, std::move(general));

		// Write signals
		WriterHeaderSignals{}(stream, std::move(signals));
	}

}
