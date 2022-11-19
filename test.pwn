#define RUN_TESTS

#include <a_samp>
#include <YSI_Core\y_testing>

#include "requests.inc"

main() {
    //
}

public OnRequestFailure(Request:id, errorCode, errorMessage[], len) {
    printf("Request %d failed with %d: '%s'", _:id, errorCode, errorMessage);
}

Test:NewClient() {
    new RequestsClient:client = RequestsClient("http://httpbin.org/");
    ASSERT(IsValidRequestsClient(client));
    printf("new requests client: %d", _:client);
}

Test:NewClientInvalid() {
    new RequestsClient:client = RequestsClient("ssh://httpbin.org/");
    ASSERT(!IsValidRequestsClient(client));
    printf("new requests client: %d", _:client);
}

Test:NewClientIP() {
    new RequestsClient:client = RequestsClient("1.1.1.1");
    ASSERT(!IsValidRequestsClient(client));
    printf("new requests client: %d", _:client);
}


// -
// Request - basic HTTP GET on basic text data
// -


new Request:OnGetData_ID;
Test:GetData() {
    new RequestsClient:client = RequestsClient("http://httpbin.org/", RequestHeaders());
    OnGetData_ID = Request(
        client,
        "robots.txt",
        HTTP_METHOD_GET,
        "OnGetData",
        .headers = RequestHeaders()
    );
}
forward OnGetData(Request:id, E_HTTP_STATUS:status, data[], dataLen);
public OnGetData(Request:id, E_HTTP_STATUS:status, data[], dataLen) {
    print("*** Test OnGetData\n");

    ASSERT(id == OnGetData_ID);
    ASSERT(status == HTTP_STATUS_OK);
    print(data);

    print("\nPASS!");
}

new Request:OnGetDataSSL_ID;
Test:GetDataSSL() {
    new RequestsClient:client = RequestsClient("https://httpbin.org/", RequestHeaders());
    OnGetDataSSL_ID = Request(
        client,
        "robots.txt",
        HTTP_METHOD_GET,
        "OnGetDataSSL",
        .headers = RequestHeaders()
    );
}
forward OnGetDataSSL(Request:id, E_HTTP_STATUS:status, data[], dataLen);
public OnGetDataSSL(Request:id, E_HTTP_STATUS:status, data[], dataLen) {
    print("*** Test OnGetDataSSL\n");

    ASSERT(id == OnGetDataSSL_ID);
    ASSERT(status == HTTP_STATUS_OK);
    print(data);

    print("\nPASS!");
}


// -
// Request - basic HTTP GET with headers test
// -


new Request:OnGetDataWithHeaders_ID;
Test:GetDataWithHeaders() {
    new RequestsClient:client = RequestsClient("http://httpbin.org/", RequestHeaders(
        "X-Pawn-Requests", "YES"
    ));
    OnGetDataWithHeaders_ID = Request(
        client,
        "headers",
        HTTP_METHOD_GET,
        "OnGetDataWithHeaders",
        .headers = RequestHeaders(
            "X-Pawn-Requests-Embedded", "YES"
        )
    );
}
forward OnGetDataWithHeaders(Request:id, E_HTTP_STATUS:status, data[], dataLen);
public OnGetDataWithHeaders(Request:id, E_HTTP_STATUS:status, data[], dataLen) {
    print("*** Test OnGetDataWithHeaders\n");

    ASSERT(id == OnGetDataWithHeaders_ID);
    ASSERT(status == HTTP_STATUS_OK);
    print(data);

    print("\nPASS!");
}

