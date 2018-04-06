# pawn-restful

restful provides an API for interacting with RESTful HTTP(S) JSON APIs.

## Project Notes

This will be removed in the next commit. I thought I'd write a bit about the
project layout while it's bare as I've put some effort into ensuring it's a good
boilerplate to start from. The project includes a quick and simple way to test
plugins as well as a CMake setup and Y_Less' plugin-natives usage.

### Requirements

This development environment requires Docker which means you can compile the
plugin for Linux while using a Windows machine. You could use a VM for this but
Docker makes the process much simpler and automated via a couple of simple
commands.

It also requires GNU Make on Windows - which is simple to install and add to
your system `%PATH%` so you can run it from the project directory.

And finally, of course it requires sampctl because it's one of my creations :)

### makefile

The makefile is the starting point, but it's not related to the C++ code at all,
it's simply a task list for managing tests.

Run `make test-setup` before doing anything to get everything set up. You should
only need to do this once. This runs `sampctl server ensure` inside `test/`
which uses the declarative `samp.json` file to automatically set up a SA:MP
server instance inside of the `test` directory. Don't worry, the `.gitignore` is
already set up to ignore the server related files. It then runs
`sampctl package ensure` to download the necessary dependencies for building the
Pawn test script - this is just the SA:MP standard library and YSI for
y_testing.

Run `make test-windows` to:

* Copy the .dll file from `test/plugins/Debug` to `test/plugins` - an awkward
  side-effect of Visual Studio
* build the Pawn test script to an .amx with y_testing
* run the test script as a full SA:MP server instance

You should see `Function called` among the typical SA:MP boilerplate text and
y_testing output.

### `src`

This contains the C++ source code. Read the file head comments in order:

* `restful.cpp`
* `common.hpp`
* `natives.hpp`
* `natives.cpp`
* `impl.hpp`
* `impl.cpp`

`CMakeLists.txt` declares which files are relevant to the build process. I'm not
a CMake expert so I'll avoid going any further with this.

`plugin-natives` is a set of useful macros by Y*Less that make declaring plugin
native functions \_way* easier than the old way. See the plugin-natives repo for
more info.

### `lib`

This contains external dependencies. These are libraries required by this
project, this includes `cmake-modules` by Zeex which provides some useful CMake
stuff for working with SA:MP plugins and `samp-plugin-sdk` which is the software
development kit for working with SA:MP itself. These are all declared as Git
submodules (`.gitmodules`) and are updatable with `git submodule update`.

### `Dockerfile`

If you're new to docker, this file declares an "Image" which is an isolated
filesystem running a particular operating system - in our case it derives from
`maddinat0r/debian-samp` which is a Debian image built specifically for
compiling SA:MP plugins by maddinat0r. This means you can compile an .so on
Windows very easily by running `make build-linux`

## Installation

Simply install to your project:

```bash
sampctl package install Southclaws/pawn-restful
```

Include in your code and begin using the library:

```pawn
#include <restful>
```

## Usage

(API currently unstable)

## Testing

Run unit tests with:

### Windows

```powershell
make test-windows
```

### Linux

```bash
make test-debian
```
