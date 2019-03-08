load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "view",
    srcs = ["view.cxx"],
    hdrs = ["view.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/cxl/ui/alert:view",
        "//src/cxl/ui/pattern_length_edit:view",
        "//src/cxl:log",
        "//src/cxl:unit",
        "//src/rcl/rclmt:event",
        "//src/rcl/rclmt:reactor",
        "//src/rcl/rclmt:reactor_delay",
        "//src/rcl/rclw:console",
        "//src/rcl/rclw:console_canvas",
        "//src/textkit:keyevent",
        "//src/textkit:mainloop",
        "//src/textkit:widget",
        ],
    copts = RSR_DEFAULT_COPTS,)
