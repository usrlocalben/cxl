load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "reactor",
    srcs = ["cxl_reactor.cxx"],
    hdrs = ["cxl_reactor.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclt:util",
        "//src/rcl/rclw:winfile",
        ":widget",
        ":log",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "widget",
    srcs = ["cxl_widget.cxx"],
    hdrs = ["cxl_widget.hxx"],
    deps = [
        "//src/rcl/rclw:console_canvas",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "config",
    srcs = ["cxl_config.cxx"],
    hdrs = ["cxl_config.hxx"],
    deps = [
        "//3rdparty/winreg",
        "//src/rcl/rclt:util",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "unit",
    srcs = ["cxl_unit.cxx"],
    hdrs = ["cxl_unit.hxx"],
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
        "//src/rcl/rcls:file",
        ":config",
        ":log",
        ":reactor",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_binary(
    name = "cxl",
    srcs = ["cxl.m.cxx"],
    deps = [
        "//3rdparty/fmt",
        "//3rdparty/wink",
        "//src/ral/ralio:asio",
        "//src/rcl/rclw:console",
        "//src/cxl/ui:root",
        ":config",
        ":log",
        ":reactor",
        ":unit",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "log",
    srcs = ["cxl_log.cxx"],
    hdrs = ["cxl_log.hxx"],
    deps = [
        "//3rdparty/wink",
        "//src/rcl/rcls:file",
        ":config",
        ],
    copts = RSR_DEFAULT_COPTS,)
