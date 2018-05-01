# -
# Setup test requirements
# -

test-setup:
	cd test && sampctl server ensure
	sampctl package ensure

# -
# Run Tests
# -

test-windows-debug:
	-cp test/plugins/Debug/requests.dll test/plugins/requests.dll
	-cp test/plugins/Debug/boost_date_time-vc141-mt-gd-x32-1_66.dll test/boost_date_time-vc141-mt-gd-x32-1_66.dll
	-cp test/plugins/Debug/boost_system-vc141-mt-gd-x32-1_66.dll test/boost_system-vc141-mt-gd-x32-1_66.dll
	-cp test/plugins/Debug/cpprest_2_10d.dll test/cpprest_2_10d.dll
	-cp test/plugins/Debug/LIBEAY32.dll test/LIBEAY32.dll
	-cp test/plugins/Debug/SSLEAY32.dll test/SSLEAY32.dll
	-cp test/plugins/Debug/zlibd1.dll test/zlibd1.dll
	sampctl package build
	cd test && sampctl server run
test-windows-release:
	-cp test/plugins/Release/requests.dll test/plugins/requests.dll
	-cp test/plugins/Release/boost_date_time-vc141-mt-x32-1_66.dll test/boost_date_time-vc141-mt-x32-1_66.dll
	-cp test/plugins/Release/boost_system-vc141-mt-x32-1_66.dll test/boost_system-vc141-mt-x32-1_66.dll
	-cp test/plugins/Release/cpprest_2_10.dll test/cpprest_2_10.dll
	-cp test/plugins/Release/LIBEAY32.dll test/LIBEAY32.dll
	-cp test/plugins/Release/SSLEAY32.dll test/SSLEAY32.dll
	-cp test/plugins/Release/zlib1.dll test/zlib1.dll
	sampctl package build
	cd test && sampctl server run

test-linux:
	sampctl package build
	cd test && sampctl server run --container

# -
# Build (Linux)
# -

build-linux:
	rm -rf build
	docker build -t southclaws/requests-build .
	docker run \
		-v $(shell pwd):/root/requests \
		--entrypoint make \
		--workdir /root/requests \
		southclaws/requests-build \
		build-inside

build-interactive:
	docker run \
		-v $(shell pwd):/root/requests \
		-it \
		southclaws/requests-build

build-inside:
	-mkdir build-container
	cd build-container && cmake .. && make

# -
# Build Release Archives
# -

release-windows:
	mkdir release-windows
	mkdir release-windows/plugins
	mkdir release-windows/dependencies
	mkdir release-windows/includes
	cp test/plugins/Release/requests.dll release-windows/plugins/requests.dll
	cp test/plugins/Release/boost_date_time-vc141-mt-gd-x32-1_66.dll release-windows/dependencies/boost_date_time-vc141-mt-gd-x32-1_66.dll
	cp test/plugins/Release/boost_system-vc141-mt-gd-x32-1_66.dll release-windows/dependencies/boost_system-vc141-mt-gd-x32-1_66.dll
	cp test/plugins/Release/cpprest_2_10d.dll release-windows/dependencies/cpprest_2_10d.dll
	cp test/plugins/Release/LIBEAY32.dll release-windows/dependencies/LIBEAY32.dll
	cp test/plugins/Release/SSLEAY32.dll release-windows/dependencies/SSLEAY32.dll
	cp test/plugins/Release/zlibd1.dll release-windows/dependencies/zlibd1.dll
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
