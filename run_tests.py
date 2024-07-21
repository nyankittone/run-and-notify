#!/usr/bin/env python3

from sys import stderr, argv
import subprocess as sp

# need each test to be a command, and have an expected exit code and output.
# Maybe in the future I will want to compare it against the output of another command? I'm not
# going to worry about that for nowk, though.

UNIT = True
NOT_UNIT = False

tests = {
    "number_range": [
        [UNIT, "number_range 4", "from: 4, to: 4, invert: false\n", 0],
        [UNIT, "number_range 2,6,12",
            "from: 2, to: 2, invert: false\n"
            "from: 6, to: 6, invert: false\n"
            "from: 12, to: 12, invert: false\n"
            , 0
        ],
        [UNIT, "number_range ^10", "from: 10, to: 10, invert: true\n", 0],
        [UNIT, "number_range ^12,6,^14",
            "from: 12, to: 12, invert: true\n"
            "from: 6, to: 6, invert: false\n"
            "from: 14, to: 14, invert: true\n"
            , 0
        ],
        [UNIT, "number_range 4:10", "from: 4, to: 10, invert: false\n", 0],
        [UNIT, "number_range 10:4", "from: 10, to: 4, invert: false\n", 0],
        [UNIT, "number_range 2:9,30:42",
            "from: 2, to: 9, invert: false\n"
            "from: 30, to: 42, invert: false\n"
            , 0
        ],
        [UNIT, "number_range 8:", "from: 8, to: 255, invert: false\n", 0],
        [UNIT, "number_range 100:", "from: 100, to: 255, invert: false\n", 0],
        [UNIT, "number_range :8", "from: 0, to: 8, invert: false\n", 0],
        [UNIT, "number_range :121", "from: 0, to: 121, invert: false\n", 0],
        [UNIT, "number_range ^11:89,69:60,20:29",
            "from: 11, to: 89, invert: true\n"
            "from: 69, to: 60, invert: false\n"
            "from: 20, to: 29, invert: false\n"
            , 0
        ],
        [UNIT, "number_range ^0", "from: 0, to: 0, invert: true\n", 0],
        [UNIT, "number_range '$'", "from: 255, to: 255, invert: false\n", 0],
        [UNIT, "number_range '$12'", "from: 243, to: 243, invert: false\n", 0],
        [UNIT, "number_range '$1'", "from: 254, to: 254, invert: false\n", 0],
        [UNIT, "number_range '$12:$'", "from: 243, to: 255, invert: false\n", 0],
        [UNIT, "number_range -4:20 -10 100", "from: -4, to: 20, invert: false\n", 0],
        [UNIT, "number_range 20:-4 -10 100", "from: 20, to: -4, invert: false\n", 0],
        [UNIT, "number_range ''", "", 0],
        [UNIT, "number_range '^$100:,209,$1:,239:220'",
            "from: 155, to: 255, invert: true\n"
            "from: 209, to: 209, invert: false\n"
            "from: 254, to: 255, invert: false\n"
            "from: 239, to: 220, invert: false\n"
            , 0
        ],
        [UNIT, "number_range funny_one_liner", "parsing error\n", 0],
        [UNIT, "number_range funny_one_liner,another_one_liner", 
            "parsing error\n"
            "parsing error\n"
            , 0
        ],
        [UNIT, "number_range 'aaaa!,bruh!,yAAAAAAAAH!!!!,$'", 
            "parsing error\n"
            "parsing error\n"
            "parsing error\n"
            "from: 255, to: 255, invert: false\n"
            , 0
        ],
        [UNIT, "number_range '5:12,bleh,$5'",
            "from: 5, to: 12, invert: false\n"
            "parsing error\n"
            "from: 250, to: 250, invert: false\n"
            , 0
        ],
        [UNIT, "number_range '^j'", "parsing error\n", 0],
        [UNIT, "number_range '^when_you'", "parsing error\n", 0],
        [UNIT, "number_range 'jeff:47,2'",
            "parsing error\n"
            "from: 2, to: 2, invert: false\n"
            , 0
        ],
        [UNIT, "number_range ':meow,1:3'", 
            "parsing error\n"
            "from: 1, to: 3, invert: false\n"
            , 0
        ],
        [UNIT, "number_range ':$what,6:11'", 
            "parsing error\n"
            "from: 6, to: 11, invert: false\n"
            , 0
        ],
        [UNIT, "number_range :", "parsing error\n", 0],
        [UNIT, "number_range ':12,,$6:'",
            "from: 0, to: 12, invert: false\n"
            "parsing error\n"
            "from: 249, to: 255, invert: false\n"
            , 0
        ],
        [UNIT, "number_range ':12,^,$6:'",
            "from: 0, to: 12, invert: false\n"
            "parsing error\n"
            "from: 249, to: 255, invert: false\n"
            , 0
        ],
        [UNIT, "number_range '$6:'", "from: 249, to: 255, invert: false\n", 0],
        [UNIT, "number_range 8,,,,",
         "from: 8, to: 8, invert: false\n"
         "parsing error\n"
         "parsing error\n"
         "parsing error\n"
         , 0
        ],
        [UNIT, "number_range 1,9,7,",
         "from: 1, to: 1, invert: false\n"
         "from: 9, to: 9, invert: false\n"
         "from: 7, to: 7, invert: false\n"
         , 0
        ],
    ],
    "pre_interpret": [
        [UNIT, "pre_interpret hello", "assemble string, 'hello'\n", 0],
        [UNIT, "pre_interpret ''", "assemble string, ''\n", 0],
        [UNIT, "pre_interpret 'meow{brace}'",
         "assemble string, 'meow'\n"
         "assemble open brace\n"
         , 0
        ],
        [UNIT, "pre_interpret '{brace}woof'",
         "assemble open brace\n"
         "assemble string, 'woof'\n"
         , 0
        ],
        [UNIT, "pre_interpret 'meow{brace}woof'",
         "assemble string, 'meow'\n"
         "assemble open brace\n"
         "assemble string, 'woof'\n"
         , 0
        ],
        [UNIT, "pre_interpret 'meow{brace}{brace}woof'",
         "assemble string, 'meow'\n"
         "assemble open brace\n"
         "assemble open brace\n"
         "assemble string, 'woof'\n"
         , 0
        ],
        [UNIT, "pre_interpret 'some text {code}!'",
         "assemble string, 'some text '\n"
         "assemble exit code\n"
         "assemble string, '!'\n"
         , 0
        ],
        [UNIT, "pre_interpret 'output was {out}'",
         "assemble string, 'output was '\n"
         "assemble stdout\n"
         , 0
        ],
        [UNIT, "pre_interpret 'error was {out}'",
         "assemble string, 'error was '\n"
         "assemble stderr\n"
         , 0
        ],
        [UNIT, "pre_interpret 'when{br}you{br}go{br}to{br}the{br}grocery{br}store!!!'",
         "assemble string, 'when'\n"
         "assemble newline\n"
         "assemble string, 'you'\n"
         "assemble newline\n"
         "assemble string, 'go'\n"
         "assemble newline\n"
         "assemble string, 'to'\n"
         "assemble newline\n"
         "assemble string, 'the'\n"
         "assemble newline\n"
         "assemble string, 'grocery'\n"
         "assemble newline\n"
         "assemble string, 'store!!!'\n"
         , 0
        ],
    ],
}

