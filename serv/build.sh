#!/bin/bash

mkdir build/
cmake -S . -B build -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=Y -DAPI_KEY=${API_KEY} -DREDIS_PORT=6380 \
  -DWATERING_SSL_CERT_PATH="/home/j00r/Downloads/ca.crt" -DREDIS_PASS="${REDIS_PASS}" -DENDPOINT="${ENDPOINT}"
