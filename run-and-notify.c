// -sh: run shell command as next arg
// -st: Success title
// -sb: Success body
// -si: Success icon
// -ft: Fail title
// -fb: Fail body
// -fi: Fail icon
// -t: generic title
// -b: generic body
// -i: generic icon
// -e: set an environment variable
// -trans: use command's exit code as this program's
// --: treat any args to the right as positional args
//
// {brace}: literal opening brace
// {0}, {1}, ...: positional arguments
// {$}: last postional argument
// {$1}: second-to-last postional argument
// {$2}: third-to-last postional argument
// {1:4}: positional arguments 1-4
// {:$1}: postional arguments 0 -> second-to-last
// {1:}: postional arguments 1 -> last
// {^1}: all positional parameters that aren't 1
// {^2:4}: all positional parameters that aren't 2, 3, or 4
// {0,7,9}: positional args 0, 7, and 9
// {cmd}: full command name
// {out}: stdout
// {err}: stderr
// {code}: exit code
// {name}: program name

#define _POSIX_C_SOURCE 200112L

// Standard library
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Dependencies
#include <libnotify/notify.h>
#include <sys/types.h>

// Other files in this project
#include <bool_string.h>
#include <parser.h>
#include <rusttypes.h>
#include <error_array.h>
#include <util.h>

#include "config.h"

void printDivider(const char *const label, uint length) {
    fputs("\33[94m", stdout);

    if(!label || !*label) {
        for(uint i = 0; i < length; i++) {
            putchar('-');
        }

        goto finish;
    }

    // WARN: This code probably depends on undefined behavior regarding integer (over/under)flows.
    // Be warned!
    uint dash_length = length - (strlen(label) + 4);
    if(dash_length > length) dash_length = 0;

    for(uint i = 0; i < dash_length; i++) {
        putchar('-');
    }

    printf("[ \33[1;96m%s\33[0;94m ]", label);

    for(uint i = 0; i < dash_length; i++) {
        putchar('-');
    }

    finish:
    putchar('\n');
}

// TODO: add the string interpolation help, too.
void printHelp(void) {
    static const char *help_table[][3] = {
        {"help", "Shows the help text and exits", NULL},
        {"version", "Shows the program name, version, and author, and exits", NULL},
        {"C", "Specify the command to run, but the string for it will get interpolated", "command"},
        {"b", "Body format to use for both success and failure", "body format"},
        {"c", "Specifiy the command to run", "command"},
        {"fb", "Body format to use for failure", "body format"},
        {"fi", "Icon to use for failure", "icon string"},
        {"ft", "Title to use for failure", "title"},
        {"i", "Icon to use for both success and failure", "icon string"},
        {"sb", "Body format to use for success", "body format"},
        {"si", "Icon to use for success", "icon string"},
        {"st", "Title to use for success", "title"},
        {"t", "Title to use for both success and failure", "title"},
        {"trans", "Use the exit code of the command run instead of this program's for exiting", NULL},
    };

    static const uint indentation = 16;
    const size_t help_table_length = sizeof(help_table) / sizeof(help_table[0]);

    // print name - description
    if(PROGRAM_NAME) printf("\33[1;93m%s\33[m - ", PROGRAM_NAME);
    puts("run a shell command and report how it ran with a notification");

    // print usage
    printf (
        "Usage: \33[1;97m%s \33[32m[options...] \33[35m[positional args...]\33[m\n",
        PROGRAM_NAME ? PROGRAM_NAME : "<program-name>"
    );

    printDivider("Options", 30);

    // print options
    // TODO: Make the the description text aligned properly.
    for(size_t i = 0; i < help_table_length; i++) {
        printf("\33[1;93m-%s", help_table[i][0]);
        if(help_table[i][2]) printf(" \33[96m<%s>", help_table[i][2]);
        printf("\33[m: %s\n", help_table[i][1]);
    }

    // print string interpolation things
}

