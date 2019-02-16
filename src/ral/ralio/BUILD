load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name="asio",
    srcs=["ralio_asio.cxx"],
    hdrs=["ralio_asio.hxx"],
    deps=[
        "//3rdparty/asiosdk",
        "//3rdparty/winreg",
        "//src/rcl/rclt:util",
        "//src/rcl/rclw:guid",
        "//src/rcl/rclw:io",
        "//src/rcl/rclw:smarti",
        ],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ['-DEFAULTLIB:ole32.lib'],
    )
