cc_library(
  name = "commons",
  srcs = glob(["src/**/*.cpp", "src/**/*.h"]),
  hdrs = glob(["include/**/*.h"]),
  includes = ["include"],
  visibility = ["//visibility:public"],
  deps = [
    "//lib/net_utils",
    "@micro//lib/microloop:microloop",
  ],
)
