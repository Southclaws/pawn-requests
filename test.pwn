#define RUN_TESTS

#include <a_samp>
#include <YSI\y_testing>

#include "restful.inc"

main() {
    //
}

Test:NewClient() {
    new Restful:client = RestfulClient("https://httpbin.org/");
    printf("new restful client: %d", _:client);
}

new Request:OnNewClientWithHeaders_ID;
Test:NewClientWithHeaders() {
    new Restful:client = RestfulClient("https://httpbin.org/", RestfulHeaders(
        "X-Pawn-Restful", "YES"
    ));
    OnNewClientWithHeaders_ID = RestfulGetData(client, "headers", "OnNewClientWithHeaders", RestfulHeaders(
        "X-Pawn-Restful-Embedded", "YES"
    ));
}
forward OnNewClientWithHeaders(Request:id, E_HTTP_STATUS:status, data[], dataLen);
public OnNewClientWithHeaders(Request:id, E_HTTP_STATUS:status, data[], dataLen) {
    ASSERT(id == OnNewClientWithHeaders_ID);
    ASSERT(status == HTTP_STATUS_OK);
    ASSERT(dataLen == 186);
    print(data);
}

#endinput

Test:JsonObjectEmpty() {
    new Node:node = JsonObject();

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{}"));
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