new Request:OnGetDataWithHeadersSSL_ID;
Test:GetDataWithHeadersSSL() {
    new RequestsClient:client = RequestsClient("https://httpbin.org/", RequestHeaders(
        "X-Pawn-Requests", "YES"
    ));
    OnGetDataWithHeadersSSL_ID = Request(
        client,
        "headers",
        HTTP_METHOD_GET,
        "OnGetDataWithHeadersSSL",
        .headers = RequestHeaders(
            "X-Pawn-Requests-Embedded", "YES"
        )
    );
}
forward OnGetDataWithHeadersSSL(Request:id, E_HTTP_STATUS:status, data[], dataLen);
public OnGetDataWithHeadersSSL(Request:id, E_HTTP_STATUS:status, data[], dataLen) {
    print("*** Test OnGetDataWithHeadersSSL\n");

    ASSERT(id == OnGetDataWithHeadersSSL_ID);
    ASSERT(status == HTTP_STATUS_OK);
    print(data);

    print("\nPASS!");
}

new Request:OnGetJsonWithHeaders_ID;
Test:GetJsonWithHeaders() {
    new RequestsClient:client = RequestsClient("http://httpbin.org/", RequestHeaders(
        "X-Pawn-Requests", "YES"
    ));
    OnGetJsonWithHeaders_ID = RequestJSON(
        client,
        "headers",
        HTTP_METHOD_GET,
        "OnGetJsonWithHeaders"
    );
}
forward OnGetJsonWithHeaders(Request:id, E_HTTP_STATUS:status, Node:node);
public OnGetJsonWithHeaders(Request:id, E_HTTP_STATUS:status, Node:node) {
    print("*** Test OnGetJsonWithHeaders\n");

    ASSERT(id == OnGetJsonWithHeaders_ID);
    ASSERT(status == HTTP_STATUS_OK);

    new string[512];
    JsonStringify(node, string);
    printf("%s", string);

    print("\nPASS!");
}


// -
// RequestJSON - basic GET on JSON data
// -


new Request:OnGetJson_ID;
Test:GetJson() {
    new RequestsClient:client = RequestsClient("http://httpbin.org/", RequestHeaders());
    OnGetJson_ID = RequestJSON(
        client,
        "anything",
        HTTP_METHOD_GET,
        "OnGetJson",
        .headers = RequestHeaders()
    );
}
forward OnGetJson(Request:id, E_HTTP_STATUS:status, Node:node);
public OnGetJson(Request:id, E_HTTP_STATUS:status, Node:node) {
    print("*** Test OnGetJson\n");

    ASSERT(id == OnGetJson_ID);
    ASSERT(status == HTTP_STATUS_OK);
    ASSERT(_:node != -1);

    new string[512];
    JsonStringify(node, string);
    printf("%s", string);

    new output[128];

    JsonGetString(node, "method", output);
    ASSERT(!strcmp(output, "GET"));

    JsonGetString(node, "origin", output);
    ASSERT(strlen(output) > 0);

    JsonGetString(node, "url", output);
    ASSERT(!strcmp(output, "http://httpbin.org/anything"));

    print("\nPASS!");
}


// -
// RequestJSON - failure cases
// -


Test:InvalidClient() {
    new Request:id = RequestJSON(
        RequestsClient:-1,
        "",
        HTTP_METHOD_GET,
        ""
    );
    ASSERT(!IsValidRequest(id));
}


// -
// WebSocket Tests
// -


new WebSocket:WebSocketEcho_ID;
Test:WebSocketEcho() {
    print("Connecting to WebSocket...");
    new start = GetTickCount();
    WebSocketEcho_ID = WebSocketClient("wss://echo.websocket.events", "OnWebSocket");
    printf("Connected! Took %dms", GetTickCount() - start);

    start = GetTickCount();
    printf("Sending WebSocket message...");
    new ret = WebSocketSend(WebSocketEcho_ID, "Hello World!");
    printf("Sent! Took %dms", GetTickCount() - start);

    ASSERT(ret == 0);
}
forward OnWebSocket(WebSocket:ws, const data[], dataLen);
public OnWebSocket(WebSocket:ws, const data[], dataLen) {
    print("*** Test OnWebSocket\n");

    printf("%d %d: '%s'", _:ws, dataLen, data);
    new ret = strcmp(data, "Hello World!", false, dataLen);
    ASSERT(!ret);

    print("\nPASS!");
}

