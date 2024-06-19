// Check this file's associate header file for documentation on the public interface.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <error_array.h>

#define ALLOC_BLOCK_SIZE (32)
#define ERROR_SUMMARY_SIZE (128)

CompoundError newCompoundError(void) {
    return (CompoundError) {0};
}

static AddErrorReturn addError(CompoundError *errors, char *message, bool should_free) {
    if(!errors) return ERROR_ADD_FAIL_BAD_CALLER;

    if(errors->length == errors->capacity) {
        size_t new_capacity = errors->capacity + 32;
        SingleError *new_ptr = realloc(errors->errors, sizeof(SingleError) * new_capacity);
        if(!new_ptr) return ERROR_ADD_FAIL_OUT_OF_MEMORY;
        
        errors->capacity = new_capacity;
        errors->errors = new_ptr;
    }

    errors->errors[errors->length++] = (SingleError) {
        .should_free = should_free,
        .error_string = message,
    };

    return ERROR_ADD_SUCCESS;
}

AddErrorReturn addStaticError(CompoundError *errors, char *message) {
    return addError(errors, message, false);
}

AddErrorReturn addFreedError(CompoundError *errors, char *message) {
    return addError(errors, message, true);
}

bool useCompoundError (
    CompoundError *const errors, const char *const main_header, const char *const sub_header,
    void (*headerMaker)(char *const error_summary, unsigned long error_count)
) {
    if(!errors || errors->length == 0) return false;

    // do displaying work here...
    if(main_header) fprintf(stderr, "%s\33[m ", main_header);
    
    char error_summary[ERROR_SUMMARY_SIZE];
    if(!headerMaker) sprintf(error_summary, "%lu errors occured!", errors->length);
    else headerMaker(error_summary, errors->length);

    // TODO: Make it so that text automatically folds according to terminal width. For now, here's
    // this much less sophisticated code.
    fprintf(stderr, "%s\n", error_summary);
    for(size_t i = 0; i < errors->length; i++) {
        if(sub_header) fprintf(stderr, "%s\33[m ", sub_header); // Fuck this line of code
        fprintf(stderr, "%s\n", errors->errors[i].error_string);
    }

    // destroying
    for(size_t i = 0; i < errors->length; i++) {
        SingleError *error = (errors->errors) + i;
        if(error->should_free) free(error->error_string);
    }

    free(errors->errors);
    *errors = (CompoundError) {0};

    return true;
}