void printVersion(void) {
    if(PROGRAM_NAME) printf("\33[1;97m%s\33[m", PROGRAM_NAME);
    else fputs("\33[1;91mno name\33[m", stdout);

    fputs(", ", stdout);

    if(PROGRAM_VERSION) printf("version \33[1;95m%s\33[m", PROGRAM_VERSION);
    else fputs("\33[1;91mno version number\33[m", stdout);

    putchar('\n');

    if(PROGRAM_AUTHOR) printf("made by \33[1;95m%s\33[m", PROGRAM_AUTHOR);
    else fputs("\33[1;91mno version number\33[m", stdout);

    if(AUTHOR_GITHUB) printf(" (\33[4m%s\33[m)", AUTHOR_GITHUB);
    putchar('\n');
}

// Btw, positionals is just a pointer to argv from main. Be warned.
typedef struct {
    char *command, *exit_code_spec,
        *title_success, *body_success, *icon_success,
        *title_failure, *body_failure, *icon_failure;
    bool transfer_exit_code, show_help, show_version;
} Args;

void printArgs(const Args *const args) {
    fprintf (
        stderr,
        "Command: \"%s\"\n"
        "Exit Code Spec: \"%s\"\n"
        "Success Title: \"%s\"\n"
        "Success Body: \"%s\"\n"
        "Success Icon: \"%s\"\n"
        "Failed Title: \"%s\"\n"
        "Failed Body: \"%s\"\n"
        "Failed Icon: \"%s\"\n"
        "Tansfer exit code: %s\n"
        "Show help: %s\n"
        "Show version: %s\n",

        args->command,
        args->exit_code_spec,
        args->title_success,
        args->body_success,
        args->icon_success,
        args->title_failure,
        args->body_failure,
        args->icon_failure,
        mBoolStr(args->transfer_exit_code),
        mBoolStr(args->show_help),
        mBoolStr(args->show_version)
    );
}

typedef enum {
    ENV_ALL_IS_GOOD,
    ENV_NULL_KEY,
    ENV_BAD_KEY,
    ENV_NO_MEMORY,
} EnvironmentVariableFuncReturn;

// This function will either set or unset an environment variable.
// WARNING: This function tries to mutate the string passed in to work! With how this function's
// being used, it *shouldn't* be a problem. But still.
static EnvironmentVariableFuncReturn handleEnvironmentVariable(char *pair) {
    if(!pair) return ENV_NULL_KEY;

    // find equals sign
    char *split_point = strchr(pair, '=');
    if(!split_point) {
        unsetenv(pair);
        return ENV_ALL_IS_GOOD;
    }

    *split_point++ = '\0';
    errno = 0;

    if(setenv(pair, split_point, true)) {
        switch(errno) {
            case EINVAL:
                return ENV_BAD_KEY;
            case ENOMEM:
                return ENV_NO_MEMORY;
        }
    }

    return ENV_ALL_IS_GOOD;
}

