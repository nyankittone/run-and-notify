#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <number_range.h>
#include <util.h>

#define NUMBER_COLLECTION_SIZE (16)
#define COLLECTION_GROW_MULTIPLIER (1.5F) // idk aboutt this one tbh... might be slower right here

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

// Some people would say, "aaaaa, this function is too long; you should split it into smaller
// ones!!!!!". To which I would say that they're weak. >:3
// TODO: Make parser tolerant of whitespace characters.
// TODO: Consider adding "~" symbol as part of the parser, as an opposite to $
RangeIterationResult iterateRangeString (
    NumberRangeIterator *const iter, CompoundError *const errors
) {
    // this is where all the magic happens >:3
    if(!iter) return (RangeIterationResult) {RANGE_ITER_NULL_PASSED, {0}};
    if(iter->length == 0) return (RangeIterationResult) {RANGE_ITER_HIT_END, {0}};

    RangeIterationResult returned = {
        .error = RANGE_ITER_SUCCESS,
    };

    // If the first character is a caret, invert the returned iteration
    if(*(iter->string) == '^') {
        returned.range.include = false;
        iter->string++;
        iter->length--;

        if(iter->length == 0) {
            // TODO: Add line for adding to the compound error here!!!
            return (RangeIterationResult) {RANGE_ITER_FAIL, {0}};
        }
    } else {
        returned.range.include = true;
    }

    // Variables for seeing if we got the "to" or "from" fields
    // Tbh, I don't think I need these two variables anymore. I should *mayyyybe* remove them?
    bool got_from = false, encountered_dollar = false;
    size_t amount_read;

    // TODO: RIGHT HERE: add support for the dollar sign thingie :3333
    // Maaaan, I'm going to have a bunch of this code everywhere,,,,
    if(*(iter->string) == '$') {
        iter->string++;
        if((iter->length--) == 0) {
            returned.range.from = returned.range.to = iter->max;
            return returned;
        }

        encountered_dollar = true;
    }

    // use special function for converting current string slice into an int
    {
        int tmp_from = shittyConvertStringToInt(iter->string, iter->length, &amount_read);

        // Were zero bytes read? If so, fill in a default of the minimum possible for min
        if(amount_read) {
            if(encountered_dollar) tmp_from = iter->max - tmp_from;

            if(tmp_from < iter->min) {
                // TODO: Add line for adding to the compound error here!!!
            }

            if(tmp_from > iter->max) {
                // TODO: Add line for adding to the compound error here!!!
            }

            got_from = true;
            returned.range.from = tmp_from;
            iter->string += amount_read;
            if((iter->length -= amount_read) == 0) {
                returned.range.to = tmp_from;
                return returned;
            }
        } else if(encountered_dollar) {
            got_from = true; // I think this is right? FUck if I know, string parsing in C sucks
            returned.range.from = iter->max;
        }
    }

    // We now need to check to see if the character that the iterator is currently sitting on is
    // one of significance. (, :)
    // If not, then uh-oh! That's a parser error!
    // What should I do if this character is \0? Should any of this code care at all?
    // NOTE: I *miiiiight* need to include some code regarding handling the dollar sign character.
    // However, I see this as unlikely.
    switch(*iter->string) {
        case ',': // incriment iter->string by one and return returned?
            iter->string++;
            iter->length--;

            // If on this iteration, we encounter only a single ",", that's a parsing error. Do the
            // appropriate things if that is the case.
            if(got_from) {
                returned.range.to = returned.range.from;
            } else {
                returned.error = RANGE_ITER_FAIL;
                // TODO: Add line for adding to the compound error here!!!
            }

            return returned;
        case ':':
            iter->string++;
            if((iter->length--) == 0) {
                // Did we get from? If yes, then return immeditately. If no, then emit an error,
                // then return.
                if(!got_from) {
                    // TODO: Add line for adding to the compound error here!!!
                    returned.error = RANGE_ITER_FAIL;
                }

                return returned;
            }

            break;
        default:
            // TODO: Add line for adding to the compound error here!!! (And maybe more...)
            ;
    }

    // It's now time to look at the *second* value.
    // This means resetting the dollar sign boolean, and doing a bunch of work again...
    // ...maybe I can outsource this to a function??? mayyybe

    // DRY? Fuck that shit, this codebase is wet asf
    if(*(iter->string) == '$') {
        iter->string++;
        if((iter->length--) == 0) {
            returned.range.to = iter->max;
            return returned;
        }

        encountered_dollar = true;
    } else {
        encountered_dollar = false;
    }

    {
        int tmp_to = shittyConvertStringToInt(iter->string, iter->length, &amount_read);

        // Were zero bytes read? If so, fill in a default of the minimum possible for min
        if(amount_read) {
            if(encountered_dollar) tmp_to = iter->max - tmp_to;

            if(tmp_to < iter->min) {
                // TODO: Add line for adding to the compound error here!!!
            }

            if(tmp_to > iter->max) {
                // TODO: Add line for adding to the compound error here!!!
            }

            returned.range.to = tmp_to;
            iter->string += amount_read;
            iter->length -= amount_read;
        } else {
            if(encountered_dollar) {
                returned.range.to = iter->max;
            } else if(!got_from) {
                // TODO: Add line for adding to the compound error here!!!
                returned.error = RANGE_ITER_FAIL;
            } else {
                returned.range.to = iter->max;
            }
        }
    }

    // Final thing to do: check that our parsed section ends in either a comma, or the length
    // counter is 0.
    if(iter->length == 0) return returned;

    if(*(iter->string) != ',') {
        // TODO: Add line for adding to the compound error here!!!
        // TODO: add logic here for "fast-forewarding through the rest of the string until I
        // I encounter another comma...
        // Me from a week later looking at the above comment: wtf does that shit mean :skull:
    } else {
        iter->string++;
        iter->length--;
    }

    return returned;
}

NumberRangeCollection *makeRangeCollection (
    char *const string, size_t length, const int min, const int max
) {
    // I want this function to return NULL on parsing errors. Maybe I should have a different
    // function that is more fault-tolerant?

    // Creating our iterator before anything else.
    NumberRangeIterator iter;
    {
        NewRangeIteratorError error;
        iter = newRangeIterator(string, length, min, max, &error);
        if(error) {
            // I miiiiiight want to dump errors thatg happen here into a CompoundError. For now, I
            // don't see the nned to.
            return NULL;
        }
    }

    size_t capacity = NUMBER_COLLECTION_SIZE;

    // Allocating initial space for the returned array.
    NumberRangeCollection *returned = mallocOrDie (
        sizeof(NumberRangeCollection) + sizeof(NumberRange) * capacity
    );

    *returned = (NumberRangeCollection) {
        .min = min,
        .max = max,
        .length = 0,
        .capacity = capacity,
    };

    bool had_error = false;
    NumberRange *ranges = mRangeAt(returned, 0);

    // Time to iterate over the string...
    for (
        RangeIterationResult result;
        (result = iterateRangeString(&iter, NULL)).error != RANGE_ITER_HIT_END;
    ) {
        if(had_error) continue;

        if(result.error == RANGE_ITER_FAIL) {
            had_error = true;
            free(returned);
            returned = NULL;

            continue;
        }

        // re-allocating if needed
        if(returned->length >= capacity) {
            capacity *= COLLECTION_GROW_MULTIPLIER;
            returned = reallocOrDie (
                returned, sizeof(NumberRangeCollection) + sizeof(NumberRange) * capacity
            );

            returned->capacity = capacity;
            ranges = mRangeAt(returned, 0);
        }

        ranges[length++] = result.range;
    }

    return returned;
}

