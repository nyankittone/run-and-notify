#include <stddef.h>
#include <string_stuff.h>
#include <error_array.h>

#define CONVERTED_INT_SIZE (12)

typedef struct {
    enum {
        ASSEMBLE_STRING,
        ASSEMBLE_OPEN_BRACE,
        ASSEMBLE_STDOUT,
        ASSEMBLE_STDERR,
        ASSEMBLE_EXIT_CODE,
    } item_type;
    const char *string;
    size_t length;
} AssembleInstruction;

typedef struct {
    _Bool just_the_string;
    size_t length;
    void *either_string_or_instructions;
} AssembleInstructions;

// I may also need some way to return tags that are being used here.
// I *also* need a way to say "Hey, don't *actually* escape stuff here!

typedef struct {
    enum {
        CANT_PRE_CALLER_FUCKED_UP,
    } status;
    void *allocated_stuff;
} PreInterpolateResult;

// This function fills a specified array of AssembleInstructions with actual assemble instructions
// from the strings provided. It will continute to fill in the array until it runs out of strings
// specified.
// A pointer to the block of memory it allocates on the heap for storing all of this is returned.
// If the pointer is NULL, that means nothing was allocated to the heap.
void *preInterpolate (
    AssembleInstructions *dest_array, size_t dest_array_length, int argc,
    char **argv, CompoundError *errors, ...
);

// TODO: Move this out of this header file into a different one! This does not categorically fit.
typedef struct {
    CharVector *stdout, *stderr;
    int exit_code;
} CapturedOutput;

MaybeFreeable postInterpolateNoEscapes (
    AssembleInstructions *instructions, CapturedOutput *captured, char *converted_exit_code
);

MaybeFreeable postInterpolateYesEscapes (
    AssembleInstructions *instructions, CapturedOutput *captured, char *converted_exit_code
);

