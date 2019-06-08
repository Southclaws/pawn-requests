FROM maddinat0r/debian-samp


# -
# zlib
# -

RUN wget http://prdownloads.sourceforge.net/libpng/zlib-1.2.11.tar.gz && \
    tar -xzf zlib-1.2.11.tar.gz && \
    cd zlib-1.2.11 && \
    CFLAGS=-m32 ./configure --static && \
    make && make install && \
    cd ..


# -
# OpenSSL
# -

RUN wget https://www.openssl.org/source/openssl-1.0.2o.tar.gz && \
    tar -xzf openssl-1.0.2o.tar.gz
RUN cd openssl-1.0.2o && \
    setarch i386 ./config -m32 no-shared && \
    make && make install && \
    cd ..
ENV OPENSSL_ROOT_DIR=/root/openssl-1.0.2o


# -
# Boost
# -
RUN wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz && \
    tar -xzf boost_1_67_0.tar.gz
RUN cd boost_1_67_0 && \
    ./bootstrap.sh && \
    ./b2 link=static address-model=32 -sZLIB_SOURCE=/root/zlib-1.2.11 && \
    cd ..
ENV BOOST_ROOT=/root/boost_1_67_0


# -
# cpprestsdk
# -

RUN git clone --recursive https://github.com/Microsoft/cpprestsdk.git casablanca
RUN cd casablanca/Release && \
    git checkout v2.10.8 && \
    mkdir build && \
    cd build && \
    cmake .. \
    -DBUILD_SAMPLES=OFF \
    -DBUILD_TESTS=OFF \
    -DWERROR=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-m32 -Wno-error -fpermissive" \
    -DBUILD_SHARED_LIBS=0 \
    -DBoost_USE_STATIC_LIBS=ON && \
    make && \
    make install && \
    cd /root


# -
# Build requests plugin by mounting repo directory into container and executing
# `make build-inside` as the entrypoint.
# -
