MSVC_FLAGS = [
    "/EHsc",  # C++ exceptions: enabled
    "/GS-",  # security checks: disabled
    "/std:c++17",  # C++ standard: c++17
    "/permissive-",  # standard conformance: strict

    "/W3",
    "/WX",
    "/wd4305",  # "truncation from 'double' to 'float'
    "/wd4244",  # "conversion from 'double' to 'float' possible loss of data
    "/wd4005",  # macro redefinition
    "/wd4267",  # conversion from 'size_t' to 'type', possible loss of data
    #"-Wno-builtin-macro-redefined",  # clang-cl is angry

    "/DUNICODE",
    "/D_UNICODE",
    "/D_ENABLE_EXTENDED_ALIGNED_STORAGE",
    #"/DWIN32_LEAN_AND_MEAN",  # Don't bloat namespace with incompatible winsock versions.
    "/DNOMINMAX",  # Don't define min and max macros (windows.h)
    ]

MSVC_RELEASE_FLAGS = [
    #"/Oi",  # intrinsic functions: enable
    #"/Ot",  # optimizer mode: speed
    #"/Ob2",  # inline expansion: level 2
    "/fp:fast",  # fpu mode: less precise
    #"/DNDEBUG",  # disable assert, etc
    ]

RSR_DEFAULT_COPTS = MSVC_FLAGS + MSVC_RELEASE_FLAGS
