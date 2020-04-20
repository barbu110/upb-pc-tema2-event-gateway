cc_binary(
  name = "gateway_server",
  srcs = ["gateway_server.cpp"],
  deps = [
    "//lib/gateway:gateway",
    "//lib/net_utils:net_utils",
    "@micro//lib/microloop:microloop",
  ],
)

cc_binary(
  name = "subscriber",
  srcs = ["subscriber.cpp"],
  deps = [
    "//lib/subscriber:subscriber",
    "@micro//lib/microloop:microloop",
  ],
)
