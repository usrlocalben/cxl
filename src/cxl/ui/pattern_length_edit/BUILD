load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "view",
    srcs = ["view.cxx"],
    hdrs = ["view.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/textkit:keyevent",
        "//src/textkit:widget",
        "//src/rcl/rclw:console",
        ],
    copts = RSR_DEFAULT_COPTS,)
