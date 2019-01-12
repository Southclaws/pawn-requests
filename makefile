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
	cp target/release/pawn_templates.dll test/plugins/templates.dll

build-win32-debug: toolchain-win32
	cargo +stable-i686-pc-windows-msvc build
	cp target/debug/pawn_templates.dll test/plugins/templates.dll

toolchain-linux:
	rustup default stable-i686-unknown-linux-gnu

build-linux-release: toolchain-linux
	cargo +stable-i686-unknown-linux-gnu build --release
	cp target/release/libpawn_templates.so test/plugins/templates.so

build-linux-debug: toolchain-linux
	cargo +stable-i686-unknown-linux-gnu build
	cp target/debug/libpawn_templates.so test/plugins/templates.so

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
	docker build -t southclaws/templates-build .
	docker run -v $(shell pwd)/test/plugins:/root/test/plugins southclaws/templates-build

# -
# Build Release Archives
# -

release-windows:
	mkdir release-windows
	mkdir release-windows/plugins
	mkdir release-windows/dependencies
	mkdir release-windows/includes
	cp test/plugins/Release/requests.dll release-windows/plugins/requests.dll
	cp test/plugins/Release/boost_date_time-vc141-mt-x32-1_68.dll release-windows/dependencies/boost_date_time-vc141-mt-x32-1_68.dll
	cp test/plugins/Release/boost_system-vc141-mt-x32-1_68.dll release-windows/dependencies/boost_system-vc141-mt-x32-1_68.dll
	cp test/plugins/Release/cpprest_2_10.dll release-windows/dependencies/cpprest_2_10.dll
	cp test/plugins/Release/LIBEAY32.dll release-windows/dependencies/LIBEAY32.dll
	cp test/plugins/Release/SSLEAY32.dll release-windows/dependencies/SSLEAY32.dll
	cp test/plugins/Release/zlib1.dll release-windows/dependencies/zlib1.dll
	cp *.inc release-windows/includes/
	cd release-windows/ && 7z a -r ../pawn-requests-windows.zip *

release-linux:
	mkdir release-linux
	mkdir release-linux/plugins
	mkdir release-linux/includes
	cp test/plugins/requests.so release-linux/plugins/requests.so
	cp *.inc release-linux/includes/
	cd release-linux/ && 7z a -r ../pawn-requests-linux.zip *

release: release-windows release-linux
