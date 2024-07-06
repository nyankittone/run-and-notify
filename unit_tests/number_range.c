#include <stdio.h>
#include <string.h>

#include <number_range.h>
#include <bool_string.h>

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
            mBoolStr(result.error)
        );
    }

    return 0;
}

