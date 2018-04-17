#define RUN_TESTS

#include <a_samp>
#include <YSI\y_testing>

#include "../../restful.inc"

main() {
    JsonObject(
        "AAA", 777,
        "key2", JsonString("value"),
        "key3", JsonObject(
            "key1", JsonNumber(1.2)
        )
    );
}
