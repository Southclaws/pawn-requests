VERSION 0.6
ARG CMAKE_VERSION=3.25.1

build-base:
    FROM debian:bookworm
    ENV DEBIAN_FRONTEND=noninteractive
    RUN dpkg --add-architecture i386 && \
        apt-get update && \
        apt-get install -yq --no-install-recommends \
            g++-multilib git ca-certificates make wget curl python3 python3-setuptools python3-pip python3-venv \
            p7zip-full ninja-build pkg-config && \
        rm -rf /var/lib/apt/lists/*
    RUN python3 -m venv /opt/conan && \
        /opt/conan/bin/pip install --no-cache-dir --upgrade pip && \
        /opt/conan/bin/pip install --no-cache-dir "conan<2" && \
        ln -s /opt/conan/bin/conan /usr/local/bin/conan
    RUN sh -c "$(curl --location https://taskfile.dev/install.sh)" -- -d -b /usr/local/bin && chmod +x /usr/local/bin/task
    RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh && \
        chmod +x ./cmake-${CMAKE_VERSION}-linux-x86_64.sh && \
        ./cmake-${CMAKE_VERSION}-linux-x86_64.sh --skip-license --prefix=/usr/local/ --exclude-subdir && \
        rm ./cmake-${CMAKE_VERSION}-linux-x86_64.sh
    WORKDIR /src
    COPY . .

build:
    FROM +build-base
    RUN --mount=type=cache,target=/root/.conan \
        rm -rf build && \
        conan profile new default --detect --force
    RUN --mount=type=cache,target=/root/.conan \
        cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    RUN --mount=type=cache,target=/root/.conan \
        cmake --build build --config Release -j$(nproc)
    SAVE ARTIFACT build/lib/requests.so AS LOCAL build/lib/requests.so

package:
    FROM +build
    RUN rm -rf releases && mkdir -p releases/linux_package/plugins releases/linux_package/includes
    RUN cp build/lib/requests.so releases/linux_package/plugins/requests.so
    RUN cp *.inc releases/linux_package/includes/
    WORKDIR /src/releases/linux_package
    RUN 7z a ../pawn-requests-linux.zip ./includes ./plugins
    WORKDIR /src
    SAVE ARTIFACT releases AS releases
    SAVE ARTIFACT releases AS LOCAL releases
    SAVE ARTIFACT releases/pawn-requests-linux.zip AS pawn-requests-linux.zip
    SAVE ARTIFACT releases/pawn-requests-linux.zip AS LOCAL releases/pawn-requests-linux.zip

release:
    BUILD +package