#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <interpret.h>
#include <util.h>

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
    enum {
        BRACE_PARSE_OK,
        BRACE_PARSE_FAIL,
    } error;
    AssembleInstruction result;
} ParseBracesReturn;

static ParseBracesReturn parseBraceInsides (
    const char *const string, const size_t length, CompoundError *const errors
) {
    assert(string != NULL);

    // NOTE: temporary mock implementation; replace with real brace parsing later...
    return (ParseBracesReturn) {BRACE_PARSE_OK, {ASSEMBLE_OPEN_BRACE, {0}}};
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

            dest->data.as_many.amount = 0;
        }

        // incriment data.as_many.amount by 1
        dest->data.as_many.amount++;

        // add new entry
        addEntry(vec, *the_instruction);
    }

    return found_something;
}

// Should this function *actually* return void, or no?
// Eeeeeeh, fuck it, idk, I'll just roll with the "side-effects".
// Is the amount of paramenters this function has a code smell?
static bool preForOne (
    InstructionVector *const vec, AssembleInstructions *const dest, size_t *const dest_index,
     const char *const string_to_parse, int argc, char **argv,
     CompoundError *const errors, bool failed
) {
    assert(vec != NULL && dest != NULL && dest_index != NULL); // Is this a good place to use assert?

    // Yippee, time to PARSE STRINGS AGAIN! YEAAAAAAH
    bool found_something = false;
    dest->just_one = true;

    for(const char *travelling = string_to_parse; *travelling;) {
        // do work for parsing string
        size_t span = strcspn(travelling, "{");
        if(span) {
            found_something = addInstruction (
                vec, dest, dest_index, found_something, 
                &(AssembleInstruction){ASSEMBLE_STRING, {.as_string = {travelling, span}}}
            );
        }

        // incriment travelling ptr
        if(!*(travelling += span)) break;
        travelling++;

        // do work for parsing brace contents
        span = strcspn(travelling, "}");

        // check to see if an ending brace *actually* exists or not; if not, do an error
        if(!*(travelling + span)) {
            // TODO: do le cool error stuffs hereeee~
        }

        // parse the innards for the braces
        ParseBracesReturn parsed = parseBraceInsides(travelling, span, errors);
        if(parsed.error) {
            failed = true;
            travelling += span + 1;
            continue;
        } else {
            // add the result of the parsing to the thing
            found_something = addInstruction(vec, dest, dest_index, found_something, &parsed.result);
        }

        travelling += span + 1;
    }

    if(!found_something) {
        dest->data.as_one = (AssembleInstruction) {ASSEMBLE_STRING, {.as_string = {"", 0}}};
    }

    return failed;
}

// This whole function will need a refactor. Fucking hell.
void *preInterpolate (
    AssembleInstructions *dest_array, size_t dest_array_length, int argc,
    char **argv, CompoundError *errors, ...
) {
    assert(dest_array != NULL);

    // create a vector of AssembleInstruction
    // this will act as a memory arena for them :3
    InstructionVector vec = mNewInstructionVector(BYTES_PER_INSTRUCTIONS * dest_array_length);

    // create VLA of indexes for where each entry in the output should point to in it's pointing out
    size_t vec_offsets[dest_array_length];
    memset(vec_offsets, 0, sizeof(vec_offsets)); // Might not need this line tbh

    // set up a va_list for the variadic args
    va_list args;
    va_start(args, errors);

    for(size_t i = 0; i < dest_array_length; i++) {
        preForOne(&vec, dest_array + i, vec_offsets + i, va_arg(args, char*), argc, argv, errors, false);
    }

    // Set the correct pointer values for each of the dest_array entries.
    if(vec.length) {
        // TODO: add said code here...
    }

    va_end(args);

    return vec.data;
}

