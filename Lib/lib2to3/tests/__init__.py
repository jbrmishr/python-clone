# Author: Collin Winter

import os
import warnings

from test.support import load_package_tests

def load_tests(*args):
    with warnings.catch_warnings():
        warnings.simplefilter('ignore', DeprecationWarning)
        return load_package_tests(os.path.dirname(__file__), *args)
