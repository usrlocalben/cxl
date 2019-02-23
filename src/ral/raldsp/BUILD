load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name="idspoutput",
    srcs=["raldsp_idspoutput.cxx"],
    hdrs=["raldsp_idspoutput.hxx"],
    deps=[],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name="math",
    srcs=["raldsp_math.cxx"],
    hdrs=["raldsp_math.hxx"],
    deps=[],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name="filter",
    srcs=["raldsp_filter.cxx"],
    hdrs=["raldsp_filter.hxx"],
    deps=[
        ":idspoutput",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name="mixer",
    srcs=["raldsp_mixer.cxx"],
    hdrs=["raldsp_mixer.hxx"],
    deps=[
        ":idspoutput",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name="syncdelay",
    srcs=["raldsp_syncdelay.cxx"],
    hdrs=["raldsp_syncdelay.hxx"],
    deps=[
        ":idspoutput",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name="sampler",
    srcs=["raldsp_sampler.cxx"],
    hdrs=["raldsp_sampler.hxx"],
    deps=[
        "//src/ral/ralw:mpcwave",
        "//src/ral/ralw:wavetable",
        ":filter",
        ":idspoutput",
        ":syncdelay",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
