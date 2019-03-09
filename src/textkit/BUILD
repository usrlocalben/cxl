load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mainloop",
    srcs = ["mainloop.cxx"],
    hdrs = ["mainloop.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclmt:deferred",
        "//src/rcl/rclmt:event",
        "//src/rcl/rclmt:reactor",
        ":keyevent",
        ":widget",
        ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "widget",
    srcs = ["widget.cxx"],
    hdrs = ["widget.hxx"],
    deps = [
        "//src/rcl/rcls:text_canvas",
        ":keyevent",
        ],
    copts = RSR_DEFAULT_COPTS,)

cc_library(
    name = "keyevent",
    srcs = ["keyevent.cxx"],
    hdrs = ["keyevent.hxx"],
    deps = [
        "//src/rcl/rclmt:reactor_keyboard",
        ],
    copts = RSR_DEFAULT_COPTS,)
