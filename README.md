# pawn-requests

[![GitHub](https://shields.southcla.ws/badge/sampctl-pawn--requests-2f2f2f.svg?style=for-the-badge)](https://github.com/Southclaws/pawn-requests)

This package provides an API for interacting with HTTP(S) APIs with support for
text and JSON data types.

## Installation

Simply install to your project:

```bash
sampctl package install Southclaws/pawn-requests
```

Include in your code and begin using the library:

```pawn
#include <requests>
```

## Usage

### Requests

The Requests API is based on common implementations of similar libraries in
languages such as Go, Python and JS (Node.js).

There is an example of a basic gamemode that uses requests to store player data
as JSON [here](https://github.com/Southclaws/pawn-requests-example).

#### Requests Client

First you create a `RequestsClient`, you should store this globally:

```pawn
new RequestsClient:client;

main() {
    client = RequestsClient("http://httpbin.org/");
}
```

When you create a RequestsClient, you specify the **endpoint** you want to send
requests to with that client. This means you don't specify the endpoint for each
individual request.

You can also set headers for the client, these headers will be sent with every
request. This is useful for setting authentication headers for a private
endpoint:

```pawn
new RequestsClient:client;

main() {
    client = RequestsClient("http://httpbin.org/", RequestHeaders(
        "Authorization", "Bearer xyz"
    ));
}
```

The `RequestHeaders` function expects an even number of string arguments. It's
good practice to lay out your headers in a key-value style, like:

```pawn
RequestHeaders(
    "Authorization", "Bearer xyz",
    "Connection", "keep-alive",
    "Cache-Control", "no-cache"
)
```

But don't forget these are just normal arguments to a function so watch out for
trailing commas!

#### Making Basic Requests

Now you have a client, you can start making requests. If you want to work with
plain text or any data other than JSON, you use the `Request` function:

```pawn
Request(
    client,
    "robots.txt",
    HTTP_METHOD_GET,
    "OnGetData",
    .headers = RequestHeaders()
);

public OnGetData(Request:id, E_HTTP_STATUS:status, data[], dataLen) {
    printf("status: %d, data: '%s'", _:status, data);
}
```

Using the client constructed earlier, this would hit
`http://httpbin.org/robots.txt` with a GET request and when the request has
finished, `OnGetData` would be called and print:

```text
status: 200, data: 'User-agent: *
Disallow: /deny
'
```

The behaviour is similar to the existing SA:MP `HTTP()` function except this
supports headers, a larger body, more methods, HTTPS and is generally safer in
terms of error handling.

#### Making JSON Requests

JSON requests allow you to inline construct JSON at the request side as well as
access JSON objects in the response.

For example, the endpoint `http://httpbin.org/anything` returns JSON data so we
can access that directly as a `Node:` object in the response callback:

```pawn
RequestJSON(
    client,
    "anything",
    HTTP_METHOD_GET,
    "OnGetJson",
    .headers = RequestHeaders()
);

public OnGetJson(Request:id, E_HTTP_STATUS:status, Node:node) {
    new output[128];
    JsonGetString(node, "method", output);
    printf("anything response: '%s'", output);
}
```

The `anything` endpoint at httpbin responds with a bunch of related data in JSON
format. The `method` field contains the method used to perform the request and
in this case, the method is `GET` so `OnGetJson` will output
`anything response: 'GET'`.

And you can also send JSON data with a POST method:

```pawn
RequestJSON(
    client,
    "post",
    HTTP_METHOD_POST,
    "OnPostJson",
    JsonObject(
        "playerName", JsonString("Southclaws"),
        "kills", JsonInt(5),
        "topThreeWeapons", JsonArray(
            JsonString("M4"),
            JsonString("MP5"),
            JsonString("Desert Eagle")
        )
    ),
    .headers = RequestHeaders()
);

public OnPostJson(Request:id, E_HTTP_STATUS:status, Node:node) {
    if(status == HTTP_STATUS_CREATED) {
        printf("successfully posted JSON!");
    }
}
```

You could quite easily build a JSON-driven storage server backed by MongoDB.

See the JSON section below for examples of manipulating JSON `Node:` objects.

See the
[pawn-requests-example](https://github.com/Southclaws/pawn-requests-example)
repository for a more full example of using requests and JSON together.

#### Request Failures

If a request fails for any reason, `OnRequestFailure` is called with the
following signature: `(Request:id, errorCode, errorMessage[], len)` where
`errorCode` and `errorMessage` contain information to help you debug the
request.

#### Keeping Track of Request IDs

Both `Request` and `RequestJSON` return a `Request:` tagged value. This value is
the request identifier and is unique during server runtime, same as how
`SetTimer` returns a unique ID.

Because responses are asynchronous and the data comes back in a callback at a
later time, most of the time you will have to store this ID so you know which
request triggered which response.

You cannot simply use the ID as an index to an array because it's an
automatically incrementing value and thus is unbounded. You should instead use
BigETI's pawn-map plugin to map request IDs to some other data - such as the
player/vehicle/house/etc that triggered the request. See the
[pawn-requests-example](https://github.com/Southclaws/pawn-requests-example) for
an example of this.

### JSON

If you don't already know what JSON is, a good place to start is
[MDN](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON).
It's pretty much a web API standard nowadays (Twitter, Discord, GitHub and just
about every other API uses it to represent data). I'll briefly go over it before
getting into the API.

This plugin stores JSON values as "Nodes". Each node represents a value of one
type. Here are some examples of the representations of different node types:

*   `{}` - Object that is empty
*   `{"key": "value"}` - Object with one key that points to a String node
*   `"hello"` - String
*   `1` - Number (integer)
*   `1.5` - Number (floating point)
*   `[1, 2, 3]` - Array, of Number nodes
*   `[{}, {}]` - Array of empty Object nodes
*   `true` - Boolean

The main point here is that everything is a node, even Objects and Arrays that
contain other nodes.

#### Building an Object

To build a JSON object to be sent in a request, you most likely want to start
with `JsonObject` however you can use any node as the root node, it depends on
where you're sending the data but for this example I'll use an Object as the
root node.

```pawn
new Node:node = JsonObject();
```

This just constructs an empty object and if you "stringify" it (stringify simply
means to turn into a string) you get:

```json
{}
```

So to add more nodes to this object, simply add parameters, as key-value pairs:

```pawn
new Node:node = JsonObject(
    "key", JsonString("value")
);
```

This would stringify as:

```json
{
    "key": "value"
}
```

You can nest objects within objects too:

```pawn
new Node:node = JsonObject(
    "key", JsonObject(
        "key", JsonString("value")
    )
);
```

```json
{
    "key": {
        "key": "value"
    }
}
```

And do arrays of any node:

```pawn
new Node:node = JsonObject(
    "key", JsonArray(
        JsonString("one"),
        JsonString("two"),
        JsonString("three"),
        JsonObject(
            "more_stuff1", JsonString("uno"),
            "more_stuff2", JsonString("dos"),
            "more_stuff3", JsonString("tres")
        )
    )
);
```

See the
[unit tests](https://github.com/Southclaws/pawn-requests/blob/master/test.pwn)
for more examples of JSON builders.

#### Accessing Data

When you request JSON data, it's provided as a `Node:` in the callback. Most of
the time, you'll get an object back but depending on the application that
responded this could differ.

Lets assume this request responds with the following data:

```json
{
    "name": "Southclaws",
    "score": 45,
    "vip": true,
    "inventory": [
        {
            "name": "M4",
            "ammo": 341
        },
        {
            "name": "Desert Eagle",
            "ammo": 32
        }
    ]
}
```

```pawn
public OnSomeResponse(Request:id, E_HTTP_STATUS:status, Node:json) {
    new ret;

    new name[MAX_PLAYER_NAME];
    ret = JsonGetString(node, "name", name);
    if(ret) {
        err("failed to get name, error: %d", ret);
        return 1;
    }

    new score;
    ret = JsonGetInt(node, "score", score);
    if(ret) {
        err("failed to get score, error: %d", ret);
        return 1;
    }

    new bool:vip;
    ret = JsonGetBool(node, "vip", vip);
    if(ret) {
        err("failed to get vip, error: %d", ret);
        return 1;
    }

    new Node:inventory;
    ret = JsonGetArray(node, "inventory", inventory);
    if(ret) {
        err("failed to get inventory, error: %d", ret);
        return 1;
    }

    new length;
    ret = JsonArrayLength(inventory, length);
    if(ret) {
        err("failed to get inventory array length, error: %d", ret);
        return 1;
    }

    for(new i; i < length; ++i) {
        new Node:item;
        ret = JsonArrayObject(inventory, i, item);
        if(ret) {
            err("failed to get inventory item %d, error: %d", i, ret);
            return 1;
        }

        new itemName[32];
        ret = JsonGetString(item, "name", itemName);
        if(ret) {
            err("failed to get inventory item %d, error: %d", i, ret);
            return 1;
        }

        new itemAmmo;
        ret = JsonGetInt(item, "name", itemAmmo);
        if(ret) {
            err("failed to get inventory item %d, error: %d", i, ret);
            return 1;
        }

        printf("item %d name: %s ammo: %d", itemName, itemAmmo);
    }

    return 0;
}
```

In this example, we extract each field from the JSON object with full error
checking. This example shows usage of object and array access as well as
primitives such as strings, integers and a boolean.

If you're not a fan of the overly terse and explicit error checking, you can
alternatively just check your errors at the end but this will mean you won't
know exactly _where_ an error occurred, just that it did.

```pawn
new ret;
ret += JsonGetString(node, "key1", value1);
ret += JsonGetString(node, "key2", value2);
ret += JsonGetString(node, "key3", value3);
if(ret) {
    err("some error occurred: %d", ret);
}
```

## Testing

To run unit tests for the plugin on Windows, first build the plugin with Visual
Studio by opening the `CMakeLists.txt` via the `File > Open > CMake` menu and
then building the project. You will need to pull the dependencies too so make
sure you've done `git submodule init && git submodule update` or cloned the
repository recursively.

Once you've done that, the .dll files will be in `./test/plugins/Debug`. There
is also a `-release` suffixed version of this make command for testing the
release binaries.

```powershell
make test-windows-debug
```

If you want to build and test the Linux version from a Windows machine, make
sure Docker is installed and run:

```powershell
make build-linux
```

Which will output `requests.so` to `./test/plugins`. To run unit tests on Linux,
run:

```powershell
make test-linux
```

Which will run the tests via sampctl with the `--container` flag set.

## Development

To set up the development environment, first install
[`vcpkg`](https://github.com/Microsoft/vcpkg) then
[cpprestsdk](https://github.com/Microsoft/cpprestsdk).

Open Visual Studio (A recent version with CMake support) and File > Open the
project `CMakeLists.txt`. VS will fail on the first attempt as it won't be able
to find cpprestsdk. To resolve this, edit `.vs/CMakeSettings.json` to contain
the necessary environment variables for a Debug and Release configuration:

```json
{
  "configurations": [
    {
      "name": "x86-Release",
      "generator": "Visual Studio 15 2017",
      "configurationType": "Release",
      "buildRoot":
        "${env.USERPROFILE}\\CMakeBuilds\\${workspaceHash}\\build\\${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "-m -v:minimal",
      "variables": [
        {
          "name": "CMAKE_TOOLCHAIN_FILE",
          "value": "C:/Users/Southclaws/vcpkg/scripts/buildsystems/vcpkg.cmake"
        }
      ]
    },
    {
      "name": "x86-Debug",
      "generator": "Visual Studio 15 2017",
      "configurationType": "Debug",
      "buildRoot":
        "${env.USERPROFILE}\\CMakeBuilds\\${workspaceHash}\\build\\${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "-m -v:minimal",
      "variables": [
        {
          "name": "CMAKE_TOOLCHAIN_FILE",
          "value": "C:/Users/Southclaws/vcpkg/scripts/buildsystems/vcpkg.cmake"
        }
      ]
    }
  ]
}
```

The configuration file may change depending on VS version or other things, as
long as the `CMAKE_TOOLCHAIN_FILE` variables are passed to CMake properly, the
build should succeed.

Once this is done, VS should start indexing all the dependencies. Once it has
finihed, in the menu bar, hit CMake > Build All and it should spit out a `.dll`.
