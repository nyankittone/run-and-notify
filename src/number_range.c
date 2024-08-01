#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <number_range.h>
#include <util.h>

// These macros define the default size and growth rate of a NumberRangeCollection.
#define NUMBER_COLLECTION_SIZE (16)
#define COLLECTION_GROW_MULTIPLIER (1.5F) // idk aboutt this one tbh... might be slower right here

NumberRangeIterator newRangeIterator(char *const string, const size_t length) {
    assert(string != NULL);

    return (NumberRangeIterator) {
        .string = string,
        .og_string = string,
        .length = length,
        .og_length = length,
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

static size_t scrubThroughBaddies (
    const char *const string, const size_t length
) {
    for(size_t i = 0; i < length; i++) {
        // OPTIMIZE: See if caching string[i] will improve performance.
        if(string[i] == ',' || string[i] == ':') return i;
    }

    return length;
}

static size_t scrubThroughBaddiesNoColon (
    const char *const string, const size_t length
) {
    for(size_t i = 0; i < length; i++) {
        if(string[i] == ',') return i;
    }

    return length;
}

// Some people would say, "aaaaa, this function is too long; you should split it into smaller
// ones!!!!!". To which I would say that they're weak. >:3
// TODO: Make parser tolerant of whitespace characters.
//// This entails making it able to handle garbage input for any particular iteration.
// TODO: Consider adding "~" symbol as part of the parser, as an opposite to $
RangeIterationResult iterateRangeString (
    NumberRangeIterator *const iter, CompoundError *const errors
) {
    assert(iter != NULL);
    if(iter->length == 0) return (RangeIterationResult) {RANGE_ITER_HIT_END, {0}};

    RangeIterationResult returned = {
        .error = RANGE_ITER_SUCCESS,
    };

    // If the first character is a caret, invert the returned iteration
    if(*(iter->string) == '^') {
        returned.range.invert = true;
        iter->string++;
        iter->length--;

        if(iter->length == 0) {
            // TODO: Add line for adding to the compound error here!!!
            return (RangeIterationResult) {RANGE_ITER_FAIL, {0}};
        }
    } else {
        returned.range.invert = false;
    }
    
    // Variables for seeing if we got the "to" or "from" fields
    // Tbh, I don't think I need these two variables anymore. I should *mayyyybe* remove them?
    bool got_from = false;
    returned.range.from.based = RANGE_ABSOLUTE;
    size_t amount_read;

    // TODO: RIGHT HERE: add support for the dollar sign thingie :3333
    // Maaaan, I'm going to have a bunch of this code everywhere,,,,
    switch(*iter->string) {
        case '$':
            iter->string++;
            iter->length--;

            returned.range.from.based = RANGE_END;

            if(iter->length == 0) {
                returned.range.to.based = RANGE_END;
                returned.range.from.offset = returned.range.to.offset = 0;
                return returned;
            }

            break;
        case '~':
            iter->string++;
            iter->length--;

            returned.range.from.based = RANGE_START;

            if(iter->length == 0) {
                returned.range.to.based = RANGE_START;
                returned.range.from.offset = returned.range.to.offset = 0;
                return returned;
            }

            break;
    }

    // use special function for converting current string slice into an int
    {
        int tmp_from = shittyConvertStringToInt(iter->string, iter->length, &amount_read);

        // Were zero bytes read? If so, fill in a default of the minimum possible for min
        if(amount_read) {
            if(returned.range.from.based == RANGE_END) tmp_from *= -1;

            got_from = true;
            returned.range.from.offset = tmp_from;
            iter->string += amount_read;
            if((iter->length -= amount_read) == 0) {
                returned.range.to = returned.range.from;
                return returned;
            }
        } else {
            returned.range.from.offset = 0;

            if(returned.range.from.based != RANGE_ABSOLUTE) {
                got_from = true; // I think this is right? Fuck if I know, string parsing in C sucks
            } else {
                returned.range.from.based = RANGE_START;
            }
        }
    }

    // We now need to check to see if the character that the iterator is currently sitting on is
    // one of significance. (, :)
    // If not, then uh-oh! That's a parser error!
    // What should I do if this character is \0? Should any of this code care at all?
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
            iter->length--;

            break;
        default:
            returned.error = RANGE_ITER_FAIL;
            {
                size_t offset = scrubThroughBaddies(iter->string, iter->length);
                // TODO: Add line for adding to the compound error here!!!
                // OPTIMIZE: Is there some way we can squeeze more performance here?

                iter->string += offset;
                if((iter->length -= offset) == 0) return returned;

            }

            if(*iter->string == ',') {
                iter->string++;
                iter->length--;

                return returned;
            }
    }

    // It's now time to look at the *second* value.
    // This means doing a bunch of work again... maybe I can outsource this to a function???

    // DRY? Fuck that shit, this codebase is wet asf. This is copy-paste city, baby.
    switch(*iter->string) {
        case '$':
            iter->string++;
            iter->length--;

            returned.range.to.based = RANGE_END;

            if(iter->length == 0) {
                returned.range.to.offset = 0;
                return returned;
            }

            break;
        case '~':
            iter->string++;
            iter->length--;

            returned.range.to.based = RANGE_START;

            if(iter->length == 0) {
                returned.range.to.offset = 0;
                return returned;
            }

            break;
        default:
            returned.range.to.based = RANGE_ABSOLUTE;
    }

    {
        int tmp_to = shittyConvertStringToInt(iter->string, iter->length, &amount_read);

        if(amount_read) {
            if(returned.range.to.based == RANGE_END) tmp_to *= -1;

            returned.range.to.offset = tmp_to;
            iter->string += amount_read;
            iter->length -= amount_read;
        } else {
            if(returned.range.to.based != RANGE_ABSOLUTE) {
                returned.range.to.offset = 0;
            } else if(!got_from) {
                // TODO: Add line for adding to the compound error here!!!
                returned.error = RANGE_ITER_FAIL;
            } else {
                // I hope this line here is right lmao
                returned.range.to = (NumberRangePoint) {.based = RANGE_END, .offset = 0};
            }
        }
    }

    // Final thing to do: check that our parsed section ends in either a comma, or the length
    // counter is 0.
    if(iter->length == 0) return returned;

    if(*iter->string == ',') {
        iter->string++;
        iter->length--;

        return returned;
    }

    // At this point, we're garantueed to be in a case of a parsing failure.
    returned.error = RANGE_ITER_FAIL;
    size_t offset = scrubThroughBaddiesNoColon(iter->string, iter->length);
    // TODO: Add line for adding to the compound error here!!!

    if(offset == iter->length) {
        iter->string += offset;
        iter->length = 0;

        return returned;
    }

    offset++;
    iter->string += offset;
    iter->length -= offset;

    return returned;
}

NumberRangeCollection *makeRangeCollection (
    char *const string, size_t length, const int min, const int max, CompoundError *const errors
) {
    // I want this function to return NULL on parsing errors. Maybe I should have a different
    // function that is more fault-tolerant?

    // Creating our iterator before anything else.
    NumberRangeIterator iter = newRangeIterator(string, length);
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
        (result = iterateRangeString(&iter, errors)).error != RANGE_ITER_HIT_END;
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

SimpleRange dumbDownNumberRange (
    const NumberRange *const range, IntOption min, IntOption max, unsigned int *const error
) {
    assert(range != NULL);

    // copy over invert flag to returned
    SimpleRange returned = {.invert = range->invert};
    unsigned int error_ret = 0;

    // analyze from
    //   if based == absolute, just copy it over
    //   else, use either the min or max value and add it to offset
    //   if the needed value doesn't exist, error out
    switch(range->from.based) {
        case RANGE_ABSOLUTE:
            returned.from = range->from.offset;
            break;
        case RANGE_START:
            if(min.exists) returned.from = range->from.offset + min.data;
            else error_ret |= 1;
            break;
        case RANGE_END:
            if(max.exists) returned.from = range->from.offset + max.data;
            else error_ret |= 2;
            break;
    }

    // analyze to
    switch(range->to.based) {
        case RANGE_ABSOLUTE:
            returned.to = range->to.offset;
            break;
        case RANGE_START:
            if(min.exists) returned.to = range->to.offset + min.data;
            else error_ret |= 4;
            break;
        case RANGE_END:
            if(max.exists) returned.to = range->to.offset + max.data;
            else error_ret |= 8;
            break;
    }

    // return
    if(*error) *error = error_ret;
    return returned;
}

// Yes, I did shamelessly copy-paste from dumbDownRange. So what?
// Oh, doing that violates DRY? How about you suck my DRY dick?
SimpleRange fastDumbDownNumberRange(const NumberRange *const range, int min, int max) {
    // copy over invert flag to returned
    SimpleRange returned = {.invert = range->invert};

    // analyze from
    //   if based == absolute, just copy it over
    //   else, use either the min or max value and add it to offset
    switch(range->from.based) {
        case RANGE_ABSOLUTE:
            returned.from = range->from.offset;
            break;
        case RANGE_START:
            returned.from = range->from.offset + min;
            break;
        case RANGE_END:
            returned.from = range->from.offset + max;
            break;
    }

    // analyze to
    switch(range->to.based) {
        case RANGE_ABSOLUTE:
            returned.to = range->to.offset;
            break;
        case RANGE_START:
            returned.to = range->to.offset + min;
            break;
        case RANGE_END:
            returned.to = range->to.offset + max;
            break;
    }

    // return
    return returned;
}

SimpleRangeIterator newSimpleRangeIterator(char *const string, size_t length, int min, int max) {
    return (SimpleRangeIterator) {
        .base = newRangeIterator(string, length),
        .min = min,
        .max = max,
    };
}

SimpleRangeIterationResult iterateSimpleRangeString (
    SimpleRangeIterator *const iter, CompoundError *const errors
) {
    assert(iter != NULL);

    RangeIterationResult tmp_result = iterateRangeString(&iter->base, errors);
    if(tmp_result.error == RANGE_ITER_FAIL) return (SimpleRangeIterationResult) {
        .error = tmp_result.error,
        .range = {
            .invert = tmp_result.range.invert,
        },
    };

    SimpleRangeIterationResult returned = {
        .error = RANGE_ITER_SUCCESS,
        .range = fastDumbDownNumberRange(&tmp_result.range, iter->min, iter->max),
    };


    // Verify that things are in-bounds
    if(returned.range.from < iter->min) {
        returned.error = RANGE_ITER_FAIL;
        // TODO: Add to compound error here!
    } else if(returned.range.from > iter->max) {
        returned.error = RANGE_ITER_FAIL;
        // TODO: Add to compound error here!
    } else if(returned.range.to < iter->min) {
        returned.error = RANGE_ITER_FAIL;
        // TODO: Add to compound error here!
    } else if(returned.range.to > iter->max) {
        returned.error = RANGE_ITER_FAIL;
        // TODO: Add to compound error here!
    }

    return returned;
}

