#include <stdio.h>
#include <string.h>

#include <number_range.h>
#include <bool_string.h>

const char *const getError(int thing) {
    switch(thing) {
        case RANGE_ITER_SUCCESS:
            return "none";
        case RANGE_ITER_HIT_END:
            return "hit end";
        case RANGE_ITER_NULL_PASSED:
            return "null passed";
        case RANGE_ITER_FAIL:
            return "parsing error";
        default:
            return "unknown error";
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) return 1;

    NumberRangeIterator iter = newRangeIterator(argv[1], strlen(argv[1]), 0, 255, NULL);
    for (
        RangeIterationResult result;
        (result = iterateRangeString(&iter, NULL)).error != RANGE_ITER_HIT_END;
    ) {
        printf (
            "from: %d, to: %d, invert: %s, error: %s\n",
            result.range.from,
            result.range.to,
            mBoolStr(!result.range.include),
            getError(result.error)
        );
    }

    return 0;
}

