// Implementation of include/parser.h
// Please reference that file for documentation of this thing's public API.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parser.h>

// Internal function that handles specifically printing the error message for a parser result.
static void printJustTheError(ParserResult *result) {
    switch(result->error) {
        case PARSER_ERROR_INVALID_OPTION:
            fprintf(stderr, "invalid option passed: \"%s\"\n", result->context.as_string);
            break;
        case PARSER_ERROR_CALLER_FUCKED_UP:
            fputs("caller of parser sent invalid parameters to it\n", stderr);
            break;
        case PARSER_ERROR_UNBALANCED_OPTION:
            fprintf(stderr, "No parameter provided for option \"%s\"\n", result->context.as_string);
            break;
    }
}

bool printParserResult(ParserResult result, const char *const err_prefix) {
    if(!result.error) {
        fputs("Parsing was successful!\n", stderr);
        return false;
    }

    if(err_prefix) fputs(err_prefix, stderr);
    printJustTheError(&result);
    return true;
}

bool printParserError(ParserResult result, const char *const err_prefix) {
    if(!result.error) return false;
    
    if(err_prefix) fputs(err_prefix, stderr);
    printJustTheError(&result);
    return true;
}

ParserResult parseArgs (
    void *arg_struct, char **argv,
    ParserError (*hook)(const char *const, void *, char***)
) {
    if(!arg_struct || !argv || !hook) {
        return (ParserResult) {PARSER_ERROR_CALLER_FUCKED_UP, NULL};
    }

    bool force_positional = false;
    char **argv_writer = argv;
    int new_argc = 0;

    // Loop over argv until a NULL is hit
    // Man this code will be SOOOOO fun to debug
    for(char **argv_reader = argv, **persistent_argv_reader = argv; *argv_reader; argv_reader++) {
        if(force_positional) {
            *(argv_writer++) = *argv_reader;
            new_argc++;
            continue;
        }

        if(**argv_reader != '-' || (*argv_reader)[1] == '\0') {
            force_positional = true;
            *(argv_writer++) = *argv_reader;
            new_argc++;
            continue;
        }

        if((*argv_reader)[1] == '-' && (*argv_reader)[2] == '\0') {
            force_positional = true;
            continue;
        }

        // this is an option by now.
        //persistent_argv_reader = argv_reader;
        char *option = *argv_reader + strspn(*argv_reader, "-");
        ParserError error = hook(option, arg_struct, &argv_reader);
        if(error) return (ParserResult) {error, option};
    }

    *argv_writer = NULL;
    return (ParserResult) {PARSER_SUCCEEDED, {.as_int = new_argc}};
}

char *getNext(char ***argv_traveller) {
    if(*(++(*argv_traveller))) {
        return **argv_traveller;
    }

    (*argv_traveller)--;
    return NULL;
}

int unwrapParserResultPtr(ParserResult *the_result, int exit_code_on_fail, char *prefix_on_fail) {
    if(the_result->error) {
        if(prefix_on_fail) fputs(prefix_on_fail, stderr);
        printJustTheError(the_result);
        exit(exit_code_on_fail);
    }

    return the_result->context.as_int;
}

int unwrapParserResult(ParserResult the_result, int exit_code_on_fail, char *prefix_on_fail) {
    if(the_result.error) {
        if(prefix_on_fail) fputs(prefix_on_fail, stderr);
        printJustTheError(&the_result);
        exit(exit_code_on_fail);
    }

    return the_result.context.as_int;
}

