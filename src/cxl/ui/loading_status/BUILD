load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "view",
    srcs = ["view.cxx"],
    hdrs = ["view.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/cxl:unit",
        "//src/rcl/rclmt:reactor",
        "//src/rcl/rclw:console",
        "//src/textkit:keyevent",
        "//src/textkit:widget",
        ],
    copts = RSR_DEFAULT_COPTS,)
