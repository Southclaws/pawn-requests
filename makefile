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
	-cp test/plugins/Debug/restful-d.dll test/plugins/restful.dll
	-cp test/plugins/Debug/boost_date_time-vc141-mt-gd-x32-1_66.dll test/boost_date_time-vc141-mt-gd-x32-1_66.dll
	-cp test/plugins/Debug/boost_system-vc141-mt-gd-x32-1_66.dll test/boost_system-vc141-mt-gd-x32-1_66.dll
	-cp test/plugins/Debug/cpprest_2_10d.dll test/cpprest_2_10d.dll
	-cp test/plugins/Debug/LIBEAY32.dll test/LIBEAY32.dll
	-cp test/plugins/Debug/SSLEAY32.dll test/SSLEAY32.dll
	-cp test/plugins/Debug/zlibd1.dll test/zlibd1.dll
	sampctl package build
	cd test && sampctl server run
test-windows-release:
	-cp test/plugins/Release/restful.dll test/plugins/restful.dll
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
	cd test && sampctl server run

# -
# Build (Linux)
# -

build-linux:
	rm -rf build
	docker build -t southclaws/restful-build .
	docker run -v $(shell pwd)/test/plugins:/root/test/plugins southclaws/restful-build

build-inside:
	cd build && cmake .. && make
