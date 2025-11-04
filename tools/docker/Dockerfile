FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    ca-certificates \
    clang-format \
    git \
    qtbase5-dev \
    qtdeclarative5-dev \
    qtdeclarative5-dev-tools \
    qttools5-dev \
    qttools5-dev-tools \
    --no-install-recommends \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Add Qt tools to PATH
ENV PATH="/usr/lib/qt5/bin:/usr/lib/x86_64-linux-gnu/qt5/bin:${PATH}"

COPY docker-runner.sh /usr/local/bin/docker-runner.sh
RUN chmod +x /usr/local/bin/docker-runner.sh

# Create a non-root user to run the tools
RUN useradd -ms /bin/bash formatter

USER formatter
WORKDIR /code
