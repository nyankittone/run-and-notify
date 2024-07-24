#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <number_range.h>
#include <bool_string.h>

const char *const getBased(const int value) {
    switch(value) {
        case RANGE_ABSOLUTE:
            return "abs";
        case RANGE_START:
            return "start";
        case RANGE_END:
            return "end";
        default:
            return "idk";
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) return 1;

    NumberRangeIterator iter = newRangeIterator(argv[1], strlen(argv[1]));

    for (
        RangeIterationResult result;
        (result = iterateRangeString(&iter, NULL)).error != RANGE_ITER_HIT_END;
    ) {
        if(result.error) {
            puts("parsing error");
            continue;
        }

        printf (
            "from: %s(%d), to: %s(%d), invert: %s\n",
            getBased(result.range.from.based),
            result.range.from.offset,
            getBased(result.range.to.based),
            result.range.to.offset,
            mBoolStr(result.range.invert)
        );
    }

    return 0;
}

