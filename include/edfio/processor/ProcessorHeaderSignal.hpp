//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "ProcessorBase.hpp"
#include "../Defs.hpp"
#include "../header/HeaderGeneral.hpp"
#include "../header/HeaderSignal.hpp"

namespace edfio
{

	struct ProcessorHeaderSignal : ProcessorBase<std::vector<HeaderSignalFields>, std::vector<HeaderSignal>>
	{
		ProcessorHeaderSignal(HeaderGeneral &general) : m_general(general) {}
		TypeOu operator ()(TypeIn in);
	private:
		HeaderGeneral &m_general;
	};

}

#include "impl/ProcessorHeaderSignal.ipp"
