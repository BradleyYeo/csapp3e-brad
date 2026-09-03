FROM --platform=linux/amd64 ubuntu:latest

# Disable interactive prompts during apt installs
ENV DEBIAN_FRONTEND=noninteractive

# Install gdb, binutils (for objdump, strings), and other useful tools
RUN apt-get update && apt-get install -y \
    gdb \
    binutils \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# By default, start an interactive bash shell
CMD ["/bin/bash"]
