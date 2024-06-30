#include <ctype.h>
#include <stdbool.h>

#include <error_array.h>
#include <number_range.h>

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

// Function that converts a string to an integer, because the functions for that in the standard
// library don't fit my use case.
static int shittyConvertStringToInt (
    const char *const string, const size_t length, size_t *const amount_read
) {
    size_t tmp_amount_read = 0;

    // Hey everybody, I'm using a `short` unironically! Please laugh at me!!!!!! :3
    short multiplier, digit_multiplier;
    switch(*string) {
        case '-':
            multiplier = -10;
            digit_multiplier = -1;
            tmp_amount_read++;
            break;
        case '+':
            tmp_amount_read++;
        default:
            multiplier = 10;
            digit_multiplier = 1;
            break;
    }

    int returned = 0;

    // TODO: add overflow checking!!!!!
    for(; tmp_amount_read < length && isdigit(string[tmp_amount_read]); tmp_amount_read++) {
        returned = returned * multiplier + digit_multiplier * (string[tmp_amount_read] - '0');
    }

    if(amount_read) *amount_read = tmp_amount_read;
    return returned;
}

// TO(never)DO: Re-write this so that it can hook into a CompoundError object. And have the errors
// be customizeable with a callback. Maybe. Idk.
RangeIterationResult iterateRangeString(NumberRangeIterator *const iter) {
    // this is where all the magic happens >:3
    if(!iter) return (RangeIterationResult) {RANGE_ITER_NULL_PASSED, {0}};
    if(iter->length == 0) return (RangeIterationResult) {RANGE_ITER_HIT_END, {0}};

    RangeIterationResult returned = {
        .error = RANGE_ITER_SUCCESS,
    };

    if(*(iter->string) == '^') {
        returned.range.include = false;
        iter->string++;
        iter->length--;

        if(iter->length == 0) return (RangeIterationResult) {RANGE_ITER_NO_NUMBERS, {0}};
    } else {
        returned.range.include = true;
    }

    bool got_from = false, got_to = false;
    size_t amount_read;

    // use special function for converting current string slice into an int
    {
        int tmp_from = shittyConvertStringToInt(iter->string, iter->length, &amount_read);

        // Were zero bytes read? If so, fill in a default of the minimum possible for min
        if(amount_read) {
            if(tmp_from < iter->min) {}
            if(tmp_from > iter->max) {} // PICK UP  WHERE YOU LEFT OFF HERE BRUUUUUUH

            got_from = true;
            returned.range.from = tmp_from;
        }
    }

    // Adjust iterator string and length based on bytes read
    // Look at character to see if there's a colon or comma or end???
    //   

    return returned;
}

