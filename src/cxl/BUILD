load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "config",
    srcs = ["config.cxx"],
    hdrs = ["config.hxx"],
    deps = [
        "//3rdparty/winreg",
        "//src/rcl/rclt:util",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "unit",
    srcs = ["unit.cxx"],
    hdrs = ["unit.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/wink",
        "//src/ral/raldsp:filter",
        "//src/ral/raldsp:mixer",
        "//src/ral/raldsp:ratereducer",
        "//src/ral/raldsp:sampler",
        "//src/ral/raldsp:syncdelay",
        "//src/ral/ralm:grid_sequencer",
        "//src/ral/ralw:wavetable",
        "//src/rcl/rclmt:reactor_file",
        "//src/rcl/rcls:file",
        ":config",
        ":log",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_binary(
    name = "cxl",
    srcs = ["main.cxx"],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/wink",
        "//src/cxl/ui/root:view",
        "//src/ral/ralio:asio",
        "//src/rcl/rclmt:reactor",
        "//src/rcl/rclw:console",
        ":config",
        ":log",
        ":unit",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "log",
    srcs = ["log.cxx"],
    hdrs = ["log.hxx"],
    deps = [
        "//3rdparty/wink",
        "//src/rcl/rcls:file",
        ":config",
        ],
    copts = RSR_DEFAULT_COPTS,)
