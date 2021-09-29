#include "kvm.h"
#include "kvm_cpuid.h"

#include <linux/kvm.h>
#include <iostream>
#include <cstdlib>

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
        return absl::FailedPreconditionError(
            absl::StrCat("failed to create VM"));
    } else {
        struct KvmCpuId kvm_cpuid;
        kvm_cpuid.nent = sizeof(kvm_cpuid.entries) / sizeof(kvm_cpuid.entries[0]);
        this->kvm_fd.ioctl(KVM_GET_SUPPORTED_CPUID, &kvm_cpuid);

        return Vm::Create(vm_fd, this->GetVcpuMapSize(), kvm_cpuid);
    }
}
