#include <memory>
#include <iostream>

#include "clang/Frontend/CompilerInstance.h" // For clang::CompilerInstance.
#include "clang/Frontend/FrontendAction.h" // For clang::SyntaxOnlyAction.
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h" // For clang::Rewriter.
#include "clang/ASTMatchers/ASTMatchers.h" // For clang::ast_matchers.
#include "clang/ASTMatchers/ASTMatchFinder.h" // For clang::ast_matchers::MatchFinder..
#include "llvm/Support/CommandLine.h" // For llvm::cl::extrahelp.

using namespace clang::tooling;
using namespace llvm;

using std::cerr;
using std::cout;
using std::endl;
using std::make_unique;
using std::unique_ptr;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory ToolCategory("rename-fmt-format");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp(
R"~(
rename-fmt-format transforms fmt::format calls into absl::Substitute calls.
)~");

// All the following is largely based on the tutorial material in
//  https://s3.amazonaws.com/connect.linaro.org/yvr18/presentations/yvr18-223.pdf

struct FmtFrontendAction : public clang::ASTFrontendAction
{
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance &CI, StringRef file) override;

private:
    clang::Rewriter FmtRewriter;
};

struct FmtMatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback
{
    // Called on every match by the \c MatchFinder.
    void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

    // Called at the start of each translation unit.
    void onStartOfTranslationUnit() override;

    // Called at the end of each translation unit.
    void onEndOfTranslationUnit() override;
};

struct FmtASTConsumer : public clang::ASTConsumer
{
    FmtASTConsumer(clang::Rewriter &R);
    void HandleTranslationUnit(clang::ASTContext &Context);

private:
    FmtMatchCallback Callback;
    clang::ast_matchers::MatchFinder Matcher;
};

// Returns our ASTConsumer implementation per translation unit.
std::unique_ptr<clang::ASTConsumer> FmtFrontendAction::CreateASTConsumer(
        clang::CompilerInstance &CI,
        StringRef file
    )
{
    cout << "creating ATS consumer for " << file.str() << endl;
    this->FmtRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return make_unique<FmtASTConsumer>(this->FmtRewriter);
}

FmtASTConsumer::FmtASTConsumer(clang::Rewriter &R)
{
    using namespace clang::ast_matchers;

    // The clang-query match expression is callExpr(callee(functionDecl(hasName("fmt::format")))).
    // We are looking for a call site where the callee is "fmt::format".
    StatementMatcher CallSiteMatcher =
        callExpr(
                callee(
                    functionDecl(
                        hasName("fmt::format")
                    )
                )
        );

    this->Matcher.addMatcher(CallSiteMatcher, &this->Callback);
}

void FmtASTConsumer::HandleTranslationUnit(clang::ASTContext &Context)
{
    this->Matcher.matchAST(Context);
}

void FmtMatchCallback::run(const clang::ast_matchers::MatchFinder::MatchResult &Result)
{
    cout << PROGNAME << ": " << "found match result" << endl;
}

void FmtMatchCallback::onStartOfTranslationUnit()
{
    cout << PROGNAME << ": " << "begin translation unit" << endl;
}

void FmtMatchCallback::onEndOfTranslationUnit()
{
    cout << PROGNAME << ": " << "end translation unit" << endl;
}

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, ToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  return Tool.run(newFrontendActionFactory<FmtFrontendAction>().get());
}
