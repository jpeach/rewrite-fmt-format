RM_F := rm -rf
LN_S := ln -sf

# LLVM release branch to clone.
LLVM_Branch := release/9.x
LLVM_Repository := https://github.com/llvm/llvm-project.git
LLVM_Dir := llvm

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


Tidy_Check_Prefix := $(LLVM_Dir)/clang-tools-extra/clang-tidy/abseil
Tidy_Check_Files := \
	FmtFormatConversionCheck.cpp \
	FmtFormatConversionCheck.h

setup: checkout-llvm install-links apply-build-patch

checkout-llvm: llvm/README.md
$(LLVM_Dir)/README.md:
	@$(RM_F) llvm
	@git clone --depth 1 --single-branch --branch $(LLVM_Branch) $(LLVM_Repository) $(dir $@)

install-links: $(patsubst %,$(Tidy_Check_Prefix)/%,$(Tidy_Check_Files))
$(patsubst %,$(Tidy_Check_Prefix)/%,$(Tidy_Check_Files)):
	$(LN_S) $$(pwd)/src/$(notdir $@) $@

apply-build-patch: src/build.patch
	@if ! grep -q FmtFormatConversionCheck.cpp $(Tidy_Check_Prefix)/CMakeLists.txt ; then \
		patch -d llvm -p0 < $< ; \
	fi

clean:
	$(RM) main.o $(PROGNAME)
