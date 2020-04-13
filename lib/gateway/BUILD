cc_library(
  name = "gateway",
  srcs = glob(["src/**/*.cpp"]),
  hdrs = glob(["include/**/*.h"]),
  includes = ["include"],
  visibility = ["//visibility:public"],
  deps = [
    "//lib/subscriber",
    "@micro//lib/microloop:microloop",
  ],
)
