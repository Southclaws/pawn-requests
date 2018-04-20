// restful provides an API for interacting with RESTful HTTP(S) JSON APIs.

// Client initialisation
native Restful:RestfulClient(const endpoint[], Headers:headers = Headers:-1);

// text data
native RestfulGetData(Restful:id, const path[], callback[], Headers:headers = Headers:-1);
native RestfulPostData(Restful:id, const path[], callback[], data[], Headers:headers = Headers:-1);

// JSON nodes
native RestfulGetJSON(Restful:id, const path[], callback[], Headers:headers = Headers:-1);
native RestfulPostJSON(Restful:id, const path[], callback[], Node:json, Headers:headers = Headers:-1);

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
