#ifndef VIRTOO_VM_H
#define VIRTOO_VM_H

#include "vcpu.h"
#include "file_handle.h"
#include "kvm_cpuid.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"

#include <filesystem>
#include <memory>
#include <sys/mman.h>

const size_t kMemMapLength = 1 << 30;

class Vm {
 public:
    static absl::StatusOr<Vm> Create(const int vm_fd_num, const int vcpu_map_size, const KvmCpuId kvm_cpuid);

    Vm(Vm &&other)
        : vm_fd(std::move(other.vm_fd)),
          vcpu_map_size(other.vcpu_map_size),
          kvm_cpuid(other.kvm_cpuid) {
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
    const int vcpu_map_size;
    KvmCpuId kvm_cpuid;

    Vm(const int vm_fd_num, const int vcpu_map_size, KvmCpuId kvm_cpuid)
        : vm_fd(vm_fd_num), vcpu_map_size(vcpu_map_size), kvm_cpuid(kvm_cpuid) {}

    absl::Status Init();
};

#endif
