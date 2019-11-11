//===--- FmtFormatConversionCheck.cpp - clang-tidy--------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FmtFormatConversionCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace abseil {

namespace {

}  // namespace

void FmtFormatConversionCheck::registerMatchers(MatchFinder *Finder) {
  if (!getLangOpts().CPlusPlus) {
  	return;
  }

  // The clang-query match expression is callExpr(callee(functionDecl(hasName("fmt::format")))).
  // We are looking for a call site where the callee is "fmt::format".
  StatementMatcher CallSiteMatcher =
    callExpr(
      callee(functionDecl(hasName("::fmt::format"))),
      hasArgument(0, stringLiteral().bind("Arg0"))
    ).bind("Call");

  Finder->addMatcher(CallSiteMatcher, this);
}

void FmtFormatConversionCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("Call");
  const auto *Arg0 = Result.Nodes.getNodeAs<StringLiteral>("Arg0");

  SourceManager& SM = *Result.SourceManager;

  SourceRange Range = CharSourceRange::getTokenRange(
      Call->getArg(0)->getBeginLoc(), Call->getArg(0)->getEndLoc()).getAsRange();

  std::string Value = Range.printToString(*Result.SourceManager);

  const char *Begin = SM.getCharacterData(Call->getArg(0)->getBeginLoc());
  const char *End = SM.getCharacterData(Call->getArg(0)->getEndLoc());

  llvm::errs() << "arg0 is '" << std::string(Begin, End) << "'\n";
  llvm::errs() << "literal is '" << Arg0->getString() << "\n";

  if (!Call->getCallee()->getBeginLoc().isValid()) {
    llvm::errs() << "invalid callee begin loc " << Call->getCallee()->getBeginLoc().printToString(SM) << "\n";
  }

  if (!Call->getCallee()->getEndLoc().isValid()) {
    llvm::errs() << "invalid callee end loc " << Call->getCallee()->getEndLoc().printToString(SM) << "\n";
  }

  // Skip the fixit if the location is invalid.
  if (Call->getCallee()->getBeginLoc().isInvalid() ||
      Call->getCallee()->getEndLoc().isInvalid()) {
    return;
  }

  // Skip the fixit if it is in a macro expansion.
  if (Arg0->getBeginLoc().isMacroID()) {
    return;
  }

  using pos_t = std::string::size_type;
  std::string replacement = Arg0->getString();

  unsigned count = 0;

  pos_t begin = 0;
  pos_t end = replacement.size() + 1;

  for (pos_t pos = replacement.find("{}", 0);
        pos != std::string::npos;
        pos = replacement.find("{}", pos + 1)) {
    if ((pos > begin && replacement[pos-1] == '{') &&
       (pos + 2 < end && replacement[pos+2] == '}')) {
      // XXX it's not really {{}} that is special, it's that you double them to
      // quote them, so {{ -> { and }} ->, which means that {{{}}} -> {$0}.
      llvm::errs() << "skipping {{}} token\n";
      ++pos;
      continue;
    }

    // Replace the {} with $count.
    replacement.replace(pos, 2, std::string("$") + std::to_string(count));
    ++count;
  }

  for (pos_t pos = replacement.find("{{}}", 0);
        pos != std::string::npos;
        pos = replacement.find("{{}}", pos + 1)) {
    // Replace the {{}} with {}.
    replacement.replace(pos, 4, "{}");
  }

  bool needsRaw = false;

  if (replacement.find("\n", 0) != std::string::npos) {
    needsRaw = true;
  }

/*
  for (pos_t pos = replacement.find("\n", 0);
        pos != std::string::npos;
        pos = replacement.find("\n", pos + 1)) {
    replacement.replace(pos, 1, "\\n");
  }
*/

  // When we get a raw string literal, the parser strips the
  // prefix and suffix. Put it back if it looks like we will
  // need it.
  if (needsRaw) {
    replacement = "R\"EOF(" + replacement + ")EOF\"";
  } else {
    replacement = "\"" + replacement + "\"";
  }

  llvm::errs() << "replacement literal is '" << replacement << "\n";

  // Emit a warning and emit fixits to go from
  //   x = fmt::Format("foo {} bar {}", ...)
  // to
  //   absl::Substitute("foo $0 bar $1", ...)
  diag(Call->getBeginLoc(), "call 'absl::Substitute' instead of 'fmt::format'")
      << FixItHint::CreateReplacement(
             CharSourceRange::getTokenRange(Call->getCallee()->getBeginLoc(), Call->getCallee()->getEndLoc()),
             "absl::Substitute")
      << FixItHint::CreateReplacement(
             CharSourceRange::getTokenRange(Arg0->getBeginLoc(), Arg0->getEndLoc()),
             replacement);
      ;
}

}  // namespace abseil
}  // namespace tidy
}  // namespace clang

// vim: set sw=2 ts=2 sts=2 et :
