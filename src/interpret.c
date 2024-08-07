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

// This struct is used for building out the array of AssembleInstruction for preInterpolate.
// When preInterpolate returns, it only returns this struct's pointer, so the data can be free'd
// leter. Everything else is lost.
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
// NOTE: This function probably isn't very useful to keep in the code tbh. I should remove it if
// it coninues to be useless.
static void addErrorRegardingNoClosingBrace(CompoundError *const errors, const char *const arg) {
    static const char error_head[] = "Forgot closing brace for parameter";

    if(!errors || !arg) return;

    size_t error_length = strlen(arg) + 42;
    char *const error = mallocOrDie(error_length);
    snprintf(error, error_length, "%s: \"%s\"", error_head, arg);
    
    addFreedError(errors, error);
}

// Function that checks if the initial length of `str1` specified by `str-length` matches `str2`.
static bool stringsEqual(const char *const str1, size_t str1_length, const char *const str2) {
    // Check if initial segment of str1 == str2
    for(size_t i = 0; i < str1_length; i++) {
        if(str1[i] != str2[i]) return false;
    }

    // check if str2 still has more bits ahead
    return !(str2[str1_length]);
}

// Return type for the getBraceEndAndColon function.
typedef struct {
    size_t where_colon, // Index of where the first colon was encountered.
    where_brace; // Index of where the closing brace was encountered.
} GetBraceEndReturn;

// Container for some parameters passed around the are needed to add an instruction to our data
// structure.
typedef struct {
    InstructionVector *const vec; // The vector for the allocated block.
    AssembleInstructions *dest; // the location of the AssembleInstructions being used at any time.
    size_t *dest_index; // location to write the memory address for writing the index into the vec.
    bool found_something; // flag used for remembering if we previously found a thing or not
} InstructionAdder;

// Subroutine for the `preForOne` function, that conditionally adds the AssembleInstruction to the
// right thing.
// the boolean that is returned is expected to be assingned to the caller's `found_something`
// variable.
static void addInstruction (
    InstructionAdder *const adder, const AssembleInstruction *const the_instruction
) {
    assert(adder != NULL);

    if(!adder->found_something) {
        if(the_instruction->item_type == ASSEMBLE_STRING) {
        }

        adder->found_something = true;
        adder->dest->data.as_one = *the_instruction;
    } else {
        if(adder->dest->just_one) {
            adder->dest->just_one = false;
            *adder->dest_index = adder->vec->length;
            // add the existing data to the vector
            addEntry(adder->vec, adder->dest->data.as_one); // Seems correct (famous last words)

            adder->dest->data.as_many.amount = 1;
        }

        adder->dest->data.as_many.amount++;
        addEntry(adder->vec, *the_instruction);
    }
}

// Function that uses an `InstructionAdder` to add all of the args from argv in a specified range.
// Note that any value of `step` that isn't 1 or -1 will cause things to break. :3
static void addArgRange (
    InstructionAdder *const adder, const int from , const int to, const int step, char **argv
) {
    for(int i = from; i != to; i += step) {
        addInstruction (
            adder,
            &(AssembleInstruction) {ASSEMBLE_STRING, {.as_string = {argv[i], strlen(argv[i])}}}
        );
        addInstruction (
            adder,
            &(AssembleInstruction) {ASSEMBLE_STRING, {.as_string = {" ", 1}}}
        );
    }

    addInstruction (
        adder,
        &(AssembleInstruction) {ASSEMBLE_STRING, {.as_string = {argv[to], strlen(argv[to])}}}
    );

}

// This function adds args from argv using the provided `InstructionAdder` and `SimpleRange`.
static InstructionAdder *addArgs (
    InstructionAdder *const adder, SimpleRange *const range, int argc, char **argv
) {
    assert(adder != NULL && argv != NULL && argc >= 0);

    int step = range->from <= range->to ? 1 : -1;
    if(!range->invert) {
        addArgRange(adder, range->from, range->to, step, argv);
        return adder;
    }

    // what should be done on an invert flag?
    // if from and to are swapped, iterate backwards, with the right side, and then the left side.
    // else, iterate forwards, startiong with the first side, then the right.

    // This if-else statement here could work as a metaphor for depression or some shit
    bool ran_thing = false;
    if(step < 0) {
        if(range->from < argc - 1) {
            addArgRange(adder, argc - 1, range->from + 1, step, argv);
            ran_thing = true;
        }

        if(range->to > 0) {
            if(ran_thing) addInstruction (
                adder,
                &(AssembleInstruction) {ASSEMBLE_STRING, {.as_string = {" ", 1}}}
            );

            addArgRange(adder, range->to - 1, 0, step, argv);
        }
    } else {
        if(range->from > 0) {
            addArgRange(adder, 0, range->from - 1, step, argv);
            ran_thing = true;
        }

        if(range->to < argc - 1) {
            if(ran_thing) addInstruction (
                adder,
                &(AssembleInstruction) {ASSEMBLE_STRING, {.as_string = {" ", 1}}}
            );
            addArgRange(adder, range->to + 1, argc - 1, step, argv);
        }
    }

    return adder;
}

