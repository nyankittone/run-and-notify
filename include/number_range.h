// I hope I don't write shitty code here, bleh

#include <stddef.h>
#include <string_stuff.h>
#include <error_array.h>

// This enum is the "error" value returned alongside the result in types like `RangeIterationResult`
// and `SimpleRangeIterationResult`. It's possible enumberations are `RANGE_ITER_SUCCESS`,
// `RANGE_ITER_HIT_END`, and `RANGE_ITER_FAIL`.
enum RangeIterationError {
    RANGE_ITER_SUCCESS,
    RANGE_ITER_HIT_END,
    RANGE_ITER_FAIL,
};

// This type is used for the `from` and `to` fields in the `NumberRange` type.
typedef struct {
    enum {
        RANGE_ABSOLUTE, // Treat `offset` literally; an offset of 4 means use element 4.
        RANGE_START, // Add `offset` to the value for the start of the range to get the real element to use.
        RANGE_END, // Add `offset` to the value for the end of the range to get the real element to use.
    } based; // Enum representing what value to use when getting the real number to use.
    int offset;
} NumberRangePoint;

// Struct that contains a single range
// NOTE: Info on the minimum and maximum values for the range are *not* contained here. Those need
// to be tracked seperately.
typedef struct {
    NumberRangePoint from, to;
    _Bool invert; // Flag used to determine if this number range should be "inverted" in some way.
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
    char *string; // char* used to remember where in the string specified we are between iterations.
    const char *const og_string; // Original char* passed in, used for errors.
    size_t length; // Used as the length for the `string` field.
    const size_t og_length; // Used as the length for the `og_string` field.
} NumberRangeIterator;

// This is returned by the function that uses a `NumberRangeIterator` struct. It's composed of
// simply a `NumberRange` and an enum for tracking if an error occured.
typedef struct {
    enum RangeIterationError error; // Value representing an error that may have happened or not.
    NumberRange range; // Contains the `NumberRange` we care about, hopefully.
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

// Struct a "simplified" number range, that is unable to track ranges of indeterminite beginning
// and end, at the gain of ease of use.
typedef struct {
    int from, to;
    _Bool invert; // Flag used to determine if this number range should be "inverted" in some way.
} SimpleRange;

// Simple option type for an `int`. You know what an "option type" in programming is, yes? No?
// Well, in that case, a monad is-
typedef struct {
    _Bool exists;
    int data;
} IntOption;

#define INT_NONE ((IntOption) {.exists = 0,})
#define INT_SOME(number) ((IntOption) {.exists = 1, .data = (number),})

// Function that takes a `NumberRange*`, and given optional parameters for the minimum and maximum values
// for the range, converts it into a `SimpleRange`. If `error` is not NULL, it will be written to
// to indicate any errors with doing this operation.
SimpleRange dumbDownNumberRange (
    const NumberRange *const range, IntOption from, IntOption to, unsigned int *const error
);

// This function is like `dumbDownNumberRange`, but the parameters for `min` and `max` aren't
// optional. This garantuees no error will occur, and makes the conversion faster.
SimpleRange fastDumbDownNumberRange(const NumberRange *const range, int min, int max);

// This is a bundle of state to be used for an iterator function, much like the `NumberRangeIterator`
// object. This one, however, is intended for creating `SimpleRange`s, and adds on two `int`s for
// a minimum and maximum value so it can be used to convert `NumberRange`s to their `SimpleRange`
// equivalents.
typedef struct {
    NumberRangeIterator base;
    int min, max;
} SimpleRangeIterator;

// It's like a `RangeIterationResult`, but for `SimpleRange`s. Waow!!!!
typedef struct {
    enum RangeIterationError error; // Value representing an error that may have happened or not.
    SimpleRange range; // Contains the `SimpleRange` we care about, hopefully.
} SimpleRangeIterationResult;

// Creates a new `SimpleRangeIterator` for (checks notes) iterating over a number range string.
// This function is very similar to `newRangeIterator`, except it works with `SimpleRangeIterator`s
// instead.
SimpleRangeIterator newSimpleRangeIterator(char *const string, size_t length, int min, int max);

// Does one iteration with a `SimpleRangeIterator`. Returns a `SimpleRange` as output, alongside
// a code indicating an error or if it hit the end of the iteration.
SimpleRangeIterationResult iterateSimpleRangeString (
    SimpleRangeIterator *const iter, CompoundError *const errors
);

#define mRangeAt(collection, i) (((NumberRange*) ((NumberRangeCollection*) (collection) + 1)) + (i))

