#include "kvm.h"

#include <linux/kvm.h>
#include <iostream>
#include <cstdlib>

std::unique_ptr<Kvm> Kvm::Create() {
    FileHandle kvm_fd(kKvmDevice, O_RDWR);
    if (!kvm_fd.is_valid()) {
        return std::unique_ptr<Kvm>(nullptr);
    } else {
        return std::unique_ptr<Kvm>(new Kvm(kvm_fd));
    }
}

std::unique_ptr<Vm> Kvm::CreateVm() const {
    int vm_fd = this->kvm_fd.ioctl(KVM_CREATE_VM, nullptr);
    if (vm_fd < 0) {
        return std::unique_ptr<Vm>(nullptr);
    } else {
        return Vm::Create(vm_fd);
    }
}
