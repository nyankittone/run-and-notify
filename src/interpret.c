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
    size_t length, capacity;
    AssembleInstruction *data;
} InstructionVector;

static InstructionVector *const addEntry (
    InstructionVector *const vec, const AssembleInstruction instruction, const size_t init_size
) {
    if(!vec) return NULL;
    if(!vec->data) {
        vec->data = mallocOrDie(sizeof(AssembleInstruction) * init_size);
        vec->capacity = init_size;
    }

    if(++(vec->length) > vec->capacity) {
        vec->capacity *= REALLOC_MULTIPLIER;
        vec->data = reallocOrDie(vec->data, sizeof(AssembleInstruction) * vec->capacity);
    }

    vec->data[vec->length] = instruction;
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
            heap_stuff, (AssembleInstruction) {ASSEMBLE_OPEN_BRACE},
            init_length_i_hate_this_parameter
        );
    }
}

// This whole function will need a refactor. Fucking hell.
void *preInterpolate (
    AssembleInstructions *dest_array, size_t dest_array_length, int argc,
    char **argv, CompoundError *errors, ...
) {
    if(!dest_array) return NULL;

    // create a vector of AssembleInstruction
    // this will act as a memory arena for them :3
    InstructionVector vec = {0};

    // create VLA of indexes for where each entry in the output should point to in it's pointing out
    size_t vec_offsets[dest_array_length];
    memset(vec_offsets, 0, sizeof(vec_offsets)); // Might not need this line tbh

    // set up a va_list for the variadic args
    va_list args;
    va_start(args, errors);

    for(size_t i = 0; i < dest_array_length; i++) {
        preForOne(&vec, dest_array + i, vec_offsets + i, va_arg(args, char*), argc, argv);
    }

    // Set the correct pointer values for each of the dest_array entries.
    if(vec.length) {
        // TODO: add said code here...
    }

    va_end(args);

    return vec.data;
}

