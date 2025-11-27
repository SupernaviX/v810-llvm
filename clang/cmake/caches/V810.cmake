# MOS.cmake
# Configure llvm-mos for building a distribution
# Usage for configuring:
#   cmake -C [path-to-this-file] ...

set(LLVM_TARGETS_TO_BUILD ARM;X86;V810 CACHE STRING "")
set(LLVM_ENABLE_PROJECTS clang;clang-tools-extra;lld;lldb CACHE STRING "")
set(LLVM_ENABLE_LIBXML2 "OFF" CACHE STRING "")
set(LLVM_ENABLE_ZLIB "OFF" CACHE STRING "")
set(LLVM_ENABLE_ZSTD "OFF" CACHE STRING "")

set(LLVM_ENABLE_RUNTIMES compiler-rt CACHE STRING "")

set(LLVM_BUILTIN_TARGETS v810-unknown-vb CACHE STRING "")
set(LLVM_RUNTIME_TARGETS v810-unknown-vb CACHE STRING "")
set(BUILTINS_v810-unknown-vb_COMPILER_RT_BAREMETAL_BUILD ON CACHE BOOL "")
set(BUILTINS_v810-unknown-vb_COMPILER_RT_BUILTINS_ENABLE_PIC OFF CACHE BOOL "")
set(BUILTINS_v810-unknown-vb_COMPILER_RT_BUILD_BUILTINS ON CACHE BOOL "")
set(BUILTINS_v810-unknown-vb_COMPILER_RT_BUILD_CRT OFF CACHE BOOL "")
set(BUILTINS_v810-unknown-vb_CMAKE_BUILD_TYPE Release CACHE BOOL "")

set(LLVM_DEFAULT_TARGET_TRIPLE v810-unknown-vb CACHE STRING "")

set(CMAKE_BUILD_TYPE Release CACHE STRING "CMake build type")

# disable lldb testing until the lldb tests stabilize
set(LLDB_INCLUDE_TESTS OFF CACHE BOOL "Include lldb tests")

# Ship the release with these tools
set(LLVM_INSTALL_TOOLCHAIN_ONLY ON CACHE BOOL "")
set(LLVM_TOOLCHAIN_TOOLS
  llvm-addr2line
  llvm-ar
  llvm-cxxfilt
  llvm-dwarfdump
  llvm-mc
  llvm-nm
  llvm-objcopy
  llvm-objdump
  llvm-ranlib
  llvm-readelf
  llvm-readobj
  llvm-size
  llvm-strings
  llvm-strip
  llvm-symbolizer CACHE STRING "")

set(LLVM_DISTRIBUTION_COMPONENTS
  builtins
  clang
  lld
  lldb
  liblldb
  lldb-argdumper
  lldb-dap
  lldb-server
  clang-apply-replacements
  clang-format
  clang-resource-headers
  clang-include-fixer
  clang-refactor
  clang-scan-deps
  clang-tidy
  clangd
  find-all-symbols
  ${LLVM_TOOLCHAIN_TOOLS}
  CACHE STRING "")

# Add clang symlinks prefixed with v810-* to allow distinguishing a llvm-v810
# directory from a system clang directory.
set(CLANG_LINKS_TO_CREATE
  clang++
  clang-cpp
  v810-clang
  v810-clang++
  v810-clang-cpp)
set(CLANG_LINKS_TO_CREATE ${CLANG_LINKS_TO_CREATE}
  CACHE STRING "Clang symlinks to create during install.")
