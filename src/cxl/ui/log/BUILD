load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "view",
    srcs = ["view.cxx"],
    hdrs = ["view.hxx"],
    deps = [
        "//src/cxl:log",
        "//src/rcl/rcls:text_canvas",
        "//src/textkit:widget", ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "controller",
    srcs = ["controller.cxx"],
    hdrs = ["controller.hxx"],
    deps = [
        "//src/cxl:log",
        "//src/textkit:mainloop",
        ":view",
        ],
    copts = RSR_DEFAULT_COPTS,)
