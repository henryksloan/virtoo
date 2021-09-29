#ifndef VIRTOO_VM_H
#define VIRTOO_VM_H

#include "vcpu.h"
#include "file_handle.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"

#include <filesystem>
#include <memory>
#include <sys/mman.h>

const size_t kMemMapLength = 1 << 30;

class Vm {
 public:
    static absl::StatusOr<Vm> Create(const int vm_fd_num);

    Vm(Vm &&other) : vm_fd(std::move(other.vm_fd)) {
        this->mem = other.mem;
        other.mem = nullptr;
    }

    ~Vm() {
        munmap(this->mem, kMemMapLength);
    }

    Vm(const Vm &) = delete;
    Vm &operator=(const Vm &) = delete;
    Vm &operator=(Vm &&) = delete;

    absl::StatusOr<Vcpu> CreateVcpu() const;
    absl::Status LoadKernelImage(std::filesystem::path path) const;

 private:
    FileHandle vm_fd;
    void *mem;

    Vm(const int vm_fd_num) : vm_fd(vm_fd_num) {}

    absl::Status Init();
};

#endif
