#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <interpret.h>
#include <util.h>
#include <number_range.h>

#define BYTES_PER_INSTRUCTIONS (32)
#define REALLOC_MULTIPLIER (1.5F)

typedef struct {
    const size_t init_size;
    size_t length, capacity;
    AssembleInstruction *data;
} InstructionVector;

#define mNewInstructionVector(init_size) ((InstructionVector) {(init_size), 0, 0, NULL})

static InstructionVector *const addEntry (
    InstructionVector *const vec, const AssembleInstruction instruction
) {
    assert(vec != NULL);

    if(!vec->data) {
        vec->data = mallocOrDie(sizeof(AssembleInstruction) * vec->init_size);
        vec->capacity = vec->init_size;
    }

    if(vec->length >= vec->capacity) {
        vec->capacity *= REALLOC_MULTIPLIER;
        vec->data = reallocOrDie(vec->data, sizeof(AssembleInstruction) * vec->capacity);
    }

    vec->data[vec->length++] = instruction;
    return vec;
}

// And the record for the world's longest function name goes to...
static void addErrorRegardingNoClosingBrace(CompoundError *const errors, const char *const arg) {
    static const char error_head[] = "Forgot closing brace for parameter";

    if(!errors || !arg) return;

    size_t error_length = strlen(arg) + 42;
    char *const error = mallocOrDie(error_length);
    snprintf(error, error_length, "%s: \"%s\"", error_head, arg);
    
    addFreedError(errors, error);
}

typedef enum {
    BRACE_MAKES_SENSE,
    BRACE_BAD_CODE,
    BRACE_BAD_INDEX,
} BraceParseError;

static bool stringsEqual(const char *const str1, size_t str1_length, const char *const str2) {
    // Check if initial segment of str1 == str2
    for(size_t i = 0; i < str1_length; i++) {
        if(str1[i] != str2[i]) return false;
    }

    // check if str2 still has more bits ahead
    return !(str2[str1_length]);
}

static void addBraceThing (
    InstructionVector *const heap_stuff, int argc, char **argv, CompoundError *errors,
    const char *const ptr, size_t length, size_t init_length_i_hate_this_parameter
) {
    // do shit here I guess???
    // use stringsEqual to compare the string value :3
    if(stringsEqual(ptr, length, "brace")) {
        addEntry (
            heap_stuff, (AssembleInstruction) {ASSEMBLE_OPEN_BRACE}
        );
    }
}

typedef struct {
    size_t where_colon, where_brace;
} GetBraceEndReturn;

static AssembleInstruction parseNumberRange(char *const string, const size_t length, int argc, char **argv, CompoundError *const errors) {
    assert(string != NULL);

    NumberRangeIterator iter = newRangeIterator(string, length);

    // iterate over the iterator
    for (
        RangeIterationResult result;
        (result = iterateRangeString(&iter, errors)).error != RANGE_ITER_HIT_END;
    ) {
        // I need to somehow know the positional args by here...
        // check if range is in-range for the positional args
        // if so, include the one range.
        // Looks like I will need to re-structure this code somewhat so I can add multiple entries
        // from here. (or at least have a way to make this function be an iterator in of itself...)
    }

    return (AssembleInstruction) {ASSEMBLE_IDK};
}

// `main_length` represents how long the initial section representing the "key" is supposed to be.
// `full_length` is the entire length of the string contents contained within the braces.
static AssembleInstruction parseBraceInsides (
    char *const string, const GetBraceEndReturn spans,
    int argc, char **argv, CompoundError *const errors
) {
    assert(string != NULL);

    #define mMaybeDoError(returned) \
        return spans.where_brace > spans.where_colon ? \
        (AssembleInstruction) {ASSEMBLE_IDK} : \
        (returned)

    // We need to check the span from string to spans.where_colon, and see if it matches up with
    // a valid string for brace contents. OPTIMIZE: I might use a hash map here in the future...
    if(stringsEqual(string, spans.where_colon, "brace")) {
        mMaybeDoError((AssembleInstruction) {ASSEMBLE_OPEN_BRACE});
    } else if(stringsEqual(string, spans.where_colon, "br")) {
        mMaybeDoError((AssembleInstruction) {ASSEMBLE_NEWLINE});
    } else if(stringsEqual(string, spans.where_colon, "code")) {
        mMaybeDoError((AssembleInstruction) {ASSEMBLE_EXIT_CODE});
    } else if(stringsEqual(string, spans.where_colon, "out")) {
        return (AssembleInstruction) {ASSEMBLE_STDOUT};
    } else if(stringsEqual(string, spans.where_colon, "err")) {
        return (AssembleInstruction) {ASSEMBLE_STDERR};
    } else if(stringsEqual(string, spans.where_colon, "context")) {
        mMaybeDoError((AssembleInstruction) {ASSEMBLE_CONTEXT});
    }

    // At this point, we should try to see if we can parse this as some number ranges.
    return parseNumberRange(string, spans.where_brace, argc, argv, errors);

    #undef mMaybeDoError
}

