FROM maddinat0r/debian-samp

# -
# vcpkg - install vcpkg, add to path and create a linux x86 static triplet file
# -
ENV PATH=$PATH:/root/vcpkg
RUN apt update && apt install curl unzip tar && \
    git clone https://github.com/Microsoft/vcpkg && \
    cd vcpkg && \
    ./bootstrap-vcpkg.sh && \
    touch triplets/x86-linux-static.cmake && \
    echo "set(VCPKG_TARGET_ARCHITECTURE x86)" >> triplets/x86-linux-static.cmake && \
    echo "set(VCPKG_CRT_LINKAGE static)" >> triplets/x86-linux-static.cmake && \
    echo "set(VCPKG_LIBRARY_LINKAGE static)" >> triplets/x86-linux-static.cmake && \
    echo "set(VCPKG_CMAKE_SYSTEM_NAME Linux)" >> triplets/x86-linux-static.cmake && \
    cd ..

# -
# cpprestsdk
# -
RUN vcpkg install cpprestsdk:x86-linux-static
