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

#include "status_codes"
#include "http_methods"


// -
// Request API
// -

// OnRequestFailure is only called when there is some internal failure/exception
forward OnRequestFailure(Request:id, errorCode, errorMessage[], len);

// RequestsClient initialises a new Requests client with an endpoint. The endpoint
// must include a scheme (http or https) and must not contain a path. Any
// headers set on the client are sent with every request from the client but may
// be overwritten on a per-request basis.
native RequestsClient:RequestsClient(const endpoint[], Headers:headers = Headers:-1);

// RequestHeaders constructs a headers object and returns a handle ID. This
// function is designed to only ever be used as an argument to functions that
// take `Headers:` values as those functions handle the garbage collection of
// resources allocated on the heap.
native Headers:RequestHeaders(...);

// Request performs a request to send or receive text data
native Request:Request(
    RequestsClient:id,
    const path[],
    E_HTTP_METHOD:method,
    callback[],
    body[] = "",
    Headers:headers = Headers:-1
);

// RequestJSON performs a request to send or receive JSON data
native Request:RequestJSON(
    RequestsClient:id,
    const path[],
    E_HTTP_METHOD:method,
    callback[],
    Node:json = Node:-1,
    Headers:headers = Headers:-1
);

// WebSocketClient creates a websocket client and establishes a connection at
// specified address. Received messages are sent to the callback which must have
// the signature: (WebSocket:ws, const data[], dataLen)
native WebSocket:WebSocketClient(const address[], const callback[]);
native WebSocketSend(WebSocket:ws, const data[]);

// JsonWebSockets are the same as above but the callback signature is:
// (JsonWebSocket:jws, Node:node)
native JsonWebSocket:JsonWebSocketClient(const address[], const callback[]);
native JsonWebSocketSend(JsonWebSocket:ws, Node:node);

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

// JsonParse decodes JSON and stores the root node into `output`.
native JsonParse(const string[], &Node:output);

// JsonStringify encodes a JSON node into `buf`.
native JsonStringify(Node:node, buf[], len = sizeof(buf));

// JsonNodeType returns the type of a node from the above enumerator.
native JSON_NODE:JsonNodeType(Node:node);

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

// JsonAppend returns a new `Node:` which is the result of appending b to a.
// This works on both objects and arrays and the two input nodes will be deleted
// from the global node store. For example:
//
// new Node:a = JsonObject("key1", JsonString("value"));
// new Node:b = JsonObject("key2", JsonString("value"));
// new Node:c = JsonAppend(a, b);
//
// output: {"key1": "value", "key2": "value"}
//
// new Node:a = JsonArray(JsonInt(1), JsonInt(2));
// new Node:a = JsonArray(JsonInt(3));
// new Node:c = JsonAppend(a, b);
//
// output: [1, 2, 3]
//
native Node:JsonAppend(Node:a, Node:b);
native Node:operator+(Node:a, Node:b) = JsonAppend;

// JsonSet* functions directly modify nodes by inserting or modifying keys.
native JsonSetObject(Node:node, const key[], Node:object);
native JsonSetInt(Node:node, const key[], output);
native JsonSetFloat(Node:node, const key[], Float:output);
native JsonSetBool(Node:node, const key[], bool:output);
native JsonSetString(Node:node, const key[], output[], len = sizeof(output));

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

// JsonGet* functions extract a native type from an object these functions are
// shorthand for:
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

// JsonToggleGC toggles garbage collection for a node. This prevents
// `JsonCleanup` from deleting nodes if `auto` is true. In other words,
// disabling garbage collection for a node will prevent it from being deleted
// automatically when it leaves scope. This is useful for when you want to pass
// a node through function calls or store it for a longer period of time.
// Be very careful with this function as losing a node pointer will result in a
// classic memory leak. For example:
//
// new Node:n = JsonObject();
// JsonToggleGC(n, false);
// CallLocalFunction("FillJsonObject", "d", _:n);
// JsonToggleGC(n, true);
//
// This will ensure that each hook of `FillJsonObject` does not delete `n` when
// it leaves scope.
//
native JsonToggleGC(Node:node, bool:toggle);

// -
// Internal
// -

// JsonCleanup is an internal function for cleaning up `Node:` objects. This is
// necessary because each of the object builder functions above allocate nodes
// in a pool to be passed between internal function calls. If called manually,
// leave `auto` as the default value of false which will ignore a garbage
// collection disable done with `JsonToggleGC`.
native JsonCleanup(Node:node, auto = false);

// cleans up nodes once they go out of scope.
stock operator~(const Node:nodes[], len) {
    for(new i; i < len; ++i) {
        JsonCleanup(nodes[i], true);
    }
}

// -
// Helpers
// -

// IsValidRequestsClient is for checking if the client ID returned by
// `RequestsClient` is valid and safe for use.
stock bool:IsValidRequestsClient(RequestsClient:id) {
    return (id >= RequestsClient:0);
}

// IsValidRequest is for checking if the ID returned by `Request` or
// `RequestJSON` is valid. If it is invalid, no request was sent.
stock bool:IsValidRequest(Request:id) {
    return (id >= Request:0);
}
