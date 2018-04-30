// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_requests
	#undef _inc_requests
#endif
// custom include-guard to ensure we don't duplicate
#if defined _requests_included
	#endinput
#endif
#define _requests_included


// todo: inline support for requests
#tryinclude <YSI\y_inline>
#if defined _INC_y_inline
    #define Y_INLINE_SUPPORT
#endif

#include "status_codes"
#include "http_methods"


// -
// Request API
// -

// E_CONTENT_TYPE represents the possible response types for a request.
// Response callback signatures in requests are dependent on "Content-Type":
// - CONTENT_TYPE_TEXT:
//   (Request:id, E_HTTP_STATUS:status, data[], dataLen)
// - CONTENT_TYPE_JSON:
//   (Request:id, E_HTTP_STATUS:status, Node:json)
enum E_CONTENT_TYPE {
    CONTENT_TYPE_EMPTY,
    CONTENT_TYPE_TEXT,
    CONTENT_TYPE_JSON
}

// RequestsClient initialises a new Requests client with an endpoint. The endpoint
// must include a scheme (http or https) and must not contain a path. Any
// headers set on the client are sent with every request from the client but may
// be overwritten on a per-request basis.
native Requests:RequestsClient(const endpoint[], Headers:headers = Headers:-1);

// RequestHeaders constructs a headers object and returns a handle ID. This
// function is designed to only ever be used as an argument to functions that
// take `Headers:` values as those functions handle the garbage collection of
// resources allocated on the heap.
native Headers:RequestHeaders(...);

// RequestText performs a request to send or receive text data
native Request:RequestText(
    Requests:id,
    const path[],
    E_HTTP_METHOD:method,
    E_CONTENT_TYPE:responseType,
    callback[],
    data[] = "",
    Headers:headers = Headers:-1
);

// RequestJSON performs a request to send or receive JSON data
native Request:RequestJSON(
    Requests:id,
    const path[],
    E_HTTP_METHOD:method,
    E_CONTENT_TYPE:responseType,
    callback[],
    Node:json = Node:-1,
    Headers:headers = Headers:-1
);

// -
// JSON API
// -

enum JSON_NODE {
    JSON_NODE_NUMBER,
    JSON_NODE_BOOLEAN,
    JSON_NODE_STRING,
    JSON_NODE_OBJECT,
    JSON_NODE_ARRAY,
    JSON_NODE_NULL,
}

// JsonNodeType returns the type of a node from the above enumerator.
JSON_NODE:JsonNodeType(Node:node);

// JsonObject allocates a node from a set of key-value pairs where each key must
// be a string and each value must be a `Node:` value. For example:
//
// JsonObject("key", JsonString("value"));
//
// output: {"key": "value"}
//
// Returns a `Node:` ID which can be passed as an argument to another JsonObject
// function in order to build nested objects. For example:
//
// JsonObject("key", JsonObject("nestedKey", JsonString("value")));
//
// output: {"key": {"nestedKey": "value"}}
//
native Node:JsonObject({_, Node}:...);

// JsonInt, JsonBool, JsonFloat, JsonString each allocate a JSON node.
native Node:JsonInt(value);
native Node:JsonBool(bool:value);
native Node:JsonFloat(Float:value);
native Node:JsonString(const value[]);

// JsonArray simply takes an argument list of `Node:` IDs.
//
// JsonArray(JsonString("value"), JsonInt(1), JsonObject("k", JsonString("v")))
//
// output: ["value", 1, {"k": "v"}]
//
native Node:JsonArray(Node:...);

// JsonGetObject returns the `Node:` stored at `key` in the given `node`.
// For example:
//
// input: {"key": {"inner": 1}}
//
// new Node:output;
// JsonGetObject(node, "key", output);
//
// `output` now contains a JSON object containing {"inner": 1}, this node can be
// treated like any other node:
//
// new outputValue;
// JsonGetInt(output, outputValue);
// outputValue == 1
//
native JsonGetObject(Node:node, const key[], &Node:output);

// JsonGetObject* functions extract a native type from an object
// these functions are shorthand for:
//
// new Node:output;
// JsonGetObject(node, "key", output);
// new string[128];
// JsonGetString(output, string);
//
// 99% of the time, you only need these functions to get values out of objects.
//
native JsonGetInt(Node:node, const key[], &output);
native JsonGetFloat(Node:node, const key[], &Float:output);
native JsonGetBool(Node:node, const key[], &bool:output);
native JsonGetString(Node:node, const key[], output[], len = sizeof(output));

// JsonGetArray returns the `Node:` stored at `index` in the given `node`. The
// `Node:` returned could be an Object or a primitive, such as an int, float,
// bool or string. Use functions below to convert `Node:` into a native type.
// For example:
//
// input: {"key": [1, 2, 3]}
//
// new Node:output;
// JsonGetArray(node, key, output);
//
// `output` now contains a JSON array and can be accessed with:
//
// new Node:element;
// JsonArrayObject(node, 1, element);
//
// `element` now contains a JSON integer type node and can be converted to a
// native integer type using `JsonGetNodeInt`.
//
native JsonGetArray(Node:node, const key[], &Node:output);
native JsonArrayLength(Node:node, &length);
native JsonArrayObject(Node:node, index, &Node:output);

// JsonGetNode* functions extract a JSON object `Node:` to `output`.
// These are useful for when you get a `Node:` that represents a primitive type
// such as from JsonGetArray.
native JsonGetNodeInt(Node:node, &output);
native JsonGetNodeFloat(Node:node, &Float:output);
native JsonGetNodeBool(Node:node, &bool:output);
native JsonGetNodeString(Node:node, output[], len = sizeof(output));

// JsonStringify serialises a JSON node into its textual representation.
native JsonStringify(Node:node, buf[], len = sizeof(buf));

// -
// Internal
// -

// JsonCleanup is an internal function for cleaning up `Node:` objects. This is
// necessary because each of the object builder functions above allocate nodes
// in a pool to be passed between internal function calls.
native JsonCleanup(Node:node);

// cleans up nodes once they go out of scope.
stock operator~(Node:nodes[], len) {
    for(new i; i < len; ++i) {
        JsonCleanup(nodes[i]);
    }
}