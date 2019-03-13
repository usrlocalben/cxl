load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "view",
    srcs = ["view.cxx"],
    hdrs = ["view.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclmt:reactor",
        "//src/rcl/rcls:text_canvas",
        "//src/textkit:keyevent",
        "//src/textkit:mainloop",
        "//src/textkit:widget",
        "//src/cxl:host",
        ],
    copts = RSR_DEFAULT_COPTS,)
