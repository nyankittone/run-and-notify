// I hope I don't write shitty code here, bleh

#include <stddef.h>
#include <string_stuff.h>

// Struct that contains a single range
typedef struct {
    int min, max;
    _Bool include;
} NumberRange;

// Struct thaqt acts as the "head" of an array of NumberRanges somewhere on the heap. A pointer to
// the array is *not* kept here; I've opted to have this struct be stored adjacent to the actual
// array in memory. :3
typedef struct {
    const int min, max;
    size_t length, capacity;
} NumberRangeCollection;

// Struct that acts as some state for an iterator to iterate over a string for individual number
// ranges.
typedef struct {
    const int min, max;
    char *string;
    size_t length;
} NumberRangeIterator;

// This is returned by the function that uses a `NumberRangeIterator` struct. It's composed of
// simply a `NumberRange` and an enum for tracking if an error occured.
typedef struct {
    enum {
        RANGE_ITER_SUCCESS,
        RANGE_ITER_HIT_END,
        RANGE_ITER_FAIL_IDK,
        RANGE_ITER_NULL_PASSED,
    } error;
    NumberRange range;
} RangeIterationResult;

typedef enum {
    RANGE_ITER_CREATE_SUCCEEDED,
    RANGE_ITER_CREATE_MIN_AND_MAX_SWAPPED,
    RANGE_ITER_CREATE_NULL_PASSED,
} NewRangeIteratorError;

// Creates a new iterator for, uhh, iterating over a string for ranges. The string's length is
// passed explicitly instead of being implied from a null byte.
NumberRangeIterator newRangeIterator (
    char *const string, size_t length, const int min, const int max, NewRangeIteratorError *error
);

// Performs a single iteration on a `NumberRangeIterator`.
RangeIterationResult iterateRangeString(NumberRangeIterator *const iter);

// Generates a `NumberRangeCollection` object. NULL is returned on failure.
// This function panics the app if it there's insufficient memory.
NumberRangeCollection *makeRangeCollection (
    char *const string, size_t length, const int min, const int max
);

// Returns a boolean based on whether or not a number passed is within any of the specified ranges.
// If the `ranges` pointer is NULL, then `false` is returned.
_Bool queryRangeCollection(NumberRangeCollection *const ranges, const int value);

