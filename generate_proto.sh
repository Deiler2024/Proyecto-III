#!/bin/bash

PROTO_DIR=common/protos
OUT_DIR=common/generated

mkdir -p "$OUT_DIR"

protoc -I="$PROTO_DIR" \
  --cpp_out="$OUT_DIR" \
  --grpc_out="$OUT_DIR" \
  --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" \
  "$PROTO_DIR/tecmfs.proto"
