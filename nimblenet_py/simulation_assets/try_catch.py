# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import ne_re as re

def check_exception(func, expectedErrorRegex):
    try:
        func()
    except Exception as err:
        print("REGEX:", expectedErrorRegex)
        print("ERROR:", err)
        assert re.search(expectedErrorRegex, str(err))
        return

    # func() should always throw an Exception
    assert False


def check_bare_except():
    try:
        raise Exception("Bare Error")
    except:
        print("Bare Except is working")
        return
    # control shouldn't reach here
    assert False


def try_catch_test(input):
    assert True
    assert True, "Assertion Passed"

    def assert_false():
        assert False

    def assert_false_with_message():
        assert False, "Sample Error"

    def raise_exception():
        raise Exception("Raised Sample Exception")

    def raise_other_datatype():
        raise 1

    def check_try_except_scope():
        try:
            x = 1
            raise Exception("Throw")
        except:
            print("printing x", x)
            y = 1

        # This should not throw as x,y are both in scope and correctly set
        print(x,y)

        try:
            a = 1
        except:
            b = 1

        # local variable b accessed before assignment
        print(a, b)

    def raise_script_error():
        lis = [1,2,3]
        lis[4]


    check_exception(assert_false,"Assertion failed")
    check_exception(assert_false_with_message,"Assertion failed with error.*Sample Error")
    check_exception(raise_exception,"Raised Sample Exception")
    check_exception(raise_other_datatype,"Only Exception.. can be thrown")
    check_exception(check_try_except_scope, "Local variable b accessed before assignment")
    check_exception(raise_script_error, "trying to access.* index for list of size=3")
    check_bare_except()

    return {}
