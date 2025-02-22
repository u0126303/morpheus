CMAKE_FLAGS_LLVM =

-include Makefile.local

MAKEFILE_DIR = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

BUILD_TYPE ?= Debug
BUILD_JOBS ?= 4
LINK_JOBS  ?= 1

INSTALLDIR ?= $(MAKEFILE_DIR)install

CMAKE_GENERATOR ?= Ninja
 
#############################################################################

MKDIR = mkdir -p
CMAKE = cmake
CP = cp

#############################################################################

SRCDIR_LLVM   = $(MAKEFILE_DIR)llvm
SRCDIR_CLANG  = $(MAKEFILE_DIR)clang

BUILDDIR_LLVM = llvm/build

CMAKE_FLAGS_LLVM += -B $(BUILDDIR_LLVM)
CMAKE_FLAGS_LLVM += -DCMAKE_INSTALL_PREFIX=$(INSTALLDIR)
CMAKE_FLAGS_LLVM += -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
CMAKE_FLAGS_LLVM += -G "$(CMAKE_GENERATOR)"
CMAKE_FLAGS_LLVM += -S $(SRCDIR_LLVM)
CMAKE_FLAGS_LLVM += -DLLVM_PARALLEL_COMPILE_JOBS=$(BUILD_JOBS)
CMAKE_FLAGS_LLVM += -DLLVM_PARALLEL_LINK_JOBS=$(LINK_JOBS)
CMAKE_FLAGS_LLVM += -DLLVM_TARGETS_TO_BUILD=RISCV
CMAKE_FLAGS_LLVM += -DLLVM_ENABLE_PROJECTS="llvm;clang;lld"
CMAKE_FLAGS_LLVM += -DLLVM_USE_LINKER=
CMAKE_FLAGS_LLVM += -DLLVM_ENABLE_LLD=1
CMAKE_FLAGS_LLVM += -DLLVM_ENABLE_PLUGINS=ON
#CMAKE_FLAGS_LLVM += -DLLVM_STATIC_LINK_CXX_STDLIB=1
#CMAKE_FLAGS_LLVM += -DLLVM_USE_NEWPM=OFF
CMAKE_FLAGS_LLVM += -DCMAKE_EXPORT_COMPILE_COMMANDS=1
CMAKE_FLAGS_LLVM += -DCMAKE_CXX_COMPILER="clang++"
CMAKE_FLAGS_LLVM += -DCMAKE_C_COMPILER=clang
CMAKE_FLAGS_LLVM += -DCMAKE_C_COMPILER_LAUNCHER=ccache
CMAKE_FLAGS_LLVM += -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

#CMAKE_FLAGS_LLVM += -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON

#############################################################################

.PHONY: all
all:
	@echo BUILD_TYPE=$(BUILD_TYPE)
	@echo BUILD_JOBS=$(BUILD_JOBS)
	@echo LINK_JOBS=$(LINK_JOBS)
	@echo CMAKE_GENERATOR=$(CMAKE_GENERATOR)

.PHONY: configure-build
configure-build:
	$(MKDIR) $(BUILDDIR_LLVM)
	$(CMAKE) $(CMAKE_FLAGS_LLVM)

.PHONY: build
build:
	$(CMAKE) --build $(BUILDDIR_LLVM)

.PHONY: install
install:
	$(CMAKE) --build $(BUILDDIR_LLVM) --target install

.PHONY: build-llc
build-llc:
	$(CMAKE) --build $(BUILDDIR_LLVM) --target llc

.PHONY: install-llc
install-llc:
	$(CMAKE) --build $(BUILDDIR_LLVM) --target llc
	$(CP) $(BUILDDIR_LLVM)/bin/llc $(INSTALLDIR)/bin/llc
