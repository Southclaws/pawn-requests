#define RUN_TESTS

#include <a_samp>
#include <YSI\y_testing>

#include "restful.inc"

main() {
    //
}

Test:NewClient() {
    new Restful:client = RestfulClient("http://httpbin.org/");
    printf("new restful client: %d", _:client);
}


// -
// RestfulGetData - basic HTTP GET on basic text data
// -


new Request:OnGetData_ID;
Test:GetData() {
    new Restful:client = RestfulClient("http://httpbin.org/", RestfulHeaders());
    OnGetData_ID = RestfulGetData(client, "robots.txt", "OnGetData", RestfulHeaders());
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
    new Restful:client = RestfulClient("https://httpbin.org/", RestfulHeaders());
    OnGetDataSSL_ID = RestfulGetData(client, "robots.txt", "OnGetDataSSL", RestfulHeaders());
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
// RestfulGetData - basic HTTP GET with headers test
// -


new Request:OnGetDataWithHeaders_ID;
Test:GetDataWithHeaders() {
    new Restful:client = RestfulClient("http://httpbin.org/", RestfulHeaders(
        "X-Pawn-Restful", "YES"
    ));
    OnGetDataWithHeaders_ID = RestfulGetData(client, "headers", "OnGetDataWithHeaders", RestfulHeaders(
        "X-Pawn-Restful-Embedded", "YES"
    ));
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
    new Restful:client = RestfulClient("https://httpbin.org/", RestfulHeaders(
        "X-Pawn-Restful", "YES"
    ));
    OnGetDataWithHeadersSSL_ID = RestfulGetData(client, "headers", "OnGetDataWithHeadersSSL", RestfulHeaders(
        "X-Pawn-Restful-Embedded", "YES"
    ));
}
forward OnGetDataWithHeadersSSL(Request:id, E_HTTP_STATUS:status, data[], dataLen);
public OnGetDataWithHeadersSSL(Request:id, E_HTTP_STATUS:status, data[], dataLen) {
    print("*** Test OnGetDataWithHeadersSSL\n");

    ASSERT(id == OnGetDataWithHeadersSSL_ID);
    ASSERT(status == HTTP_STATUS_OK);
    print(data);

    print("\nPASS!");
}


// -
// JSON Tests
// -


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
