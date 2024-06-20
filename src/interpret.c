#include <stdarg.h>
#include <stdbool.h>
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
    InstructionVector *const vec, const AssembleInstruction *const instruction,
    const size_t init_size
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

    vec->data[vec->length] = *instruction;
    return vec;
}

void *preInterpolate(AssembleInstructions *dest_array, size_t dest_array_length, ...) {
    if(!dest_array) return NULL;

    va_list args;
    va_start(args, dest_array_length);

    // start value un-malloced
    for(size_t i = 0; i < dest_array_length; i++) {
        const char *arg = va_arg(args, char*);
        bool heaped = false;

        dest_array[i] = (AssembleInstructions) {
            .just_the_string = true,
            .length = 0,
            .capacity = 0,
            .either_string_or_instructions = (void*) arg,
        };

        // scan the string for an opening brace :3
        // idk
    }

    va_end(args);
    return NULL;
}
