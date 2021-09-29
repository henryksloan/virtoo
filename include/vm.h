#ifndef VIRTOO_VM_H
#define VIRTOO_VM_H

#include "vcpu.h"
#include "file_handle.h"

#include <memory>

class Vm {
 public:
    static std::unique_ptr<Vm> Create(const int vm_fd_num);

    std::unique_ptr<Vcpu> CreateVcpu() const;

 private:
    FileHandle vm_fd;
    void *mem;

    Vm(const int vm_fd_num) : vm_fd(vm_fd_num) {}

    bool Init();
};

#endif
