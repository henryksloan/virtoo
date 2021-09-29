#include "kvm.h"

#include <iostream>
#include <memory>
#include <string_view>

constexpr void exit_if(bool fail_condition, std::string_view verb) {
    if (fail_condition) {
        std::cerr << "failed to " << verb << std::endl;
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    std::unique_ptr<Kvm> kvm = Kvm::Create();
    exit_if(!kvm, "open /dev/kvm");

    std::unique_ptr<Vm> vm = kvm->CreateVm();
    exit_if(!vm, "create vm");

    std::unique_ptr<Vcpu> vcpu = vm->CreateVcpu();
    exit_if(!vcpu, "create vcpu");

    return 0;
}