// This function iterates over the number range string passed to it, while adding args from argv
// based on the ranges fetched from each iteration.
static bool parseNumberRange (
    InstructionAdder *const adder, bool failed, char *const string, const size_t length,
    int argc, char **argv, CompoundError *const errors
) {
    assert(string != NULL);
    if(!length) {
        // TODO: Add compounderror BS here!!!!
        return false;
    }

    SimpleRangeIterator iter = newSimpleRangeIterator(string, length, 0, argc - 1);
    SimpleRangeIterationResult result = iterateSimpleRangeString(&iter, errors);
    if(result.error == RANGE_ITER_HIT_END) return failed;
    if(result.error == RANGE_ITER_FAIL) {
        // TODO: Add compounderror BS here!!!!
        failed = true;
    } else {
        // if so, include the one range.
        //   This means adding them in the correct order.
        addArgs(adder, &result.range, argc, argv);
    }

    // iterate over the iterator
    for (
        SimpleRangeIterationResult result;
        (result = iterateSimpleRangeString(&iter, errors)).error != RANGE_ITER_HIT_END;
    ) {
        if(result.error == RANGE_ITER_FAIL) {
            failed = true;
        } else {
            addArgs(adder, &result.range, argc, argv);
        }
    }

    return failed;
}

// `main_length` represents how long the initial section representing the "key" is supposed to be.
// `full_length` is the entire length of the string contents contained within the braces.
static bool parseBraceInsides (
    InstructionAdder *const adder, bool failed, char *const string,
    const GetBraceEndReturn spans, int argc, char **argv, CompoundError *const errors
) {
    assert(string != NULL);

    #define mErrOnParameter(returned) \
        if(spans.where_brace > spans.where_colon) { \
             failed = true; /* TODO: add compounderror shit here */ \
        } else if(!failed) { \
            addInstruction(adder, &(returned)); \
        }

    // We need to check the span from string to spans.where_colon, and see if it matches up with
    // a valid string for brace contents. OPTIMIZE: I might use a hash map here in the future...
    if(stringsEqual(string, spans.where_colon, "brace")) {
        mErrOnParameter((AssembleInstruction) {ASSEMBLE_OPEN_BRACE});
    } else if(stringsEqual(string, spans.where_colon, "br")) {
        mErrOnParameter((AssembleInstruction) {ASSEMBLE_NEWLINE});
    } else if(stringsEqual(string, spans.where_colon, "code")) {
        mErrOnParameter((AssembleInstruction) {ASSEMBLE_EXIT_CODE});
    } else if(stringsEqual(string, spans.where_colon, "out")) {
        addInstruction(adder, &(AssembleInstruction) {ASSEMBLE_STDOUT});
    } else if(stringsEqual(string, spans.where_colon, "err")) {
        addInstruction(adder, &(AssembleInstruction) {ASSEMBLE_STDERR});
    } else if(stringsEqual(string, spans.where_colon, "context")) {
        mErrOnParameter((AssembleInstruction) {ASSEMBLE_CONTEXT});
    } else if(stringsEqual(string, spans.where_colon, "arg")) {
        failed = parseNumberRange (
            adder, failed, string + spans.where_colon + 1,
            spans.where_brace - spans.where_colon - 1, argc, argv, errors
        );
    }

    // At this point, we should try to see if we can parse this as some number ranges.
    return failed;

    #undef mMaybeDoError
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
// FIXME: It looks like this function will add stuff to the returned vec regardless of if a failure
// occured or not. If so, that's a major problem.
static bool preForOne (
    InstructionAdder *const adder,
    char *const string_to_parse, int argc, char **argv,
    CompoundError *const errors, bool failed
) {
    assert(adder != NULL);

    // Yippee, time to PARSE STRINGS AGAIN! YEAAAAAAH
    adder->dest->just_one = true;

    for(char *travelling = string_to_parse; *travelling;) {
        // do work for parsing string
        {
            size_t span = strcspn(travelling, "{");
            if(span) {
                addInstruction (
                    adder, &(AssembleInstruction){ASSEMBLE_STRING, {.as_string = {travelling, span}}}
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
        failed = parseBraceInsides(adder, failed, travelling, spans, argc, argv, errors);
        /*if(failed) {*/
        /*    travelling += spans.where_brace + 1;*/
        /*    continue;*/
        /*} else {*/
        /*    // add the result of the parsing to the thing*/
        /*    addInstruction(adder, &parsed);*/
        /*}*/
        /**/
        travelling += spans.where_brace + 1;
    }

    if(!adder->found_something) {
        adder->dest->data.as_one = (AssembleInstruction) {ASSEMBLE_STRING, {.as_string = {"", 0}}};
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
    InstructionAdder adder = {&vec, dest_array, vec_offsets, false};
    bool failed = false;
    for(size_t i = 0; i < dest_array_length; i++) {
        failed = preForOne(&adder, va_arg(args, char*), argc, argv, errors, failed);

        // BUG: This check needs to run in a seperate loop after this current one. If not, I think
        // it's possible for a realloc of our InstructionVector to cause the pointer value to
        // become wrong, and point to uninitialized memory. For some reason, my tests are not
        // catching this bug... peculiar.
        if(!dest_array[i].just_one) {
            dest_array[i].data.as_many.ptr = vec.data + vec_offsets[i]; // what a cool line of code
        }

        adder.dest++;
        adder.dest_index++;
        adder.found_something = false;
    }

    va_end(args);
    return vec.data;
}

