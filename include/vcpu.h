#ifndef VIRTOO_VCPU_H
#define VIRTOO_VCPU_H

#include "file_handle.h"

#include "absl/status/statusor.h"
#include "absl/status/status.h"

#include <linux/kvm.h>
#include <sys/mman.h>

class Vcpu {
 public:
    static absl::StatusOr<Vcpu> Create(const int vcpu_fd_num, const int vcpu_map_size);

 private:
    FileHandle vcpu_fd;
    struct kvm_run *run;

    Vcpu(const int vcpu_fd_num, const int vcpu_map_size) : vcpu_fd(vcpu_fd_num) {
        this->run = static_cast<struct kvm_run *>(mmap(0, vcpu_map_size, PROT_READ | PROT_WRITE, MAP_SHARED, this->vcpu_fd.getRaw(), 0));
    }

    absl::Status InitSpecialRegisters() const;
    absl::Status InitRegisters() const;
    absl::Status InitCpuId() const;
};

#endif
