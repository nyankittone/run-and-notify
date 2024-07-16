#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <number_range.h>
#include <bool_string.h>

int main(int argc, char *argv[]) {
    if(argc < 2) return 1;

    NumberRangeIterator iter = newRangeIterator (
        argv[1],
        strlen(argv[1]),
        argc < 3 ? 0 : atoi(argv[2]),
        argc < 4 ? 255 : atoi(argv[3]),
        NULL
    );

    for (
        RangeIterationResult result;
        (result = iterateRangeString(&iter, NULL)).error != RANGE_ITER_HIT_END;
    ) {
        if(result.error) {
            puts("parsing error");
            continue;
        }

        printf (
            "from: %d, to: %d, invert: %s\n",
            result.range.from,
            result.range.to,
            mBoolStr(!result.range.include)
        );
    }

    return 0;
}

