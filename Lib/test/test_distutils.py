"""Tests for distutils.

The tests for distutils are defined in the Lib.test.distutils_tests package;
the test_suite() function there returns a test suite that's ready to
be run.
"""

import Lib.test.distutils_tests as distutils_tests
import test.support


def test_main():
    # used by regrtest
    test.support.run_unittest(distutils_tests.test_suite())
    test.support.reap_children()


def load_tests(*_):
    # used by unittest
    return distutils_tests.test_suite()


if __name__ == "__main__":
    test_main()
