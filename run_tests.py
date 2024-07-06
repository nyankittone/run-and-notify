#!/usr/bin/env python3

from sys import stderr
import subprocess as sp

# need each test to be a command, and have an expected exit code and output.
# Maybe in the future I will want to compare it against the output of another command? I'm not
# going to worry about that for nowk, though.

tests = {
    "number_range": [
        ["unit_test_bin/num 4", "from: 4, to: 4, invert: false, error: none\n", 0],
        ["unit_test_bin/num 2,6,12", "from: 2, to: 2, invert: false, error: none\n"
            "from: 6, to: 6, invert: false, error: none\n"
            "from: 12, to: 12, invert: false, error: none\n"
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
    print(f"\33[1;96mRunning test \33[97m{count}\33[96m of \33[97m{track.total}\33[96m...\33[m ({test[0]})", file=stderr)
    
    result = sp.run(test[0], stdout=sp.PIPE, shell=True).stdout.decode("utf-8", errors="replace")
    if result == test[1]:
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
          f"\33[1;93m{track.total - track.passed}\33[91m of \33[93m{track.total}\33[91m tests failed.\33[m")


if __name__ == "__main__":
    main(tests)

