#include "kvm.h"

#include <linux/kvm.h>
#include <linux/kvm_para.h>

#include <cstdlib>
#include <iostream>

absl::StatusOr<Kvm> Kvm::Create() {
    FileHandle kvm_fd(kKvmDevice, O_RDWR);
    if (!kvm_fd.IsValid()) {
        return absl::FailedPreconditionError(
          absl::StrCat("failed to open ", kKvmDevice));
    } else {
        return Kvm(kvm_fd);
    }
}

absl::StatusOr<Vm> Kvm::CreateVm() const {
    int vm_fd = this->kvm_fd.ioctl(KVM_CREATE_VM, nullptr);
    if (vm_fd < 0) {
        return absl::FailedPreconditionError(absl::StrCat("failed to create VM"));
    } else {
        return Vm::Create(vm_fd, this->GetVcpuMapSize(), this->kvm_cpuid);
    }
}

void Kvm::InitCpuId() {
    this->kvm_cpuid.nent =
      sizeof(kvm_cpuid.entries) / sizeof(kvm_cpuid.entries[0]);
    this->kvm_fd.ioctl(KVM_GET_SUPPORTED_CPUID, &this->kvm_cpuid);

    for (unsigned int i = 0; i < kvm_cpuid.nent; i++) {
        struct kvm_cpuid_entry2 *entry = &kvm_cpuid.entries[i];

        switch (entry->function) {
        case KVM_CPUID_SIGNATURE:
            entry->eax = KVM_CPUID_FEATURES;
            entry->ebx = 0x4b4d564b; // KVMK
            entry->ecx = 0x564b4d56; // VMKV
            entry->edx = 0x4d;       // M
            break;
        case KVM_CPUID_FEATURES:
            entry->eax &= ~(1 << 14); // KVM_FEATURE_ASYNC_PF_INT_BIT)
            entry->ecx |= 1 << 24;    // TSC_DEADLINE_TIMER_ECX_BIT
            entry->ecx |= 1 << 31;    // HYPERVISOR_ECX_BIT
            entry->edx |= 1 << 12;    // MTRR_EDX_BIT
            break;
        case 0x80000008:
            entry->eax = (entry->eax & 0xffffff00) | (46 & 0xff);
            break;
        }
    }
}