new JsonWebSocket:JsonWebSocketEcho_ID;
Test:JsonWebSocketEcho() {
    print("Connecting to WebSocket...");
    new start = GetTickCount();
    JsonWebSocketEcho_ID = JsonWebSocketClient("wss://echo.websocket.events", "OnJsonWebSocket");
    printf("Connected! Took %dms", GetTickCount() - start);

    start = GetTickCount();
    printf("Sending WebSocket message...");
    new ret = JsonWebSocketSend(JsonWebSocketEcho_ID, JsonObject(
        "key1", JsonString("value1"),
        "key2", JsonString("value2"),
        "key3", JsonString("value3")
    ));
    printf("Sent! Took %dms", GetTickCount() - start);

    ASSERT(ret == 0);
}
forward OnJsonWebSocket(JsonWebSocket:ws, Node:node);
public OnJsonWebSocket(JsonWebSocket:ws, Node:node) {
    print("*** Test OnJsonWebSocket\n");

    new string[512];
    JsonStringify(node, string);
    ASSERT(!strcmp(string, "{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"}"));
    printf("%d '%s'", _:ws, string);

    print("\nPASS!");
}

Test:WebSocketFail() {
    new WebSocket:id = WebSocketClient("wss://example.com", "OnWebSocket");
    ASSERT(_:id == -1);
}

Test:JsonWebSocketFail() {
    new JsonWebSocket:id = JsonWebSocketClient("wss://example.com", "OnWebSocket");
    ASSERT(_:id == -1);
}


// -
// JSON Tests
// -


Test:JsonParse() {
    new Node:node;
    new ret;

    new input[] = "{\"list\":[{\"a_listobj_float\":66.599998474121094,\"a_listobj_number\":76,\"a_listobj_string\":\"another value\",\"one\":\"value one\"},{\"a_listobj_float\":66.599998474121094,\"a_listobj_number\":76,\"a_listobj_string\":\"another value\",\"two\":\"value two\"},{\"a_listobj_float\":66.599998474121094,\"a_listobj_number\":76,\"a_listobj_string\":\"another value\",\"three\":\"value three\"}],\"object\":{\"a_float\":66.599998474121094,\"a_number\":76,\"a_string\":\"a value\",\"nested_object\":{\"a_deeper_float\":66.599998474121094,\"a_deeper_number\":76,\"a_deeper_string\":\"another value\"}}}";

    ret = JsonParse(input, node);
    ASSERT(ret == 0);

    new output[1024];
    ret = JsonStringify(node, output);
    ASSERT(!strcmp(input, output));
}

Test:JsonNodeType() {
    new Node:number = JsonInt(3); // JSON_NODE_NUMBER
    ASSERT(JsonNodeType(number) ==  JSON_NODE_NUMBER);

    new Node:boolean = JsonBool(true); // JSON_NODE_BOOLEAN
    ASSERT(JsonNodeType(boolean) ==  JSON_NODE_BOOLEAN);

    new Node:string = JsonString("hi"); // JSON_NODE_STRING
    ASSERT(JsonNodeType(string) ==  JSON_NODE_STRING);

    new Node:object = JsonObject("k", JsonInt(1)); // JSON_NODE_OBJECT
    ASSERT(JsonNodeType(object) ==  JSON_NODE_OBJECT);

    new Node:array = JsonArray(JsonInt(1), JsonInt(2)); // JSON_NODE_ARRAY
    ASSERT(JsonNodeType(array) ==  JSON_NODE_ARRAY);

    new Node:null = Node:-1; // JSON_NODE_NULL
    ASSERT(JsonNodeType(null) ==  JSON_NODE_NULL);
}

Test:JsonObjectEmpty() {
    new Node:node = JsonObject();

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{}"));
}

Test:JsonObjectInt() {
    new Node:node = JsonObject(
        "key", JsonInt(1)
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":1}"));
    print(buf);
}

