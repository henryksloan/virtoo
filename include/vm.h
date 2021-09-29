#ifndef VIRTOO_VM_H
#define VIRTOO_VM_H

#include "vcpu.h"
#include "file_handle.h"

#include <memory>

class Vm {
 public:
    Vm(int &vm_fd_num) : vm_fd(vm_fd_num) {}

    std::unique_ptr<Vcpu> CreateVcpu() const;

 private:
    FileHandle vm_fd;
};

#endif
