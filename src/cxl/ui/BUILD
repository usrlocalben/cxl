load("//:copts.bzl", "RSR_DEFAULT_COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "root",
    srcs = ["cxl_ui_root.cxx"],
    hdrs = ["cxl_ui_root.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclw:console",
        "//src/cxl:log",
        "//src/cxl:reactor",
        "//src/cxl:unit",
        "//src/cxl:widget",
        ":pattern_editor",
        ":loading_status",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "pattern_editor",
    srcs = ["cxl_ui_pattern_editor.cxx"],
    hdrs = ["cxl_ui_pattern_editor.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/cxl:log",
        "//src/cxl:reactor",
        "//src/cxl:unit",
        "//src/cxl:widget",
        "//src/rcl/rclw:console",
        ":pattern_length_edit",
        ":alert",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "loading_status",
    srcs = ["cxl_ui_loading_status.cxx"],
    hdrs = ["cxl_ui_loading_status.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclw:console",
        "//src/cxl:reactor",
        "//src/cxl:unit",
        "//src/cxl:widget",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "pattern_length_edit",
    srcs = ["cxl_ui_pattern_length_edit.cxx"],
    hdrs = ["cxl_ui_pattern_length_edit.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclw:console",
        "//src/cxl:widget",
        ],
    copts = RSR_DEFAULT_COPTS,)


cc_library(
    name = "alert",
    srcs = ["cxl_ui_alert.cxx"],
    hdrs = ["cxl_ui_alert.hxx"],
    deps = [
        "//3rdparty/fmt",
        "//src/rcl/rclw:console",
        "//src/cxl:widget",
        ],
    copts = RSR_DEFAULT_COPTS,)
