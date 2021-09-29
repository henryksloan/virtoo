#ifndef VIRTOO_VCPU_H
#define VIRTOO_VCPU_H

#include "file_handle.h"

class Vcpu {
 public:
    Vcpu(int &vcpu_fd_num) : vcpu_fd(vcpu_fd_num) {}

 private:
    FileHandle vcpu_fd;
};

#endif
