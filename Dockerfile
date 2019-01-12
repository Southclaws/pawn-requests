FROM rust:1.31

WORKDIR /root
COPY . .
RUN apt update && apt install -y gcc-multilib && make toolchain-linux

ENTRYPOINT [ "make", "build-linux-release" ]
