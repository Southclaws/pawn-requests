# -
# Setup test requirements
# -

test-setup:
	cd test && sampctl server ensure
	sampctl package ensure

# -
# Run Tests
# -

test-windows:
	cp test/plugins/Debug/restful.dll test/plugins/restful.dll
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
