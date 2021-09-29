#include "vm.h"

#include <asm/bootparam.h>
#include <linux/kvm.h>
#include <sys/stat.h>

absl::StatusOr<Vm> Vm::Create(const int vm_fd_num, const int vcpu_map_size, KvmCpuId &kvm_cpuid) {
    Vm vm(vm_fd_num, vcpu_map_size, kvm_cpuid);

    absl::Status init_status = vm.Init();
    if (init_status.ok()) {
        return vm;
    } else {
        return init_status;
    }
}

absl::StatusOr<Vcpu> Vm::CreateVcpu() const {
    int vcpu_fd = this->vm_fd.ioctl(KVM_CREATE_VCPU, nullptr);
    if (vcpu_fd < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to create VCPU"));
    } else {
        return Vcpu::Create(vcpu_fd, this->vcpu_map_size, this->kvm_cpuid);
    }
}

absl::Status Vm::LoadKernelImage(std::filesystem::path image_path) const {
    size_t datasz;
    void *data;
    int fd = open(image_path.string().c_str(), O_RDONLY);
    if (fd < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to open kernel image"));
    }

    struct stat st;
    fstat(fd, &st);
    data = mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    datasz = st.st_size;
    close(fd);

    struct boot_params *boot =
        (struct boot_params *)(((uint8_t *) this->mem) + 0x10000);
    void *cmdline = (void *)(((uint8_t *) this->mem) + 0x20000);
    void *kernel = (void *)(((uint8_t *) this->mem) + 0x100000);

    memset(boot, 0, sizeof(struct boot_params));
    memmove(boot, data, sizeof(struct boot_params));
    size_t setup_sectors = boot->hdr.setup_sects;
    size_t setupsz = (setup_sectors + 1) * 512;
    boot->hdr.vid_mode = 0xFFFF; // VGA
    boot->hdr.type_of_loader = 0xFF;
    boot->hdr.ramdisk_image = 0x0;
    boot->hdr.ramdisk_size = 0x0;
    boot->hdr.loadflags |= CAN_USE_HEAP | 0x01 | KEEP_SEGMENTS;
    boot->hdr.heap_end_ptr = 0xFE00;
    boot->hdr.ext_loader_ver = 0x0;
    boot->hdr.cmd_line_ptr = 0x20000;
    memset(cmdline, 0, boot->hdr.cmdline_size);
    memcpy(cmdline, "console=ttyS0", 14);
    memmove(kernel, (char *)data + setupsz, datasz - setupsz);

    return absl::OkStatus();
}

absl::Status Vm::Init() {
    // Set the GPA of the task state segment (TSS)
    int set_tss_result = this->vm_fd.ioctl(
        KVM_SET_TSS_ADDR,
        reinterpret_cast<void *>(0xffffd000));
    if (set_tss_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to set TSS address"));
    }

    // Set the GPA of the identity map
    // TODO: Can this also be a reinterpret cast?
    __u64 map_addr = 0xffffc000;
    int set_identity_map_result = this->vm_fd.ioctl(
        KVM_SET_IDENTITY_MAP_ADDR,
        &map_addr);
    if (set_identity_map_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to set identity map address"));
    }

    // Initialize the virtual interrupt controller
    int create_irqchip_result = this->vm_fd.ioctl(KVM_CREATE_IRQCHIP, nullptr);
    if (create_irqchip_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to create IRQ chip"));
    }

    // Configure and create the programmable interval timer
    struct kvm_pit_config pit = {
      .flags = 0,
    };
    int create_pit_result = this->vm_fd.ioctl(KVM_CREATE_PIT2, &pit);
    if (create_pit_result < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to create PIT"));
    }

    // Map a region of memory for the VM
    this->mem = mmap(NULL, kMemMapLength, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (this->mem == NULL) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to map memory region"));
    }

    // Configure the userspace region of the memory map
    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0,
        .memory_size = 1 << 30,
        .userspace_addr = (__u64) this->mem,
    };
    if (this->vm_fd.ioctl(KVM_SET_USER_MEMORY_REGION, &region) < 0) {
        return absl::FailedPreconditionError(
            absl::StrCat("failed to configure userspace memory region"));
    }

    return absl::OkStatus();
}
