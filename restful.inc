// restful provides an API for interacting with RESTful HTTP(S) JSON APIs.


#include "status_codes"


// RestfulClient initialises a new Restful client with an endpoint. The endpoint
// must include a scheme (http or https) and must not contain a path. Any
// headers set on the client are sent with every request from the client but may
// be overwritten on a per-request basis.
native Restful:RestfulClient(const endpoint[], Headers:headers = Headers:-1);

// -
// Callback information:
// Response callback signatures are dependent on "Content-Type":
// - text/plain, application/octet-stream:
//   (Request:id, E_HTTP_STATUS:status, data[], dataLen)
// - application/json
//   (Request:id, E_HTTP_STATUS:status, Node:json)
// If the expected content 
// -

// -
// Binary/text/generic data
// -

// RestfulGetData performs a HTTP GET to the Restful client's endpoint+path.
native Request:RestfulGetData(Restful:id, const path[], callback[], Headers:headers = Headers:-1);

// RestfulPostData performs a HTTP POST to the Restful client's endpoint+path.
native Request:RestfulPostData(Restful:id, const path[], callback[], data[], Headers:headers = Headers:-1);

// -
// JSON nodes
// -

// RestfulGetJSON performs a HTTP GET to the Restful client's endpoint+path.
native Request:RestfulGetJSON(Restful:id, const path[], callback[], Headers:headers = Headers:-1);

// RestfulGetJSON performs a HTTP POST to the Restful client's endpoint+path.
native Request:RestfulPostJSON(Restful:id, const path[], callback[], Node:json, Headers:headers = Headers:-1);

// RestfulHeaders constructs a headers object and returns a handle ID. This
// function is designed to only ever be used as an argument to functions that
// take `Headers:` values as those functions handle the garbage collection of
// resources allocated on the heap.
native Headers:RestfulHeaders(...);

// -
// JSON
// -

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

// JsonString, JsonInt, JsonFloat each allocate a JSON node.
native Node:JsonString(const value[]);
native Node:JsonInt(value);
native Node:JsonFloat(Float:value);

// JsonArray simply takes an argument list of `Node:` IDs.
//
// JsonArray(JsonString("value"), JsonInt(1), JsonObject("k", JsonString("v")))
//
// output: ["value", 1, {"k": "v"}]
//
native Node:JsonArray(Node:...);

// JsonStringify serialises a JSON node into its textual representation.
native JsonStringify(Node:node, buf[], len = sizeof(buf));

// JsonGetObject returns the `Node:` stored at `key` in the given `node`.
// For example:
//
// input: {"key": "value"}
//
// new Node:node = JsonGetObject(node, "key");
//
// `node` now contains a JSON string containing "value" and can be extracted by:
//
// new output[128];
// JsonGetString(node, output);
//
native Node:JsonGetObject(Node:node, const key[]);

// JsonGet* functions extract a JSON object `Node:` to `output`.
native JsonGetString(Node:node, output[], len = sizeof(output));
native JsonGetInt(Node:node, &output);
native JsonGetFloat(Node:node, &Float:output);

// JsonGetArray returns the `Node:` stored at `index` in the given `node`.
// For example:
//
// input: [1, 2, 3]
//
// new Node:node = JsonGetArray(node, 1);
//
// `node` now contains a JSON number containing 1 and can be extracted by:
//
// new output;
// JsonGetInt(node, output);
//
native Node:JsonGetArray(Node:node, index);

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
