#define RUN_TESTS

#include <a_samp>
#include <YSI\y_testing>

#include "../../restful.inc"

main() {
    new Node:node = JsonObject(
    );

    printf("node: %d", _:node);

    new buf[128];
    new ret = JsonStringify(node, buf);
    printf("ret: %d", ret);
    printf("buf: '%s'", buf);
}

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
