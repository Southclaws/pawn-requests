VERSION 0.6
FROM debian:buster

ENV DEBIAN_FRONTEND=noninteractive
ENV CMAKE_VERSION=3.25.1

RUN dpkg --add-architecture i386 && \
        apt-get update && \
        apt -yq --no-install-recommends install -y g++-multilib git ca-certificates make wget git curl python3 python3-setuptools python3-pip p7zip-full && \
        python3 -m pip install conan

RUN sh -c "$(curl --location https://taskfile.dev/install.sh)" -- -d -b /usr/local/bin && chmod +x /usr/local/bin/task

WORKDIR /build
COPY . .

build:
    DO +INSTALL_CMAKE
    WORKDIR /build
    RUN task build:release

release:
    BUILD +build
    RUN task pkg:linux
    SAVE ARTIFACT ./releases AS LOCAL ./releases

INSTALL_CMAKE:
    COMMAND
    WORKDIR /build-cmake
    RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh && \
        chmod +x ./cmake-${CMAKE_VERSION}-linux-x86_64.sh && \
        ./cmake-${CMAKE_VERSION}-linux-x86_64.sh --skip-license --prefix=/usr/local/ --exclude-subdir
    ENV PATH "/usr/local/bin:$PATH"