#include "vm.h"

#include <linux/kvm.h>

std::unique_ptr<Vcpu> Vm::CreateVcpu() const {
    int vcpu_fd = this->vm_fd.ioctl(KVM_CREATE_VCPU, nullptr);
    if (vcpu_fd < 0) {
        return std::unique_ptr<Vcpu>(nullptr);
    } else {
        return std::make_unique<Vcpu>(vcpu_fd);
    }
}
