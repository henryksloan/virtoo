#ifndef VIRTOO_KVM_H
#define VIRTOO_KVM_H

#include "file_handle.h"
#include "vm.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"

#include <linux/kvm.h>
#include <memory>

const char *const kKvmDevice = "/dev/kvm";

class Kvm {
 public:
    static absl::StatusOr<Kvm> Create();

    absl::StatusOr<Vm> CreateVm() const;

    int GetVcpuMapSize() const {
        return this->vcpu_map_size;
    }

 private:
    FileHandle kvm_fd;
    int vcpu_map_size;

    Kvm(FileHandle &kvm_fd) : kvm_fd(std::move(kvm_fd)) {
        this->vcpu_map_size = this->kvm_fd.ioctl(KVM_GET_VCPU_MMAP_SIZE, nullptr);
    }
};

#endif
