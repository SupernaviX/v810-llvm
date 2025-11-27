#include "V810.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"
#include "llvm/TargetParser/Host.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

std::string v810::getV810TargetCPU(const Driver &D, const ArgList &Args,
                                   const llvm::Triple &Triple) {

  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_march_EQ)) {
    D.Diag(diag::err_drv_unsupported_opt_for_target)
        << A->getSpelling() << Triple.getTriple();
    return "";
  }

  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_mcpu_EQ)) {
    StringRef CPUName = A->getValue();
    return std::string(CPUName);
  }

  return "";

}

void v810::getV810TargetFeatures(const Driver &D, const ArgList &Args,
                                 std::vector<StringRef> &Features) {
  // Mark these inputs as used to make some tests happy
  Args.claimAllArgs(options::OPT_fintegrated_as, options::OPT_fno_integrated_as);
  if (Arg *A = Args.getLastArg(options::OPT_mgprel, options::OPT_mno_gprel)) {
    if (A->getOption().matches(options::OPT_mgprel))
      Features.push_back("+gprel");
    else
      Features.push_back("-gprel");
  }
  if (Arg *A = Args.getLastArg(options::OPT_mapp_regs, options::OPT_mno_app_regs)) {
    if (A->getOption().matches(options::OPT_mapp_regs))
      Features.push_back("+app-regs");
    else
      Features.push_back("-app-regs");
  }
}