Test:JsonObjectInts() {
    new Node:node = JsonObject(
        "key1", JsonInt(1),
        "key2", JsonInt(2),
        "key3", JsonInt(3)
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key1\":1,\"key2\":2,\"key3\":3}"));
    print(buf);
}

Test:JsonObjectFloat() {
    new Node:node = JsonObject(
        "key", JsonFloat(1.5)
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":1.5}"));
    print(buf);
}

Test:JsonObjectFloats() {
    new Node:node = JsonObject(
        "key1", JsonFloat(1.5),
        "key2", JsonFloat(2.5),
        "key3", JsonFloat(3.5)
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key1\":1.5,\"key2\":2.5,\"key3\":3.5}"));
    print(buf);
}

Test:JsonObjectBool() {
    new Node:node = JsonObject(
        "key", JsonBool(true)
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":true}"));
    print(buf);
}

Test:JsonObjectBools() {
    new Node:node = JsonObject(
        "key1", JsonBool(false),
        "key2", JsonBool(true),
        "key3", JsonBool(false)
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key1\":false,\"key2\":true,\"key3\":false}"));
    print(buf);
}

Test:JsonObjectString() {
    new Node:node = JsonObject(
        "key", JsonString("value")
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":\"value\"}"));
    print(buf);
}

Test:JsonObjectStrings() {
    new Node:node = JsonObject(
        "key1", JsonString("value1"),
        "key2", JsonString("value2"),
        "key3", JsonString("value3")
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"}"));
    print(buf);
}

Test:JsonStringArray() {
    new Node:node = JsonArray(
        JsonString("one"),
        JsonString("two"),
        JsonString("three")
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "[\"one\",\"two\",\"three\"]"));
    print(buf);
}

Test:JsonIntArray() {
    new Node:node = JsonArray(
        JsonInt(1),
        JsonInt(2),
        JsonInt(3)
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "[1,2,3]"));
    print(buf);
}

Test:JsonFloatArray() {
    new Node:node = JsonArray(
        JsonFloat(1.5),
        JsonFloat(2.5),
        JsonFloat(3.5)
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "[1.5,2.5,3.5]"));
    print(buf);
}

Test:JsonObjectArray() {
    new Node:node = JsonArray(
        JsonObject(
            "one", JsonString("value one")
        ),
        JsonObject(
            "two", JsonString("value two")
        ),
        JsonObject(
            "three", JsonString("value three")
        )
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "[{\"one\":\"value one\"},{\"two\":\"value two\"},{\"three\":\"value three\"}]"));
    print(buf);
}

