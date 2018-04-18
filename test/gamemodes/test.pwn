#define RUN_TESTS

#include <a_samp>
#include <YSI\y_testing>

#include "../../restful.inc"

main() {
    new Node:node = JsonObject(
        "AAA", JsonNumber(777),
        "key2", JsonString("value"),
        "key3", JsonObject(
            "key1", JsonNumber(1.2)
        )
    );

    printf("node: %d", node);

    new buf[128];
    new ret = JsonStringify(node, buf);
    printf("ret: %d", ret);
    printf("buf: '%s'", buf);
}
