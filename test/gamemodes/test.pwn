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
        "key2", JsonString("value1"),
        "key3", JsonString("value1")
    );

    new buf[128];
    new ret = JsonStringify(node, buf);
    ASSERT(ret == 0);
    ASSERT(!strcmp(buf, "{\"key1\":\"value1\",\"key2\":\"value1\",\"key3\":\"value1\"}"));
    print(buf);
}
