// I hope I don't write shitty code here, bleh

#include <stddef.h>
#include <string_stuff.h>
#include <error_array.h>

typedef struct {
    enum {
        RANGE_ABSOLUTE,
        RANGE_START,
        RANGE_END,
    } based;
    int offset;
} NumberRangePoint;

// Struct that contains a single range
// NOTE: Info on the minimum and maximum values for the range are *not* contained here. Those need
// to be tracked seperately.
typedef struct {
    NumberRangePoint from, to;
    _Bool invert;
} NumberRange;

// Struct that acts as the "head" of an array of NumberRanges somewhere on the heap. A pointer to
// the array is *not* kept here; I've opted to have this struct be stored adjacent to the actual
// array in memory. :3
typedef struct {
    int min, max;
    size_t length, capacity;
} NumberRangeCollection;

// Struct that acts as some state for an iterator to iterate over a string for individual number
// ranges.
typedef struct {
    char *string;
    const char *const og_string;
    size_t length;
    const size_t og_length;
} NumberRangeIterator;

// This is returned by the function that uses a `NumberRangeIterator` struct. It's composed of
// simply a `NumberRange` and an enum for tracking if an error occured.
typedef struct {
    enum {
        RANGE_ITER_SUCCESS,
        RANGE_ITER_HIT_END,
        RANGE_ITER_NULL_PASSED,
        RANGE_ITER_FAIL,
    } error;
    NumberRange range;
} RangeIterationResult;

// Creates a new iterator for, uhh, iterating over a string for ranges. The string's length is
// passed explicitly instead of being implied from a null byte.
NumberRangeIterator newRangeIterator(char *const string, const size_t length);

// Performs a single iteration on a `NumberRangeIterator`. If `errors` is not NULL, any parsing
// errors are sent to it.
RangeIterationResult iterateRangeString (
    NumberRangeIterator *const iter, CompoundError *const errors
);

// Generates a `NumberRangeCollection` object. NULL is returned on failure.
// This function panics the app if it there's insufficient memory.
NumberRangeCollection *makeRangeCollection (
    char *const string, size_t length, const int min, const int max, CompoundError *const errors
);

// Returns a boolean based on whether or not a number passed is within any of the specified ranges.
// If the `ranges` pointer is NULL, then `false` is returned.
_Bool queryRangeCollection(NumberRangeCollection *const ranges, const int value);

#define mRangeAt(collection, i) (((NumberRange*) ((NumberRangeCollection*) (collection) + 1)) + (i))