/*
JsonObjectComplex generates this rather complex JSON object:
{
  "object": {
    "a_float": 66.599998474121094,
    "a_number": 76,
    "a_string": "a value",
    "nested_object": {
      "a_deeper_float": 66.599998474121094,
      "a_deeper_number": 76,
      "a_deeper_string": "another value"
    }
  },
  "list": [
    {
      "a_listobj_float": 66.599998474121094,
      "a_listobj_number": 76,
      "a_listobj_string": "another value",
      "one": "value one"
    },
    {
      "a_listobj_float": 66.599998474121094,
      "a_listobj_number": 76,
      "a_listobj_string": "another value",
      "two": "value two"
    },
    {
      "a_listobj_float": 66.599998474121094,
      "a_listobj_number": 76,
      "a_listobj_string": "another value",
      "three": "value three"
    }
  ]
}
*/
Test:JsonObjectComplex() {
    new Node:node = JsonObject(
        "object", JsonObject(
            "a_string", JsonString("a value"),
            "a_number", JsonInt(76),
            "a_float", JsonFloat(66.6),
            "nested_object", JsonObject(
                "a_deeper_string", JsonString("another value"),
                "a_deeper_number", JsonInt(76),
                "a_deeper_float", JsonFloat(66.6)
            )
        ),
        "list", JsonArray(
            JsonObject(
                "one", JsonString("value one"),
                "a_listobj_string", JsonString("another value"),
                "a_listobj_number", JsonInt(76),
                "a_listobj_float", JsonFloat(66.6)
            ),
            JsonObject(
                "two", JsonString("value two"),
                "a_listobj_string", JsonString("another value"),
                "a_listobj_number", JsonInt(76),
                "a_listobj_float", JsonFloat(66.6)
            ),
            JsonObject(
                "three", JsonString("value three"),
                "a_listobj_string", JsonString("another value"),
                "a_listobj_number", JsonInt(76),
                "a_listobj_float", JsonFloat(66.6)
            )
        )
    );

    new buf[1024];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"list\":[{\"a_listobj_float\":66.599998474121094,\"a_listobj_number\":76,\"a_listobj_string\":\"another value\",\"one\":\"value one\"},{\"a_listobj_float\":66.599998474121094,\"a_listobj_number\":76,\"a_listobj_string\":\"another value\",\"two\":\"value two\"},{\"a_listobj_float\":66.599998474121094,\"a_listobj_number\":76,\"a_listobj_string\":\"another value\",\"three\":\"value three\"}],\"object\":{\"a_float\":66.599998474121094,\"a_number\":76,\"a_string\":\"a value\",\"nested_object\":{\"a_deeper_float\":66.599998474121094,\"a_deeper_number\":76,\"a_deeper_string\":\"another value\"}}}"));
    print(buf);
}

Test:JsonAppendObject() {
    new Node:a = JsonObject(
        "key1", JsonString("value1"),
        "key2", JsonString("value2")
    );
    new Node:b = JsonObject(
        "key3", JsonString("value3")
    );

    new Node:c = JsonAppend(a, b);

    new buf[128];
    new ret = JsonStringify(c, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"}"));
    print(buf);
}

Test:JsonAppendArray() {
    new Node:a = JsonArray(
        JsonInt(1),
        JsonInt(2)
    );
    new Node:b = JsonArray(
        JsonInt(3)
    );

    new Node:c = JsonAppend(a, b);

    new buf[128];
    new ret = JsonStringify(c, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "[1,2,3]"));
    print(buf);
}

Test:JsonSetObject() {
    new Node:node = JsonObject();
    new ret = JsonSetObject(node, "key", JsonObject("key", JsonString("value")));
    ASSERT(ret == 0);

    new buf[128];
    ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":{\"key\":\"value\"}}"));
    print(buf);
}

Test:JsonSetInt() {
    new Node:node = JsonObject();
    new ret = JsonSetInt(node, "key", 5);
    ASSERT(ret == 0);

    new buf[128];
    ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":5}"));
    print(buf);
}

Test:JsonSetFloat() {
    new Node:node = JsonObject();
    new ret = JsonSetFloat(node, "key", 5.5);
    ASSERT(ret == 0);

    new buf[128];
    ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":5.5}"));
    print(buf);
}

Test:JsonSetBool() {
    new Node:node = JsonObject();
    new ret = JsonSetBool(node, "key", true);
    ASSERT(ret == 0);

    new buf[128];
    ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":true}"));
    print(buf);
}

Test:JsonSetString() {
    new Node:node = JsonObject();
    new ret = JsonSetString(node, "key", "value");
    ASSERT(ret == 0);

    new buf[128];
    ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key\":\"value\"}"));
    print(buf);
}

Test:JsonGetInt() {
    new Node:node = JsonObject(
        "key1", JsonInt(1),
        "key2", JsonInt(2),
        "key3", JsonInt(3)
    );

    new got;
    new ret;

    ret = JsonGetInt(node, "key1", got);
    ASSERT(ret == 0);
    ASSERT(got == 1);

    ret = JsonGetInt(node, "key2", got);
    ASSERT(ret == 0);
    ASSERT(got == 2);

    ret = JsonGetInt(node, "key3", got);
    ASSERT(ret == 0);
    ASSERT(got == 3);

    ret = JsonGetInt(node, "key4", got);
    ASSERT(ret == 2);
}

