# LLVM_Root is the root of the LLVM install tree. Probably the
# value passed to -DCMAKE_INSTALL_PREFIX when building LLVM.
LLVM_Root := /opt/llvm

LLVM_Config := $(LLVM_Root)/bin/llvm-config

# Minimal list of libraries taken from clang-refactor.
LLVM_Libraries_Clang_Refactor := \
	-lclangAST \
	-lclangBasic \
	-lclangFormat \
	-lclangFrontend \
	-lclangLex \
	-lclangRewrite \
	-lclangSerialization \
	-lclangTooling \
	-lclangToolingCore \
	-lclangToolingRefactoring \
	-lclang-cpp

PROGNAME := rename-fmt-format

DEFINES := -DPROGNAME=\"$(PROGNAME)\"
CXXFLAGS := -Wall -O2 $(shell $(LLVM_Config) --cxxflags) $(DEFINES)

LDFLAGS := \
	$(shell $(LLVM_Config) --ldflags) \
	-rpath $(shell $(LLVM_Config) --libdir)

$(PROGNAME): main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) \
		-o $@ $^ \
		$(LLVM_Libraries_Clang_Refactor) \
		$(shell $(LLVM_Config) --libs) \
		$(shell $(LLVM_Config) --system-libs)

clean:
	$(RM) main.o $(PROGNAME)
