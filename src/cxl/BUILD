load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "cxl",
    srcs = ["cxl.m.cxx"],
    deps = [
        "//src/ral/raldsp:mixer",
        "//src/ral/raldsp:sampler",
        "//src/ral/ralio:asio",
        "//src/ral/ralm:grid_sequencer",
        "//src/ral/ralw:wavetable",
        "//src/rcl/rclw:console",
        ],
    linkopts = ['/DEFAULTLIB:winmm.lib'],
    copts = RSR_DEFAULT_COPTS,)
