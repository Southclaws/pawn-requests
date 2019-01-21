# -
# Development
# -

prepare:
	cd test && sampctl server ensure
	sampctl package ensure

toolchain-win32:
	rustup default stable-i686-pc-windows-msvc

build-win32-release: toolchain-win32
	cargo +stable-i686-pc-windows-msvc build --release
	cp target/release/pawn_requests.dll test/plugins/requests.dll

build-win32-debug: toolchain-win32
	cargo +stable-i686-pc-windows-msvc build
	cp target/debug/pawn_requests.dll test/plugins/requests.dll

toolchain-linux:
	rustup default stable-i686-unknown-linux-gnu

build-linux-release: toolchain-linux
	cargo +stable-i686-unknown-linux-gnu build --release
	cp target/release/libpawn_requests.so test/plugins/requests.so

build-linux-debug: toolchain-linux
	cargo +stable-i686-unknown-linux-gnu build
	cp target/debug/libpawn_requests.so test/plugins/requests.so

# -
# Run Tests
# -

test-native:
	sampctl package build
	cd test && sampctl server run

test-container:
	sampctl package build
	cd test && sampctl server run --container

# -
# Build inside container
# -

build-container:
	rm -rf build
	docker build -t southclaws/requests-build .
	docker run -v $(shell pwd)/test/plugins:/root/test/plugins southclaws/requests-build
