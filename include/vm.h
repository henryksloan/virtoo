#ifndef VIRTOO_VM_H
#define VIRTOO_VM_H

#include "vcpu.h"
#include "file_handle.h"

#include "absl/status/statusor.h"

#include <memory>

class Vm {
 public:
    static absl::StatusOr<Vm> Create(const int vm_fd_num);

    absl::StatusOr<Vcpu> CreateVcpu() const;

 private:
    FileHandle vm_fd;
    void *mem;

    Vm(const int vm_fd_num) : vm_fd(vm_fd_num) {}

    bool Init();
};

#endif