// hahahahaa louis remember the time I was writing an argument parser in C
// FIXME: Change the interface of this thing and the argument parsing code to allow for this to
// emit custom error messages!!!!! (not doing it now bc lazy :3)
static ParserError parseArg(const char *const arg, void *object, char ***argv_traveller) {
    #define args ((Args *const) object)

    // TODO: Experiment with making this a hash map instead of wall if else-ifs. See if it improves
    // performance.
    // NOTE: the semicolons missinbg here are not only possible here, but in-fact required, due to
    // mSetNext being a stupid-ass macro. I hate this.
    if(!strcmp(arg, "sh")) mSetNext(args->command, argv_traveller)
    else if(!strcmp(arg, "c")) mSetNext(args->exit_code_spec, argv_traveller)
    else if(!strcmp(arg, "st")) mSetNext(args->title_success, argv_traveller)
    else if(!strcmp(arg, "sb")) mSetNext(args->body_success, argv_traveller)
    else if(!strcmp(arg, "si")) mSetNext(args->icon_success, argv_traveller)
    else if(!strcmp(arg, "ft")) mSetNext(args->title_failure, argv_traveller)
    else if(!strcmp(arg, "fb")) mSetNext(args->body_failure, argv_traveller)
    else if(!strcmp(arg, "fi")) mSetNext(args->icon_failure, argv_traveller)

    // TODO: Change functionality here so that they're only overridden when they're NULL
    else if(!strcmp(arg, "t")) {
        mSetNext(args->title_success, argv_traveller)
        args->title_failure = **argv_traveller;
    } else if(!strcmp(arg, "b")) {
        mSetNext(args->body_success, argv_traveller)
        args->body_failure = **argv_traveller;
    } else if(!strcmp(arg, "i")) {
        mSetNext(args->icon_success, argv_traveller)
        args->icon_failure = **argv_traveller;
    } else if(!strcmp(arg, "trans")) args->transfer_exit_code = true;
    else if(!strcmp(arg, "help")) args->show_help = true;
    else if(!strcmp(arg, "version")) args->show_version = true;
    else if(!strcmp(arg, "e")) {
        char *arg = getNext(argv_traveller);
        if(!arg) return PARSER_ERROR_UNBALANCED_OPTION;
        handleEnvironmentVariable(arg);
    } else return PARSER_ERROR_INVALID_OPTION;

    return PARSER_SUCCEEDED;
    #undef args
}

int main(int argc, char *argv[]) {
    Args args = {
        .command = NULL,
        .exit_code_spec = NULL,
        .title_success = "Command ran successfully!",
        .title_failure = "Command failed!",
        .body_success = "{cmd}",
        .body_failure = "\"{cmd}\" exited with {code}",
        .icon_success = "dialog-information",
        .icon_failure = "dialog-information",
        .transfer_exit_code = false,
        .show_help = false,
        .show_version = false,
    };

    argc = unwrapParserResult(parseArgs(&args, ++argv, &parseArg), EXIT_PARSING_FAILURE, FATAL_ERROR_TEXT);

    // Now what?
    // Well, we need to add support for the help and version flags.
    if(args.show_help) {
        printHelp();

        if(args.show_version) {
            putchar('\n');
            printVersion();
        }

        return EXIT_SUCCESS;
    }

    if(args.show_version) {
        printVersion();
        return EXIT_SUCCESS;
    }

    printArgs(&args);
    fprintf(stderr, "\nargc: %d\n", argc);

    // Then, we need to verify that the format we passed to the body is correct.
    // Here's an idea of mine: we can allow the program to accept an entire shell-command as-is
    // (-sh flag), *or* we can have it by default take the positional arguments and run that as a
    // command. Let's do both!!!!!!!
    // This means nothing extra will be needed for the actual command, period. We still need to
    // look at the title and body formats, to verify that they are syntatctically valid, though.
    CompoundError errors = newCompoundError();

    // error out if no shell command or positional args passed
    // TODO: Make these errors sorted??? Maybe???
    if(!args.command && argc == 0) addStaticError(&errors, "No command to be run was specified!");

    // interpret the exit code spec

    // pre-interpret the title and body texts passed in

    if(useCompoundError(&errors, FATAL_ERROR_TEXT, "\33[1;97m>\33[m", NULL)) {
        return EXIT_PRE_LAUNCH_ERROR;
    }

    notify_init("run-and-notify");
    NotifyNotification *hi = notify_notification_new (
        "Hello, world!~ :3",
        "&lt;i&gt;This is my first time using libnotify from C!!! :333&lt;/i&gt;",
        "dialog-information"
    );

    // notify_notification_set_urgency(hi, NOTIFY_URGENCY_CRITICAL);
    notify_notification_show(hi, NULL);
    g_object_unref(hi);
    notify_uninit();

    return EXIT_SUCCESS;
}

