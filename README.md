# edfio

[![CI](https://github.com/idotta/edfio/workflows/CI/badge.svg)](https://github.com/idotta/edfio/actions/workflows/ci.yml)
[![Code Quality](https://github.com/idotta/edfio/workflows/Code%20Quality/badge.svg)](https://github.com/idotta/edfio/actions/workflows/code-quality.yml)
[![Security](https://github.com/idotta/edfio/workflows/Security/badge.svg)](https://github.com/idotta/edfio/actions/workflows/security.yml)

A C++20 header-only library to read/write EDF(+)/BDF(+) files.

## Building and Testing

This project uses CMake for building tests and examples. See [BUILD.md](BUILD.md) for detailed build instructions.

Quick start:
```bash
cmake --preset debug
cmake --build build/debug
ctest --test-dir build/debug
```

### Sample file
The sample file 'Calib5.edf' provided in the root directory of this source tree
was taken from the 
<a href="https://sites.google.com/view/diegoalvarezestevez/projects/polyman">
Polyman
</a>
Demodata only for testing.

### Terms and Conditions
Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)

This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.

Official repository: https://github.com/idotta/edfio
