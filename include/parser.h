// Enum that contains all the return codes for the parseArgs function.
typedef enum {
    PARSER_SUCCEEDED = 0,
    PARSER_ERROR_CALLER_FUCKED_UP,
    PARSER_ERROR_INVALID_OPTION,
    PARSER_ERROR_UNBALANCED_OPTION,
} ParserError;

// Return type of the parseArgs function. Kinda like the `Result` monad from Rust, but shitter bc
// this ain't Rust.
typedef struct {
    ParserError error;
     union {
         char *as_string;
         int as_int;
     } context;
} ParserResult;

// This function is the thingie you call when you want to parse some args, baby.
// `arg_struct` is a pointer to the struct you want to store your final parsed args in.
// `argc` and `argv` are supposed to be the argc and argv parameters from the main function that you
// pass over.
// `hook` is a function that processes each command option that is encountered, and decides what to
// do for each of them.
ParserResult parseArgs (
    void *arg_struct, char **argv,
    ParserError (*hook)(const char *const arg, void *arg_struct, char ***argv_traveller)
);

// Prints a string of text to stderr representing either an error message, or some text saying
// parsing succeeded.
// `true` is returned if `result` is an error of some kind. Otherwise, `false` is returned.
// If `err_prefix` is not NULL and there's an error, it will be printed before anything else.
_Bool printParserResult(ParserResult result, const char *const err_prefix);

// Same as function `printParserResult`, except that no text is printed if there's no error.
_Bool printParserError(ParserResult result, const char *const err_prefix);

// This function tries to get the nexxt element in argv from a pointer into it. If it's already at
// the end of the array, NULL is reaturned and no incriment happens.
char *getNext(char ***argv_traveller);

// penis
#define mSetNext(to, from) {(to) = getNext(from); if(!(to)) return PARSER_ERROR_UNBALANCED_OPTION;}

// Returns how many positional arguments that are left if the parsing was successful. Otherwise,
// display an error and quit the program.
int unwrapParserResult(ParserResult the_result, int exit_code_on_fail, char *prefix_on_fail);

// Same as `unwrapParserResult`, but it takes in a pointer of a `ParserResult`, insead of the thing
// by value.
int unwrapParserResultPtr(ParserResult *the_result, int exit_code_on_fail, char *prefix_on_fail);
