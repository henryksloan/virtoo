#include "vcpu.h"

#include <linux/kvm_para.h>

absl::StatusOr<Vcpu> Vcpu::Create(const int vcpu_fd_num, const int vcpu_map_size) {
    Vcpu vcpu(vcpu_fd_num, vcpu_map_size);

    absl::Status special_reg_status = vcpu.InitSpecialRegisters();
    if (!special_reg_status.ok()) {
        return special_reg_status;
    }

    absl::Status reg_status = vcpu.InitRegisters();
    if (!reg_status.ok()) {
        return reg_status;
    }

    absl::Status cpuid_status = vcpu.InitCpuId();
    if (!cpuid_status.ok()) {
        return cpuid_status;
    }

    return vcpu;
}

absl::Status Vcpu::InitSpecialRegisters() const {
    struct kvm_sregs sregs;

    int get_sregs_result = this->vcpu_fd.ioctl(KVM_GET_SREGS, &(sregs));
    if (get_sregs_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to get special registers"));
    }

    sregs.cs.base = 0;
    sregs.cs.limit = ~0;
    sregs.cs.g = 1;

    sregs.ds.base = 0;
    sregs.ds.limit = ~0;
    sregs.ds.g = 1;

    sregs.fs.base = 0;
    sregs.fs.limit = ~0;
    sregs.fs.g = 1;

    sregs.gs.base = 0;
    sregs.gs.limit = ~0;
    sregs.gs.g = 1;

    sregs.es.base = 0;
    sregs.es.limit = ~0;
    sregs.es.g = 1;

    sregs.ss.base = 0;
    sregs.ss.limit = ~0;
    sregs.ss.g = 1;

    sregs.cs.db = 1;
    sregs.ss.db = 1;
    sregs.cr0 |= 1; /* enable protected mode */

    int set_sregs_result = this->vcpu_fd.ioctl(KVM_SET_SREGS, &(sregs));
    if (set_sregs_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to set special registers"));
    }

    return absl::OkStatus();
}

absl::Status Vcpu::InitRegisters() const {
    struct kvm_regs regs;

  int get_regs_result = this->vcpu_fd.ioctl(KVM_GET_REGS, &(regs));
  if (get_regs_result < 0) {
      return absl::FailedPreconditionError(
          absl::StrCat("failed to get registers"));
  }

  regs.rflags = 2;
  regs.rip = 0x100000;
  regs.rsi = 0x10000;

  int set_regs_result = this->vcpu_fd.ioctl(KVM_SET_REGS, &(regs));
  if (set_regs_result < 0) {
      return absl::FailedPreconditionError(
          absl::StrCat("failed to set registers"));
  }

  return absl::OkStatus();
}

absl::Status Vcpu::InitCpuId() const {
    struct {
        uint32_t nent;
        uint32_t padding;
        struct kvm_cpuid_entry2 entries[100];
    } kvm_cpuid;
    kvm_cpuid.nent = sizeof(kvm_cpuid.entries) / sizeof(kvm_cpuid.entries[0]);
    this->vcpu_fd.ioctl(KVM_GET_SUPPORTED_CPUID, &kvm_cpuid);

    for (unsigned int i = 0; i < kvm_cpuid.nent; i++) {
        struct kvm_cpuid_entry2 *entry = &kvm_cpuid.entries[i];
        if (entry->function == KVM_CPUID_SIGNATURE) {
            entry->eax = KVM_CPUID_FEATURES;
            entry->ebx = 0x4b4d564b; // KVMK
            entry->ecx = 0x564b4d56; // VMKV
            entry->edx = 0x4d;       // M
        }
    }

    this->vcpu_fd.ioctl(KVM_SET_CPUID2, &kvm_cpuid);

    return absl::OkStatus();
}
