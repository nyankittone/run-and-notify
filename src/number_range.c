#include <number_range.h>
#include <error_array.h>

NumberRangeIterator newRangeIterator (
    char *const string, size_t length, const int min, const int max, NewRangeIteratorError *error
) {
    NewRangeIteratorError returned_error = RANGE_ITER_CREATE_SUCCEEDED;
    if(!string) returned_error |= RANGE_ITER_CREATE_NULL_PASSED;
    if(min > max) returned_error |= RANGE_ITER_CREATE_MIN_AND_MAX_SWAPPED;

    if(returned_error) {
        if(error) *error = returned_error;
        return (NumberRangeIterator) {0};
    }

    if(error) *error = returned_error;
    return (NumberRangeIterator) {
        .min = min,
        .max = max,
        .string = string,
        .length = length,
    };
}

RangeIterationResult iterateRangeString(NumberRangeIterator *const iter) {
    // this is where all the magic happens >:3
    if(!iter) return (RangeIterationResult) {RANGE_ITER_NULL_PASSED, {0}};
    
}

