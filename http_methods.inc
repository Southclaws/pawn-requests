// http_methods.inc contains an enumerated set of HTTP request methods

enum E_HTTP_METHOD {
    HTTP_METHOD_GET,     // Requests a representation of the specified resource. Requests using GET should only retrieve data.
    HTTP_METHOD_HEAD,    // Asks for a response identical to that of a GET request, but without the response body.
    HTTP_METHOD_POST,    // Is used to submit an entity to the specified resource, often causing a change in state or side effects on the server
    HTTP_METHOD_PUT,     // Replaces all current representations of the target resource with the request payload.
    HTTP_METHOD_DELETE,  // Deletes the specified resource.
    HTTP_METHOD_CONNECT, // Establishes a tunnel to the server identified by the target resource.
    HTTP_METHOD_OPTIONS, // Is used to describe the communication options for the target resource.
    HTTP_METHOD_TRACE,   // Performs a message loop-back test along the path to the target resource.
    HTTP_METHOD_PATCH,   // Is used to apply partial modifications to a resource.
}
