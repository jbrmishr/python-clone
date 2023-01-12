# This script is used by test_misc.

import _imp
import _testinternalcapi
import json
import os
import sys


def import_singlephase():
    assert '_singlephase' not in sys.modules
    try:
        import _singlephase
    except ImportError:
        return True
    else:
        del sys.modules['_singlephase']
        return False


def check_singlephase(override):
    # Check using the default setting.
    settings_initial = _testinternalcapi.get_interp_settings()
    allowed_initial = import_singlephase()
    assert(_testinternalcapi.get_interp_settings() == settings_initial)

    # Apply the override and check.
    override_initial = _imp._override_multi_interp_extensions_check(override)
    settings_after = _testinternalcapi.get_interp_settings()
    allowed_after = import_singlephase()

    # Apply the override again and check.
    override_after = _imp._override_multi_interp_extensions_check(override)
    settings_noop = _testinternalcapi.get_interp_settings()
    allowed_noop = import_singlephase()

    # Restore the original setting and check.
    override_noop = _imp._override_multi_interp_extensions_check(override_initial)
    settings_restored = _testinternalcapi.get_interp_settings()
    allowed_restored = import_singlephase()

    # Restore the original setting again.
    override_restored = _imp._override_multi_interp_extensions_check(override_initial)
    assert(_testinternalcapi.get_interp_settings() == settings_restored)

    return {
        'requested': override,
        'override__initial': override_initial,
        'override_after': override_after,
        'override_noop': override_noop,
        'override_restored': override_restored,
        'settings__initial': settings_initial,
        'settings_after': settings_after,
        'settings_noop': settings_noop,
        'settings_restored': settings_restored,
        'allowed__initial': allowed_initial,
        'allowed_after': allowed_after,
        'allowed_noop': allowed_noop,
        'allowed_restored': allowed_restored,
    }


def run_singlephase_check(override, outfd):
    with os.fdopen(outfd, 'w') as outfile:
        sys.stdout = outfile
        sys.stderr = outfile
        results = check_singlephase(override)
        json.dump(results, outfile)
