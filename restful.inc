// restful provides an API for interacting with RESTful HTTP(S) JSON APIs.


#include "status_codes"

// Client initialisation
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

// Headers builder
native Headers:RestfulHeaders(...);

// JSON
native Node:JsonObject({_, Node}:...);
native Node:JsonString(const value[]);
native Node:JsonInt(value);
native Node:JsonFloat(Float:value);
native Node:JsonArray(Node:...);
native JsonStringify(Node:node, buf[], len = sizeof(buf));
native JsonCleanup(Node:node);

// cleans up nodes once they go out of scope.
stock operator~(Node:nodes[], len) {
    for(new i; i < len; ++i) {
        JsonCleanup(nodes[i]);
    }
}
