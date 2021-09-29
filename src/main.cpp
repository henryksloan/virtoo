#include "kvm.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string_view>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

void exit_if_error(absl::Status status) {
    if (!status.ok()) {
        std::cerr << status << std::endl;
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [kernel image path]" << std::endl;
        exit(-1);
    }

    absl::StatusOr<Kvm> kvm = Kvm::Create();
    exit_if_error(kvm.status());

    absl::StatusOr<Vm> vm = kvm->CreateVm();
    exit_if_error(vm.status());

    absl::StatusOr<Vcpu> vcpu = vm->CreateVcpu();
    exit_if_error(vcpu.status());

    std::filesystem::path path(argv[1]);
    absl::Status load_image_result = vm->LoadKernelImage(path);
    exit_if_error(load_image_result);

    return 0;
}
