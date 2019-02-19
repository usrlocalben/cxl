load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "reactor",
    srcs = ["cxl_reactor.cxx"],
    hdrs = ["cxl_reactor.hxx"],
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
        "//src/ral/raldsp:mixer",
        "//src/ral/raldsp:sampler",
        "//src/ral/ralm:grid_sequencer",
        "//src/ral/ralw:wavetable",
        "//src/rcl/rcls:file",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "unit_view",
    srcs = ["cxl_unit_view.cxx"],
    hdrs = ["cxl_unit_view.hxx"],
    deps = [
        "//src/rcl/rclw:console",
        ":unit",
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
        ":unit_view",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_binary(
    name = "cxl",
    srcs = ["cxl.m.cxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/ral/ralio:asio",
        "//src/rcl/rclw:console",
        ":config",
        ":controller",
        ":reactor",
        ":unit",
        ":unit_view",
        ],
    copts = RSR_DEFAULT_COPTS,)