// Subroutine for the `preForOne` function, that conditionally adds the AssembleInstruction to the
// right thing.
// the boolean that is returned is expected to be assingned to the caller's `found_something`
// variable.
static bool addInstruction (
    InstructionVector *const vec, AssembleInstructions *const dest,
    size_t *const dest_index, bool found_something, const AssembleInstruction *const the_instruction
) {
    assert(vec != NULL && dest != NULL && dest_index != NULL && the_instruction != NULL);

    if(!found_something) {
        found_something = true;
        dest->data.as_one = *the_instruction;
    } else {
        if(dest->just_one) {
            dest->just_one = false;
            *dest_index = vec->length;
            // add the existing data to the vector
            addEntry(vec, dest->data.as_one); // Seems correct (famous last words)

            dest->data.as_many.amount = 1;
        }

        dest->data.as_many.amount++;
        addEntry(vec, *the_instruction);
    }

    return found_something;
}

static GetBraceEndReturn getBraceEndAndColon(const char *string) {
    assert(string != NULL);
    GetBraceEndReturn returned = {0};

    for(;; string++) {
        if(!*string || *string == '}') return returned;

        if(*string == ':') {
            returned.where_brace++;
            string++;
            break;
        }

        returned.where_brace = ++returned.where_colon;
    }

    for(; *string && *string != '}'; string++) {
        returned.where_brace++;
    }

    return returned;
}

// Should this function *actually* return void, or no?
// Eeeeeeh, fuck it, idk, I'll just roll with the "side-effects".
// Is the amount of paramenters this function has a code smell?
static bool preForOne (
    InstructionVector *const vec, AssembleInstructions *const dest, size_t *const dest_index,
     char *const string_to_parse, int argc, char **argv,
     CompoundError *const errors, bool failed
) {
    assert(vec != NULL && dest != NULL && dest_index != NULL); // Is this a good place to use assert?

    // Yippee, time to PARSE STRINGS AGAIN! YEAAAAAAH
    bool found_something = false;
    dest->just_one = true;

    for(char *travelling = string_to_parse; *travelling;) {
        // do work for parsing string
        {
            size_t span = strcspn(travelling, "{");
            if(span) {
                found_something = addInstruction (
                    vec, dest, dest_index, found_something, 
                    &(AssembleInstruction){ASSEMBLE_STRING, {.as_string = {travelling, span}}}
                );
            }

            // incriment travelling ptr
            if(!*(travelling += span)) break;
        }

        travelling++;

        // do work for parsing brace contents
        //span = strcspn(travelling, "}");
        GetBraceEndReturn spans = getBraceEndAndColon(travelling);
        assert(spans.where_brace >= spans.where_colon); // Might remove this assert in the future...

        // check to see if an ending brace *actually* exists or not; if not, do an error
        if(!*(travelling + spans.where_brace)) {
            // TODO: do le cool error stuffs hereeee~
        }

        // parse the innards for the braces
        AssembleInstruction parsed = parseBraceInsides(travelling, spans, argc, argv, errors);
        if(parsed.item_type == ASSEMBLE_IDK) {
            failed = true;
            travelling += spans.where_brace + 1;
            continue;
        } else {
            // add the result of the parsing to the thing
            found_something = addInstruction(vec, dest, dest_index, found_something, &parsed);
        }

        travelling += spans.where_brace + 1;
    }

    if(!found_something) {
        dest->data.as_one = (AssembleInstruction) {ASSEMBLE_STRING, {.as_string = {"", 0}}};
    }

    return failed;
}

// This function will hopefully NOT need a whole refactor, because I wrote it in a way I currently
// LIKE. Let's see how bad this comment ages!!!!!!!
void *preInterpolate (
    AssembleInstructions *dest_array, size_t dest_array_length, int argc,
    char **argv, CompoundError *const errors, ...
) {
    assert(dest_array != NULL);

    // create a vector of AssembleInstruction
    // this will act as a memory arena for them :3
    InstructionVector vec = mNewInstructionVector(BYTES_PER_INSTRUCTIONS * dest_array_length);

    // create VLA of indexes for where each entry in the output should point to in it's pointing out
    size_t vec_offsets[dest_array_length];
    memset(vec_offsets, 0, sizeof(vec_offsets)); // Might not need this line tbh

    va_list args;
    va_start(args, errors);

    // TODO: Rethink about the fail case of preForOne. I want to make it free the memory on failure
    // immediately, and set vec.data to NULL to indicate such.
    bool failed = false;
    for(size_t i = 0; i < dest_array_length; i++) {
        failed = preForOne (
            &vec, dest_array + i, vec_offsets + i, va_arg(args, char*), argc, argv, errors, failed
        );

        if(!dest_array[i].just_one) {
            dest_array[i].data.as_many.ptr = vec.data + vec_offsets[i]; // what a cool line of code
        }
    }

    va_end(args);
    return vec.data;
}

