FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git \
    python3 \
    cmake \
    ninja-build \
    build-essential \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    libcurl4-openssl-dev \
    libssl-dev \
    pkg-config \
    libusb-1.0-0-dev \
    libcjson-dev && \
    rm -rf /var/lib/apt/lists/*

RUN git clone --branch v1.2.0 https://github.com/redis/hiredis.git /opt/hiredis && \
    cd /opt/hiredis && \
    make USE_SSL=1 && \
    make USE_SSL=1 PREFIX=/usr install && \
    ldconfig

ENV PICO_SDK_PATH=/opt/pico-sdk
RUN git clone https://github.com/raspberrypi/pico-sdk.git $PICO_SDK_PATH && \
    cd $PICO_SDK_PATH && git submodule update --init


ENV PICO_EXTRAS_PATH=/opt/pico-extras
RUN git clone https://github.com/raspberrypi/pico-extras.git $PICO_EXTRAS_PATH && \
    cd $PICO_EXTRAS_PATH && git submodule update --init


WORKDIR /workspace