Test:JsonGetFloat() {
    new Node:node = JsonObject(
        "key1", JsonFloat(1.5),
        "key2", JsonFloat(2.5),
        "key3", JsonFloat(3.5)
    );

    new Float:got;
    new ret;

    ret = JsonGetFloat(node, "key1", got);
    ASSERT(ret == 0);
    ASSERT(got == 1.5);

    ret = JsonGetFloat(node, "key2", got);
    ASSERT(ret == 0);
    ASSERT(got == 2.5);

    ret = JsonGetFloat(node, "key3", got);
    ASSERT(ret == 0);
    ASSERT(got == 3.5);

    ret = JsonGetFloat(node, "key4", got);
    ASSERT(ret == 2);
}

Test:JsonGetBool() {
    new Node:node = JsonObject(
        "key1", JsonBool(false),
        "key2", JsonBool(true),
        "key3", JsonBool(false)
    );

    new bool:got;
    new ret;

    ret = JsonGetBool(node, "key1", got);
    ASSERT(ret == 0);
    ASSERT(got == false);

    ret = JsonGetBool(node, "key2", got);
    ASSERT(ret == 0);
    ASSERT(got == true);

    ret = JsonGetBool(node, "key3", got);
    ASSERT(ret == 0);
    ASSERT(got == false);

    ret = JsonGetBool(node, "key4", got);
    ASSERT(ret == 2);
}

Test:JsonGetString() {
    new Node:node = JsonObject(
        "key1", JsonString("value1"),
        "key2", JsonString("value2"),
        "key3", JsonString("value3")
    );

    new got[128];
    new ret;

    ret = JsonGetString(node, "key1", got);
    ASSERT(ret == 0);
    ASSERT(!strcmp(got, "value1"));

    ret = JsonGetString(node, "key2", got);
    ASSERT(ret == 0);
    ASSERT(!strcmp(got, "value2"));

    ret = JsonGetString(node, "key3", got);
    ASSERT(ret == 0);
    ASSERT(!strcmp(got, "value3"));

    ret = JsonGetString(node, "key4", got);
    ASSERT(ret == 2);
}

Test:JsonGetArray() {
    new Node:node = JsonObject(
        "key1", JsonArray(
            JsonString("one"),
            JsonString("two"),
            JsonString("three")
        )
    );

    new Node:arrayNode;
    new ret;

    ret = JsonGetArray(node, "key1", arrayNode);
    printf("JsonGetArray:%d arrayNode: %d", ret, _:arrayNode);
    ASSERT(ret == 0);

    new Node:output;
    new gotString[32];

    ret = JsonArrayObject(arrayNode, 0, output);
    ASSERT(ret == 0);
    ret = JsonGetNodeString(output, gotString);
    ASSERT(ret == 0);
    ASSERT(!strcmp(gotString, "one"));

    ret = JsonArrayObject(arrayNode, 1, output);
    ASSERT(ret == 0);
    ret = JsonGetNodeString(output, gotString);
    ASSERT(ret == 0);
    ASSERT(!strcmp(gotString, "two"));

    ret = JsonArrayObject(arrayNode, 2, output);
    ASSERT(ret == 0);
    ret = JsonGetNodeString(output, gotString);
    ASSERT(ret == 0);
    ASSERT(!strcmp(gotString, "three"));
}

Test:JsonGetIntInvalid() {
    new Node:node = JsonObject("k", JsonString("v"));
    new gotInt;
    new ret = JsonGetInt(node, "key4", gotInt);
    ASSERT(ret == 2);
}

