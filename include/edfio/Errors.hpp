//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

namespace edfio {

enum class FileErrc {
  FileDoesNotOpen,
  FileNotOpened,
  FileReadError,
  FileContainsFormatErrors,
  FileContainsInvalidAnnotations,
  FileWriteError,
  FileWriteInvalidAnnotations
};

[[nodiscard]] inline constexpr const char *GetError(FileErrc err) {
  switch (err) {
  case FileErrc::FileDoesNotOpen:
    return "Error: file does not open";
  case FileErrc::FileNotOpened:
    return "Error: file not opened";
  case FileErrc::FileReadError:
    return "Error: can't read file";
  case FileErrc::FileContainsFormatErrors:
    return "Error: file contains format errors";
  case FileErrc::FileContainsInvalidAnnotations:
    return "Error: file contains invalid annotations";
  case FileErrc::FileWriteError:
    return "Error: can't write on file";
  case FileErrc::FileWriteInvalidAnnotations:
    return "Error: writing invalid annotations";
  default:
    return "Unspecified error";
  }
}

} // namespace edfio
