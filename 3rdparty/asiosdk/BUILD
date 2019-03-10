load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name='asiosdk',
    hdrs=glob(["common/*.h"]), #, "host/*.h", "host/pc/*.h"]),
    includes = ["common"], #, "host", "host/pc"],
    srcs=[
        #"common/asio.cpp",
        #"host/asiodrivers.cpp",
        #"host/pc/asiolist.cpp",
        ],
    copts = RSR_DEFAULT_COPTS,
    )
