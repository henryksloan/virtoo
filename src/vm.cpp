#include "vm.h"

#include <linux/kvm.h>
#include <sys/mman.h>

absl::StatusOr<Vm> Vm::Create(const int vm_fd_num) {
    Vm vm(vm_fd_num);

    absl::Status init_status = vm.Init();
    if (init_status.ok()) {
        return vm;
    } else {
        return init_status;
    }
}

absl::StatusOr<Vcpu> Vm::CreateVcpu() const {
    int vcpu_fd = this->vm_fd.ioctl(KVM_CREATE_VCPU, nullptr);
    if (vcpu_fd < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to create VCPU"));
    } else {
        return Vcpu(vcpu_fd);
    }
}

absl::Status Vm::Init() {
    // Set the GPA of the task state segment (TSS)
    int set_tss_result = this->vm_fd.ioctl(
        KVM_SET_TSS_ADDR,
        reinterpret_cast<void *>(0xffffd000));
    if (set_tss_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to set TSS address"));
    }

    // Set the GPA of the identity map
    // TODO: Can this also be a reinterpret cast?
    __u64 map_addr = 0xffffc000;
    int set_identity_map_result = this->vm_fd.ioctl(
        KVM_SET_IDENTITY_MAP_ADDR,
        &map_addr);
    if (set_identity_map_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to set identity map address"));
    }

    // Initialize the virtual interrupt controller
    int create_irqchip_result = this->vm_fd.ioctl(KVM_CREATE_IRQCHIP, nullptr);
    if (create_irqchip_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to create IRQ chip"));
    }

    // Configure and create the programmable interval timer
    struct kvm_pit_config pit = {
      .flags = 0,
    };
    int create_pit_result = this->vm_fd.ioctl(KVM_CREATE_PIT2, &pit);
    if (create_pit_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to create PIT"));
    }

    // Map a region of memory for the VM
    this->mem = mmap(NULL, 1 << 30, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (this->mem == NULL) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to map memory region"));
    }

    // Configure the userspace region of the memory map
    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0,
        .memory_size = 1 << 30,
        .userspace_addr = (__u64) this->mem,
    };
    if (this->vm_fd.ioctl(KVM_SET_USER_MEMORY_REGION, &region) < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to configure userspace memory region"));
    }

    return absl::OkStatus();
}