Test:JsonGetFloatInvalid() {
    new Node:node = JsonObject("k", JsonString("v"));
    new Float:gotFloat;
    new ret = JsonGetFloat(node, "key4", gotFloat);
    ASSERT(ret == 2);
}

Test:JsonGetBoolInvalid() {
    new Node:node = JsonObject("k", JsonString("v"));
    new bool:gotBool;
    new ret = JsonGetBool(node, "key4", gotBool);
    ASSERT(ret == 2);
}

Test:JsonGetStringInvalid() {
    new Node:node = JsonObject("k", JsonString("v"));
    new gotString[1];
    new ret = JsonGetString(node, "key4", gotString);
    ASSERT(ret == 2);
}

Test:JsonGetArrayInvalid() {
    new Node:node = JsonObject("k", JsonString("v"));
    new Node:gotNode;
    new ret = JsonGetArray(node, "key4", gotNode);
    ASSERT(ret == 2);
}

Test:JsonArrayLength() {
    new Node:node = JsonArray(
        JsonString("one"),
        JsonString("two"),
        JsonString("three")
    );

    new length;
    new ret;
    ret = JsonArrayLength(node, length);
    printf("ret %d length %d", ret, length);
    ASSERT(ret == 0);
    ASSERT(length == 3);
}

Test:JsonArrayObject() {
    new Node:node = JsonArray(
        JsonString("one"),
        JsonString("two"),
        JsonString("three")
    );

    new Node:output;
    new ret;
    ret = JsonArrayObject(node, 1, output);
    ASSERT(ret == 0);

    new got[32];
    ret = JsonGetNodeString(output, got);
    ASSERT(ret == 0);
    ASSERT(!strcmp(got, "two"));
}

Test:JsonGetNodeInt() {
    new Node:node = JsonObject(
        "key", JsonInt(1)
    );

    new Node:output;
    new ret;
    ret = JsonGetObject(node, "key", output);
    ASSERT(ret == 0);

    new got;
    ret = JsonGetNodeInt(output, got);
    ASSERT(ret == 0);
    ASSERT(got == 1);
}

Test:JsonGetNodeFloat() {
    new Node:node = JsonObject(
        "key", JsonFloat(1.34)
    );

    new Node:output;
    new ret;
    ret = JsonGetObject(node, "key", output);
    ASSERT(ret == 0);

    new Float:got;
    ret = JsonGetNodeFloat(output, got);
    ASSERT(ret == 0);
    ASSERT(got == 1.34);
}

Test:JsonGetNodeBool() {
    new Node:node = JsonObject(
        "key", JsonBool(true)
    );

    new Node:output;
    new ret;
    ret = JsonGetObject(node, "key", output);
    ASSERT(ret == 0);

    new bool:got;
    ret = JsonGetNodeBool(output, got);
    ASSERT(ret == 0);
    ASSERT(got == true);
}

Test:JsonGetNodeString() {
    new Node:node = JsonObject(
        "key", JsonString("value")
    );

    new Node:output;
    new ret;
    ret = JsonGetObject(node, "key", output);
    ASSERT(ret == 0);

    new got[32];
    ret = JsonGetNodeString(output, got);
    ASSERT(ret == 0);
    ASSERT(!strcmp(got, "value"));
}

// Test:JsonScopeGC() {
//     new Node:node = JsonObject();
//     scopeNodeGC(node);
//     ASSERT(JsonCleanup(node) == 1);
// }

Test:JsonToggleGC() {
    new Node:node = JsonObject(
        "key", JsonString("value")
    );
    JsonToggleGC(node, false);
    scopeNodeGC(node);
    new value[6];
    JsonGetString(node, "key", value);
    ASSERT(!strcmp(value, "value"));
    ASSERT(JsonCleanup(node) == 0);
    ASSERT(JsonCleanup(node) == 1);
}

scopeNodeGC(Node:node) {
    printf("scoped %d", _:node);
}
