load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "state",
    srcs = ["state.cxx"],
    hdrs = ["state.hxx"],
    deps = [],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "view",
    srcs = ["view.cxx"],
    hdrs = ["view.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/cxl/ui/host:view",
        "//src/cxl/ui/log:view",
        "//src/cxl/ui/pattern:view",
        "//src/cxl/ui/splash:view",
        "//src/cxl:unit",
        "//src/rcl/rcls:text_canvas",
        "//src/textkit:mainloop",
        "//src/textkit:widget",
        ":state",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "controller",
    srcs = ["controller.cxx"],
    hdrs = ["controller.hxx"],
    deps = [
        "//src/cxl/ui/host:controller",
        "//src/cxl/ui/loading:view",
        "//src/cxl/ui/log:controller",
        "//src/cxl/ui/pattern:controller",
        "//src/cxl/ui/splash:controller",
        "//src/cxl:host",
        "//src/cxl:log",
        "//src/cxl:unit",
        "//src/rcl/rclmt:event",
        "//src/rcl/rclmt:reactor",
        "//src/rcl/rclmt:reactor_timer",
        "//src/rcl/rcls:text_canvas",
        "//src/textkit:keyevent",
        "//src/textkit:mainloop",
        "//src/textkit:widget",
        ":state",
        ":view",
        ],
    copts = RSR_DEFAULT_COPTS,)
