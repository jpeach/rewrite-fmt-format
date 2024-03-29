
## Notes

## Dealing with default headers

On macOS in particular, you need to run clang tools with additional
arguments to ensure that it has access to SDK and toolchain headers
that are not in the default path for upstream LLVM builds. The
[clang-extra-args.sh](./clang-extra-args.sh) script pokes the default
clang to automatically add default header search paths.

## References

* [Clang Refactoring Engine](https://clang.llvm.org/docs/RefactoringEngine.html)
* [Clang AST Matcher Reference](https://clang.llvm.org/docs/LibASTMatchersReference.html)
* [LLVM Doxygen](https://clang.llvm.org/doxygen/index.html)

* [ How to build a C++ processing tool using the Clang libraries](https://s3.amazonaws.com/connect.linaro.org/yvr18/presentations/yvr18-223.pdf) by Peter Smith
* Steven Kelly's [clang-query](https://steveire.wordpress.com/2018/11/11/future-developments-in-clang-query/) series
    * [Extending clang-tidy](https://blogs.msdn.microsoft.com/vcblog/2018/10/19/exploring-clang-tooling-part-1-extending-clang-tidy)
    * [Examining the clang AST with clang-query](https://blogs.msdn.microsoft.com/vcblog/2018/10/23/exploring-clang-tooling-part-2-examining-the-clang-ast-with-clang-query)
    * [Rewriting Code with clang-tidy](https://blogs.msdn.microsoft.com/vcblog/2018/11/06/exploring-clang-tooling-part-3-rewriting-code-with-clang-tidy)
