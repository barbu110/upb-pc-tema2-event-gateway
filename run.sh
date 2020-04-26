#!/bin/bash

printf "Host IP: "
ip a s eth0 | grep -Po 'inet \K[\d.]+'
echo

APP_NAME=$1

case $use_valgrind in
  "yes")
    VALGRIND_CMD="valgrind"
    ;;
  "no")
    VALGRIND_CMD=""
    ;;
esac

if [ "$APP_NAME" == "gateway_server" ]; then
  if [ "$#" -lt "2" ]; then
    echo "usage: $APP_NAME <server_port>"
    exit 1
  fi

  echo "Running Gateway..."

  $VALGRIND_CMD bazel-bin/main/gateway_server $2
elif [ "$APP_NAME" == "subscriber" ]; then
  if [ "$#" -lt "4" ]; then
    echo "usage: $APP_NAME <client_id> <server_ip> <server_port>"
    exit 1
  fi

  echo "Running Subscriber..."

  $VALGRIND_CMD bazel-bin/main/subscriber $2 $3 $4
else
  echo "Supported commands: gateway_server, subscriber."
  exit 1
fi
