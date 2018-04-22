# pawn-restful

restful provides an API for interacting with RESTful HTTP(S) JSON APIs.

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
new Node:JsonObject();
```

This just constructs an empty object and if you "stringify" it (stringify simply
means to turn into a string) you get:

```json
{}
```

So to add more nodes to this object, simply add parameters, as key-value pairs:

```pawn
new Node:JsonObject(
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
new Node:JsonObject(
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
new Node:JsonObject(
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
[unit tests](https://github.com/Southclaws/pawn-restful/blob/master/test.pwn)
for more examples of JSON builders.

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
