load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name="grid_sequencer",
    srcs=["ralm_grid_sequencer.cxx"],
    hdrs=["ralm_grid_sequencer.hxx"],
    deps=[
        "//src/ral/raldsp:sampler",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
