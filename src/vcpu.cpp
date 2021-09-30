#include "vcpu.h"

#include <iostream>

absl::StatusOr<Vcpu> Vcpu::Create(const int vcpu_fd_num, const int vcpu_map_size, const KvmCpuId kvm_cpuid) {
    Vcpu vcpu(vcpu_fd_num, vcpu_map_size);
    vcpu.InitCpuId(kvm_cpuid);

    absl::Status special_reg_status = vcpu.InitSpecialRegisters();
    if (!special_reg_status.ok()) {
        return special_reg_status;
    }

    absl::Status reg_status = vcpu.InitRegisters();
    if (!reg_status.ok()) {
        return reg_status;
    }

    return vcpu;
}

int Vcpu::RunLoop() const {
    while (true) {
        int ret = this->vcpu_fd.ioctl(KVM_RUN, nullptr);
        if (ret < 0) {
            std::cerr << "kvm_run failed" << std::endl;
            return -1;
        }

        switch (run->exit_reason) {
        case KVM_EXIT_IO:
          if (run->io.port == 0x3f8 && run->io.direction == KVM_EXIT_IO_OUT) {
              uint32_t size = run->io.size;
              uint64_t offset = run->io.data_offset;
              printf("%.*s", size * run->io.count, (char *)run + offset);
          } else if (run->io.port == 0x3f8 + 5 &&
                     run->io.direction == KVM_EXIT_IO_IN) {
              char *value = (char *)run + run->io.data_offset;
              *value = 0x20;
          }
          break;
        case KVM_EXIT_SHUTDOWN:
            printf("shutdown\n");
            return 0;
        default:
            printf("reason: %d\n", run->exit_reason);
            return -1;
        }
    }
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

void Vcpu::InitCpuId(KvmCpuId kvm_cpuid) const {
    this->vcpu_fd.ioctl(KVM_SET_CPUID2, &kvm_cpuid);
}
