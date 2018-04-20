FROM maddinat0r/debian-samp

# attempt to build cpprestsdk from scratch... doesn't work.
# RUN apt update && apt install -y g++ git make zlib1g-dev libboost-all-dev libssl-dev cmake
# RUN git clone https://github.com/Microsoft/cpprestsdk.git casablanca
# RUN cd casablanca/Release && \
#     mkdir build && \
#     cd build && \
#     cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-m32 -DBUILD_SHARED_LIBS=0 -DBoost_USE_STATIC_LIBS=ON -DBoost_DEBUG=ON && \
#     make

# install from apt, also doesn't work.
RUN apt update && apt install -y libcpprest-dev

ADD . .
RUN mkdir build
ENTRYPOINT [ "make", "build-inside" ]
