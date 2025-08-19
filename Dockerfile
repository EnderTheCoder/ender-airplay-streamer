# Dockerfile for Airplay Streamer build environment
FROM ubuntu:24.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install FFmpeg development libraries
RUN apt-get update && \
    apt-get install -y \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libavutil-dev \
    libavfilter-dev \
    libavdevice-dev \
    && rm -rf /var/lib/apt/lists/*

# Install OpenSSL development libraries
RUN apt-get update && \
    apt-get install -y \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install LibPlist development libraries
RUN apt-get update && \
    apt-get install -y \
    libplist-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Bonjour/Avahi development libraries (for dns_sd.h)
RUN apt-get update && \
    apt-get install -y \
    libavahi-compat-libdnssd-dev \
    && rm -rf /var/lib/apt/lists/*

COPY . /airplay-streamer

WORKDIR /airplay-streamer

# Build the project
RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja && \
    cmake --build . -j && \
    cmake --install .

# Default command
CMD ["/bin/bash"]
