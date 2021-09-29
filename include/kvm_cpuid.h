#ifndef VIRTOO_KVM_CPUID
#define VIRTOO_KVM_CPUID

#include <linux/kvm.h>
#include <cstdint>

struct KvmCpuId {
    uint32_t nent;
    uint32_t padding;
    struct kvm_cpuid_entry2 entries[100];
};

#endif
