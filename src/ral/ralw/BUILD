load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name="wavfile",
    srcs=["ralw_wavfile.cxx"],
    hdrs=["ralw_wavfile.hxx"],
    deps=[],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name="wavstream",
    srcs=["ralw_wavstream.cxx"],
    hdrs=["ralw_wavstream.hxx"],
    deps=[
        ":wavfile",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name="mpcwave",
    srcs=["ralw_mpcwave.cxx"],
    hdrs=["ralw_mpcwave.hxx"],
    deps=[
        ":wavfile",
        ],
    copts = RSR_DEFAULT_COPTS,
    )

cc_library(
    name="wavetable",
    srcs=["ralw_wavetable.cxx"],
    hdrs=["ralw_wavetable.hxx"],
    deps=[
        ":mpcwave",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