def get_test_count(tested):
    if type(tested) is list:
        return len(tested)

    if not type(tested) is dict:
        raise TypeError("Input needs to be only lists and dictionaries")

    returned = 0
    for value in tested.values():
        returned += get_test_count(value)

    return returned


class TestTracking:
    def __init__(self):
        self.total, self.passed = 0, 0


def run_test(test, track, count):
    print(f"\33[1;96mRunning test \33[97m{count}\33[96m of \33[97m{track.total}\33[96m...\33[m ({test[1]})", file=stderr)

    try:
        result = sp.run(test[1] if not test[0] else f"{argv[1]}/{test[1]}",
            stdout=sp.PIPE, shell=True, timeout=0.2
        ).stdout.decode("utf-8", errors="replace")
    except sp.TimeoutExpired:
        print("\33[1;91mTest failed! Took too long to run! (infinite loop?)\33[m")
        return

    if result == test[2]:
        track.passed += 1
        print("\33[1;92mTest passed.\33[m", file=stderr)
    else:
        print(f"\33[1;91mTest failed! \33[mGot:\n{result}\n", file=stderr)


def run_tests(tested, track: TestTracking):
    def recursive_run_tests(tested, track: TestTracking, prefix: str, count: int) -> int:
        if type(tested) is list:
            for test in tested:
                run_test(test, track, count)
                count += 1

            return count

        if not type(tested) is dict:
            raise TypeError("Input needs to be only lists and dictionaries")

        for key, value in tested.items():
            temp_prefix = prefix + "/" + key if len(prefix) > 0 else key
            print(f"\33[1;93mRunning section \"{temp_prefix}\"...\33[m")
            count = recursive_run_tests(value, track, temp_prefix, count)

        return count

    recursive_run_tests(tested, track, "", 1)


def main(tested):
    # get total tests needed?
    track = TestTracking()

    track.total = get_test_count(tested)
    print(f"\33[1;97m{track.total}\33[m tests in total.")

    run_tests(tested, track)

    print("\33[1;92mAll tests passed! :3\33[m" if track.total == track.passed else
          f"\33[1;97m{track.total - track.passed}\33[91m of \33[97m{track.total}\33[91m tests failed.\33[m")


if __name__ == "__main__":
    if len(argv) < 3:
        print("Path for unit test bins not passed!!!")
        exit(69)

    main(tests)

