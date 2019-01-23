FROM rust:1.31

WORKDIR /root
COPY . .
RUN dpkg --add-architecture i386 && apt update && apt install -y gcc-multilib libssl-dev:i386 && make toolchain-linux

ENTRYPOINT [ "make", "build-linux-release" ]
