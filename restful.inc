// restful provides an API for interacting with RESTful HTTP(S) JSON APIs.

// text data
native RestfulGetData(const endpoint[], callback[], Headers:headers);
native RestfulPostData(const endpoint[], callback[], data[], Headers:headers);

// JSON nodes
native RestfulGetJSON(const endpoint[], callback[], Headers:headers);
native RestfulPostJSON(const endpoint[], callback[], Node:json, Headers:headers);

// headers builder
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
operator~(Node:nodes[], len) {
    for(new i; i < len; ++i) {
        JsonCleanup(nodes[i]);
    }
}
