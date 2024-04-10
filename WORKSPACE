load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Download and configure aspect_gcc_toolchain
http_archive(
    name = "aspect_gcc_toolchain",
    sha256 = "3341394b1376fb96a87ac3ca01c582f7f18e7dc5e16e8cf40880a31dd7ac0e1e",
    strip_prefix = "gcc-toolchain-0.4.2",
    urls = [
        "https://github.com/aspect-build/gcc-toolchain/archive/0.4.2.tar.gz",
    ],
)

load("@aspect_gcc_toolchain//toolchain:repositories.bzl", "gcc_toolchain_dependencies")

gcc_toolchain_dependencies()

load("@aspect_gcc_toolchain//toolchain:defs.bzl", "gcc_register_toolchain", "ARCHS")

gcc_register_toolchain(
    name = "gcc_toolchain_arm",
    target_arch = ARCHS.aarch64,  # Use aarch64 for ARM architecture
)

http_archive(
    name = "fmeum_rules_jni",
    sha256 = "9a387a066f683a8aac4d165917dc7fe15ec2a20931894a97e153a9caab6123ca",
    strip_prefix = "rules_jni-0.4.0",
    url = "https://github.com/fmeum/rules_jni/archive/refs/tags/v0.4.0.tar.gz",
)

load("@fmeum_rules_jni//jni:repositories.bzl", "rules_jni_dependencies")

rules_jni_dependencies()


http_archive(
    name = "rules_clojure",
    sha256 = "c841fbf94af331f0f8f02de788ca9981d7c73a10cec798d3be0dd4f79d1d627d",
    strip_prefix = "rules_clojure-c044cb8608a2c3180cbfee89e66bbeb604afb146",
    urls = ["https://github.com/simuons/rules_clojure/archive/c044cb8608a2c3180cbfee89e66bbeb604afb146.tar.gz"],
)

load("@rules_clojure//:repositories.bzl", "rules_clojure_dependencies", "rules_clojure_toolchains")

rules_clojure_dependencies()

rules_clojure_toolchains()
