"""Sanity-check tests for the "freeze" tool."""

import os
import os.path
import subprocess
import sys
import textwrap
import unittest

from . import imports_under_tool
with imports_under_tool('freeze', 'test'):
    import freeze as helper


@unittest.skipIf(sys.platform.startswith('win'), 'not supported on Windows')
class TestFreeze(unittest.TestCase):

    def test_freeze_simple_script(self):
        os.makedirs(helper.OUTDIR, exist_ok=True)
        script = textwrap.dedent("""
            import sys
            print('running...')
            sys.exit(0)
            """)
        scriptfile = os.path.join(helper.OUTDIR, 'app.py')
        with open(scriptfile, 'w') as outfile:
            outfile.write(script)
        python = helper.pre_freeze(verbose=False)
        executable = helper.freeze(python, scriptfile, verbose=False)
        text = helper.run(executable)
        self.assertEqual(text, 'running...')
