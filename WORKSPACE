workspace(name = "clojure_jni_project")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_jni",
    url = "https://github.com/fmeum/rules_jni/archive/refs/tags/v0.4.0.zip",
    strip_prefix = "rules_jni-0.4.0",
)

RULES_CLOJURE_SHA = "13d76465c3ed0a23d2d2515c8138cdde59cf8bd1"
http_archive(name = "rules_clojure",
             strip_prefix = "rules_clojure-%s" % RULES_CLOJURE_SHA,
             url = "https://github.com/griffinbank/rules_clojure/archive/%s.zip" % RULES_CLOJURE_SHA)

load("@rules_clojure//:repositories.bzl", "rules_clojure_deps")
rules_clojure_deps()

load("@rules_clojure//:setup.bzl", "rules_clojure_setup")
rules_clojure_setup()

http_archive(
    name = "rules_cc",
    url = "https://github.com/bazelbuild/rules_cc/archive/main.zip",
    strip_prefix = "rules_cc-main",
)

new_local_repository(
    name = "postgresql",
    build_file_content = """
cc_library(
    name = "postgresql",
    hdrs = glob(["include/postgresql@14/server/**/*.h"]),
    includes = ["include/postgresql@14/server"],
    strip_include_prefix = "include/postgresql@14/server",
    visibility = ["//visibility:public"],
)
""",
    path = "/opt/homebrew/opt/postgresql@14",
)