cc_library(
  name = "gateway",
  srcs = glob(["src/**/*.cpp"]),
  hdrs = glob(["include/**/*.h"]),
  includes = ["include"],
  visibility = ["//visibility:public"],
  deps = [
    "//lib/commons",
    "//lib/net_utils",
    "@micro//lib/microloop:microloop",
  ],
)
