FROM maddinat0r/debian-samp

RUN apt update && apt install -y g++ git make zlib1g-dev libboost-all-dev libssl-dev cmake
RUN git clone https://github.com/Microsoft/cpprestsdk.git casablanca
RUN cd casablanca/Release && mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-m32 -DBUILD_SHARED_LIBS=0 && make

ADD . .
RUN mkdir build
ENTRYPOINT [ "make", "build-inside" ]
