//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../core/Record.hpp"
#include "../header/HeaderGeneral.hpp"
#include "../header/HeaderSignal.hpp"

namespace edfio
{

	enum class SampleType
	{
		Physical,
		Digital
	};

	namespace impl
	{

		template <SampleType SampleT>
		struct Sample
		{
		};

		template <>
		struct Sample<SampleType::Physical>
		{
			using type = double;
		};

		template <>
		struct Sample<SampleType::Digital>
		{
			using type = int;
		};

		Sample<SampleType::Digital>::type ConvertSample(double offset, double scaling, Sample<SampleType::Physical>::type sample)
		{
			return static_cast<Sample<SampleType::Digital>::type>((sample - offset) / scaling);
		}

		Sample<SampleType::Physical>::type ConvertSample(double offset, double scaling, Sample<SampleType::Digital>::type sample)
		{
			return scaling * static_cast<Sample<SampleType::Physical>::type>(sample) + offset;
		}

	}

}
