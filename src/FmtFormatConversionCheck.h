//===--- FmtFormatConversionCheck.h - clang-tidy------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_ABSEIL_FMTFORMATCONVERSIONCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_ABSEIL_FMTFORMATCONVERSIONCHECK_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace abseil {

/// Flags uses of fmt::format to append to a string. Suggests absl::Substitute
/// should be used instead. 
class FmtFormatConversionCheck : public ClangTidyCheck {
public:
  FmtFormatConversionCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace abseil
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_ABSEIL_FMTFORMATCONVERSIONCHECK_H
