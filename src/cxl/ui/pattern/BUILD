load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "state",
    srcs = ["state.cxx"],
    hdrs = ["state.hxx"],
    deps = [],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "page",
    srcs = ["page.cxx"],
    hdrs = ["page.hxx"],
    deps = [
        "//src/cxl:unit", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "view",
    srcs = ["view.cxx"],
    hdrs = ["view.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclmt:event",
        "//src/rcl/rclmt:reactor",
        "//src/rcl/rclmt:reactor_timer",
        "//src/rcl/rcls:console",
        "//src/rcl/rcls:text_canvas",
        "//src/textkit:keyevent",
        "//src/textkit:mainloop",
        "//src/textkit:widget",
        "//src/cxl:log",
        "//src/cxl:unit",
        ":state",
        ":page",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "controller",
    srcs = ["controller.cxx"],
    hdrs = ["controller.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/cxl/ui/alert:view",
        "//src/cxl/ui/pattern_length_edit:controller",
        "//src/cxl:log",
        "//src/cxl:unit",
        "//src/rcl/rclmt:event",
        "//src/rcl/rclmt:reactor",
        "//src/rcl/rclmt:reactor_timer",
        "//src/rcl/rcls:console",
        "//src/rcl/rcls:text_canvas",
        "//src/textkit:keyevent",
        "//src/textkit:mainloop",
        "//src/textkit:widget",
        ":state",
        ":page",
        ":view",
        ],
    copts = RSR_DEFAULT_COPTS,)
