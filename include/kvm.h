#ifndef VIRTOO_KVM_H
#define VIRTOO_KVM_H

#include "file_handle.h"
#include "vm.h"

#include "absl/status/statusor.h"

#include <memory>

const char *const kKvmDevice = "/dev/kvm";

class Kvm {
 public:
    static absl::StatusOr<Kvm> Create();

    absl::StatusOr<Vm> CreateVm() const;

 private:
    FileHandle kvm_fd;

    Kvm(FileHandle &kvm_fd) : kvm_fd(std::move(kvm_fd)) {}
};

#endif
