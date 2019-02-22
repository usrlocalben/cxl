load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "guid",
    srcs = ["rclw_guid.cxx"],
    hdrs = ["rclw_guid.hxx"],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ['-DEFAULTLIB:ole32.lib'],
    )

cc_library(
    name = "io",
    srcs = ["rclw_io.cxx"],
    hdrs = ["rclw_io.hxx"],
    deps=[
        "//src/rcl/rclt:util",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "smarti",
    srcs = ["rclw_smarti.cxx"],
    hdrs = ["rclw_smarti.hxx"],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "console",
    srcs = ["rclw_console.cxx"],
    hdrs = ["rclw_console.hxx"],
    deps=[
        "//src/rcl/rclt:util",
        ":console_canvas",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name = "console_canvas",
    srcs = ["rclw_console_canvas.cxx"],
    hdrs = ["rclw_console_canvas.hxx"],
    deps=[],
    copts = RSR_DEFAULT_COPTS,
    )
