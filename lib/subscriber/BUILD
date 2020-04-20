cc_library(
  name = "subscriber",
  srcs = glob(["src/**/*.cpp", "src/**/*.h"]),
  hdrs = glob(["include/**/*.h"]),
  includes = ["include"],
  visibility = ["//visibility:public"],
  deps = [
    "//lib/net_utils",
    "//lib/commons",
    "@micro//lib/microloop:microloop",
    "@com_google_absl//absl/strings",
  ],
)
