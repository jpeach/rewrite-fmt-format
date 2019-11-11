RM_F := rm -rf
LN_S := ln -sf
MKDIR_P := mkdir -p

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

build:
	cd $(LLVM_Dir)/build && ninja bin/clang-tidy

configure: $(LLVM_Dir)/build/rules.ninja
$(LLVM_Dir)/build/rules.ninja: $(LLVM_Dir)/build
	cd $< && cmake -G Ninja ../llvm \
		-DCMAKE_INSTALL_PREFIX=$(LLVM_Root) \
		-DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
		-DLLVM_BUILD_TESTS=OFF \
		-DLLVM_BUILD_LLVM_DYLIB=ON \
		-DBUILD_SHARED_LIBS=ON

$(LLVM_Dir)/build:
	$(MKDIR_P) $@

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
	[[ -d $(LLVM_Dir)/build ]] && cd $(LLVM_Dir)/build ]] && ninja clean
	$(RM_F) main.o $(PROGNAME)

mrproper: clean
	$(RM_F) $(LLVM_Dir)
