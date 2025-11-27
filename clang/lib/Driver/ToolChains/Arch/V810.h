#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_V810_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_V810_H

#include "clang/Driver/Driver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"
#include <string>
#include <vector>

namespace clang {
namespace driver {
namespace tools {
namespace v810 {

std::string getV810TargetCPU(const Driver &D, const llvm::opt::ArgList &Args,
                             const llvm::Triple &Triple);

void getV810TargetFeatures(const Driver &D, const llvm::opt::ArgList &Args,
                           std::vector<llvm::StringRef> &Features);
}
}
}
}

#endif