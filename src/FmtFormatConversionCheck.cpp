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

  // TODO(jpeach): this only matches string literals, so we miss cases where
  // the format string is a named variable :(

  Finder->addMatcher(CallSiteMatcher, this);
}

void FmtFormatConversionCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("Call");
  const auto *Arg0 = Result.Nodes.getNodeAs<StringLiteral>("Arg0");

  SourceManager& SM = *Result.SourceManager;

  SourceRange Range = CharSourceRange::getTokenRange(
      Call->getArg(0)->getBeginLoc(), Call->getArg(0)->getEndLoc()).getAsRange();

  std::string Value = Range.printToString(*Result.SourceManager);

  llvm::errs() << "literal is /" << Arg0->getString() << "/\n";

  if (!Call->getCallee()->getBeginLoc().isValid()) {
    llvm::errs() << "invalid callee begin loc " << Call->getCallee()->getBeginLoc().printToString(SM) << "\n";
  }

  if (!Call->getCallee()->getEndLoc().isValid()) {
    llvm::errs() << "invalid callee end loc " << Call->getCallee()->getEndLoc().printToString(SM) << "\n";
  }

  // Skip the fixit if the location is invalid. IIUC, it would be
  // invalid if this call doesn't map back to a specific source
  // location.
  if (Call->getCallee()->getBeginLoc().isInvalid() ||
      Call->getCallee()->getEndLoc().isInvalid()) {
    llvm::errs() << "skipping invalid call location\n";
    return;
  }

  // NOTE(jpeach): Some checks will call getBeginLoc.isMacroID() to
  // avoid rewriting code that is in a macro expansion. However, if
  // we do this, then most of the calls inside gtest test suites will
  // be skipped since they are all in gtest macros.

  // TODO(jpeach): Skip empty strings.

  using pos_t = std::string::size_type;
  std::string replacement = Arg0->getString();

  unsigned count = 0;
  const pos_t end = replacement.size() + 1;

  for (pos_t lhs = 0; lhs < (end - 1); ++lhs) {
    // Since we deal with paired '{}' below, we can just clean up
    // escaped '}' here.
    if (replacement[lhs] == '}' && replacement[lhs+1] == '}') {
      replacement.replace(lhs, 2, std::string("}"));
      continue;
    }

    if (replacement[lhs] != '{') {
      continue;
    }

    if (replacement[lhs + 1] == '{') {
      // This is a '{{', which is a quoted '{'. Replace it with '{'.
      replacement.replace(lhs, 2, std::string("{"));
      continue;
    }

    for (pos_t rhs = lhs + 1; rhs < end; ++rhs) {
      if (replacement[rhs] >= '0' && replacement[rhs] <= '9') {
        // We are still matching on "{[0-9]+}"
        continue;
      }

      if (replacement[rhs] == '}') {
        std::string num = replacement.substr(lhs + 1, rhs - lhs - 1);

        if (num.empty()) {
          // This is a '{}', replace it with the next counted argument.
          replacement.replace(lhs, 2, std::string("$") + std::to_string(count));
          ++count;
        } else {
          // This is a {N}, replace with $N.
          replacement.replace(lhs, num.size() + 2, std::string("$") + num);
        }

        break;
      }

      // This is some fancy formatting we can't deal with punt it.
      llvm::errs()
        << "skipping unsupported format token /"
        << replacement.substr(lhs, rhs - lhs)
        << "/\n";
    }
  }

  bool needsRaw = false;

  if (replacement.find("\n", 0) != std::string::npos) {
    needsRaw = true;
  }

  // TODO(jpeach): If this is a string literal with an embedded '\n',
  // then we end up coverting it into a raw string, which sucks.

  // When we get a raw string literal, the parser strips the
  // prefix and suffix. Put it back if it looks like we will
  // need it.
  if (needsRaw) {
    replacement = "R\"EOF(" + replacement + ")EOF\"";
  } else {
    replacement = "\"" + replacement + "\"";
  }

  llvm::errs() << "emitting replacement literal is /" << replacement << "/\n";

  // XXX(jpeach): The replacements behave strangely with macro expansions.
  // The warning output shows the range correctly, but the actual fixit
  // change is applied to the wrong offsets as though the replacement
  // range needs adjusting.

  diag(Call->getBeginLoc(), "call 'absl::Substitute' instead of 'fmt::format'")
      << FixItHint::CreateReplacement(
             CharSourceRange::getTokenRange(Call->getCallee()->getBeginLoc(), Call->getCallee()->getEndLoc()),
             "absl::Substitute")
      << FixItHint::CreateReplacement(
             CharSourceRange::getTokenRange(Arg0->getBeginLoc(), Arg0->getEndLoc()),
             replacement);
}

}  // namespace abseil
}  // namespace tidy
}  // namespace clang

// vim: set sw=2 ts=2 sts=2 et :
