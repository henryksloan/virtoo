#ifndef VIRTOO_VCPU_H
#define VIRTOO_VCPU_H

#include "file_handle.h"

#include "absl/status/statusor.h"
#include "absl/status/status.h"

#include <linux/kvm.h>

class Vcpu {
 public:
    static absl::StatusOr<Vcpu> Create(const int vcpu_fd_num);

 private:
    FileHandle vcpu_fd;

    Vcpu(const int vcpu_fd_num) : vcpu_fd(vcpu_fd_num) {}

    absl::Status InitSpecialRegisters() const;
    absl::Status InitRegisters() const;
    absl::Status InitCpuId() const;
};

#endif
