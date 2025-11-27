#ifndef LLDB_SOURCE_PLUGINS_ABI_V810_ABIV810_H
#define LLDB_SOURCE_PLUGINS_ABI_V810_ABIV810_H

#include "lldb/Target/ABI.h"
#include "lldb/lldb-private.h"

class ABIV810 : public lldb_private::RegInfoBasedABI {
public:
  ~ABIV810() override = default;

  size_t GetRedZoneSize() const override { return 0; }

  bool PrepareTrivialCall(lldb_private::Thread &thread, lldb::addr_t sp,
                          lldb::addr_t functionAddress,
                          lldb::addr_t returnAddress,
                          llvm::ArrayRef<lldb::addr_t> args) const override;

  bool GetArgumentValues(lldb_private::Thread &thread,
                         lldb_private::ValueList &values) const override;

  lldb_private::Status
  SetReturnValueObject(lldb::StackFrameSP &frame_sp,
                       lldb::ValueObjectSP &new_value) override;

  lldb::ValueObjectSP
  GetReturnValueObjectImpl(lldb_private::Thread &thread,
                           lldb_private::CompilerType &type) const override;

  lldb::UnwindPlanSP
  CreateFunctionEntryUnwindPlan() override;

  lldb::UnwindPlanSP
  CreateDefaultUnwindPlan() override;

  bool RegisterIsVolatile(const lldb_private::RegisterInfo *reg_info) override {
    return false;
  }

  bool CallFrameAddressIsValid(lldb::addr_t cfa) override {
    return true;
  }

  bool CodeAddressIsValid(lldb::addr_t pc) override {
    return !(pc & 0xffffffff00000000);
  }

  const lldb_private::RegisterInfo *
  GetRegisterInfoArray(uint32_t &count) override;

  static void Initialize();
  static void Terminate();
  static lldb::ABISP CreateInstance(lldb::ProcessSP process_sp, const lldb_private::ArchSpec &arch);

  static llvm::StringRef GetPluginNameStatic() { return "v810"; }
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
private:
  using lldb_private::RegInfoBasedABI::RegInfoBasedABI;
};

#endif