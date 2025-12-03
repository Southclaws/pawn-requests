# syntax=docker/dockerfile:1
FROM ghcr.io/qdm12/basedevcontainer:debian

USER root

ENV DEBIAN_FRONTEND=noninteractive \
    CMAKE_VERSION=3.25.1

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        g++-multilib \
        git \
        ca-certificates \
        curl \
        wget \
        make \
        python3 \
        python3-pip \
        python3-setuptools \
        pkg-config \
        unzip \
        p7zip-full \
        ninja-build \
        docker.io \
        xz-utils && \
    rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh -O /tmp/cmake.sh && \
    chmod +x /tmp/cmake.sh && \
    /tmp/cmake.sh --skip-license --prefix=/usr/local --exclude-subdir && \
    rm /tmp/cmake.sh

RUN python3 -m pip install --no-cache-dir "conan<2" --break-system-packages

RUN sh -c "$(curl --location https://taskfile.dev/install.sh)" -- -d -b /usr/local/bin && \
    chmod +x /usr/local/bin/task

RUN wget https://github.com/earthly/earthly/releases/latest/download/earthly-linux-amd64 -O /usr/local/bin/earthly && \
    chmod +x /usr/local/bin/earthly && \
    earthly --version && \
    earthly bootstrap --no-buildkit --with-autocomplete || true

RUN curl -sSL https://github.com/Southclaws/sampctl/releases/latest/download/sampctl_1.12.0_linux_amd64.tar.gz | \
    tar -xz -C /usr/local/bin sampctl && \
    chmod +x /usr/local/bin/sampctl

ENV PATH="/root/.local/bin:${PATH}"

WORKDIR /workspaces/pawn-requests
