load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name='winreg',
    hdrs=["winreg.hxx"],
    includes = ["."],
    srcs=[],
    copts = RSR_DEFAULT_COPTS,
    linkopts = ['-DEFAULTLIB:advapi32.lib'],
    )
