FROM csapp-env:latest

# Disable interactive prompts during apt installs
ENV DEBIAN_FRONTEND=noninteractive

# Install valgrind for cache memory tracing and python3 for driver.py
RUN apt-get update && apt-get install -y \
    valgrind \
    python3 \
    python-is-python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
CMD ["/bin/bash"]

