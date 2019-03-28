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
        "//src/ral/raldsp:mixer",
        "//src/ral/raldsp:sampler",
        "//src/ral/ralw:wavetable",
        "//src/rcl/rclmt:reactor_file",
        "//src/rcl/rclmt:signal",
        "//src/rcl/rcls:file",
        ":channelstrip",
        ":config",
        ":effect",
        ":log",
        ":sequencer",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "effect",
    srcs = ["effect.cxx"],
    hdrs = ["effect.hxx"],
    deps = [
        "//src/ral/raldsp:filter",
        "//src/ral/raldsp:multimode",
        "//src/ral/raldsp:ratereducer",
        "//src/ral/raldsp:syncdelay",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "channelstrip",
    srcs = ["channelstrip.cxx"],
    hdrs = ["channelstrip.hxx"],
    deps = [
        "//src/ral/raldsp:distortion",
        "//src/ral/raldsp:panning",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_binary(
    name = "cxl",
    srcs = ["main.cxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/cxl/ui/root:controller",
        "//src/rcl/rclmt:reactor",
        "//src/rcl/rcls:console",
        "//src/rcl/rcls:file",
        "//src/rcl/rclt:util",
        ":config",
        ":log",
        ":unit",
        ":host",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "log",
    srcs = ["log.cxx"],
    hdrs = ["log.hxx"],
    deps = [
        "//src/rcl/rclmt:signal",
        "//src/rcl/rcls:file",
        ":config",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "host",
    srcs = ["host.cxx"],
    hdrs = ["host.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/ral/ralio:asio",
        "//src/rcl/rclmt:signal",
        ":log",
        ":unit", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "tap_tempo",
    srcs = ["tap_tempo.cxx"],
    hdrs = ["tap_tempo.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rcls:timer",
        ":log", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name="sequencer",
    srcs=["sequencer.cxx"],
    hdrs=["sequencer.hxx"],
    deps=[
        "//src/ral/raldsp:sampler",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
