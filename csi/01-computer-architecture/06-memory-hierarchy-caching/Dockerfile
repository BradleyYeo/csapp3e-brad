FROM ubuntu:24.04

# Disable interactive prompts during apt installs
ENV DEBIAN_FRONTEND=noninteractive

# Install build tools, Valgrind for cache tracing, and perf tools
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    gcc \
    make \
    valgrind \
    linux-tools-generic \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
CMD ["/bin/bash"]

