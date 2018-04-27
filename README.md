# pawn-requests

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

### Restful Client

### JSON

If you don't already know what JSON is, a good place to start is
[MDN](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON).
It's pretty much a web API standard nowadays (Twitter, Discord, GitHub and just
about every other API uses it to represent data). I'll briefly go over it before
getting into the API.

This plugin stores JSON values as "Nodes". Each node represents a value of one
type. Here are some examples of the representations of different node types:

* `{}` - Object that is empty
* `{"key": "value"}` - Object with one key that points to a String node
* `"hello"` - String
* `1` - Number (integer)
* `1.5` - Number (floating point)
* `[1, 2, 3]` - Array, of Number nodes
* `[{}, {}]` - Array of empty Object nodes
* `true` - Boolean

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

**Note:** _the signature for response callbacks is in a comment in requests.inc_

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

Run unit tests with:

### Windows

```powershell
make test-windows
```

### Linux

```bash
make test-debian
```
