load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "reactor",
    srcs = ["cxl_reactor.cxx"],
    hdrs = ["cxl_reactor.hxx"],
    deps = [
        ":widget",
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
        "//src/ral/raldsp:mixer",
        "//src/ral/raldsp:sampler",
        "//src/ral/ralm:grid_sequencer",
        "//src/ral/ralw:wavetable",
        "//src/rcl/rcls:file",
        ":config",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "controller",
    srcs = ["cxl_controller.cxx"],
    hdrs = ["cxl_controller.hxx"],
    deps = [
        "//src/rcl/rclw:console",
        ":reactor",
        ":unit",
        ":widget",
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
        ":config",
        ":controller",
        ":reactor",
        ":unit",
        ],
    copts = RSR_DEFAULT_COPTS,)
