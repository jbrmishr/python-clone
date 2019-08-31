from test.support import temp_dir
from test.support.script_helper import assert_python_failure
import unittest
import sys
import cgitb

class BadGetAttr:
    """Support class to ValueError on getattr while examining the
    traceback.
    """
    def __init__(self):
        self._bad = False
        self._h = self._fn
    def __getattr__(self, attr):
        if self._bad:
            raise ValueError('getattr exception')
        return self.__dict__['_'+attr]
    def _fn(self):
        self._bad = True
        raise ValueError('real exception')

class BadRepr:
    """Support class to ValueError on repr while examining the
    traceback."""
    def __repr__(self):
        raise ValueError('repr exception')
    def fn(self):
        raise ValueError('real exception')


class TestCgitb(unittest.TestCase):

    def test_fonts(self):
        text = "Hello Robbie!"
        self.assertEqual(cgitb.small(text), "<small>{}</small>".format(text))
        self.assertEqual(cgitb.strong(text), "<strong>{}</strong>".format(text))
        self.assertEqual(cgitb.grey(text),
                         '<font color="#909090">{}</font>'.format(text))

    def test_blanks(self):
        self.assertEqual(cgitb.small(""), "")
        self.assertEqual(cgitb.strong(""), "")
        self.assertEqual(cgitb.grey(""), "")

    def test_html(self):
        try:
            raise ValueError("Hello World")
        except ValueError as err:
            # If the html was templated we could do a bit more here.
            # At least check that we get details on what we just raised.
            html = cgitb.html(sys.exc_info())
            self.assertIn("ValueError", html)
            self.assertIn(str(err), html)

    def test_text(self):
        try:
            raise ValueError("Hello World")
        except ValueError as err:
            text = cgitb.text(sys.exc_info())
            self.assertIn("ValueError", text)
            self.assertIn("Hello World", text)

    def test_syshook_no_logdir_default_format(self):
        with temp_dir() as tracedir:
            rc, out, err = assert_python_failure(
                  '-c',
                  ('import cgitb; cgitb.enable(logdir=%s); '
                   'raise ValueError("Hello World")') % repr(tracedir))
        out = out.decode(sys.getfilesystemencoding())
        self.assertIn("ValueError", out)
        self.assertIn("Hello World", out)
        self.assertIn("<strong>&lt;module&gt;</strong>", out)
        # By default we emit HTML markup.
        self.assertIn('<p>', out)
        self.assertIn('</p>', out)

    def test_syshook_no_logdir_text_format(self):
        # Issue 12890: we were emitting the <p> tag in text mode.
        with temp_dir() as tracedir:
            rc, out, err = assert_python_failure(
                  '-c',
                  ('import cgitb; cgitb.enable(format="text", logdir=%s); '
                   'raise ValueError("Hello World")') % repr(tracedir))
        out = out.decode(sys.getfilesystemencoding())
        self.assertIn("ValueError", out)
        self.assertIn("Hello World", out)
        self.assertNotIn('<p>', out)
        self.assertNotIn('</p>', out)

    def test_masking_getattr_exception(self):
        # bpo 1047397: if examining the traceback provoked a
        # ValueError from attribute look-up, the original exception
        # was lost.  Provoking this artificially involves a convoluted
        # dance.
        bga = BadGetAttr()
        try:
            bga.h()
        except ValueError as err:
            html = cgitb.html(sys.exc_info())
            self.assertIn('ValueError', html)
            self.assertIn(str(err), html)

    def test_text_masking_getattr_exception(self):
        # bpo 1047397: if examining the traceback provoked a
        # ValueError from attribute look-up, the original exception
        # was lost.  Provoking this artificially involves a convoluted
        # dance.
        bga = BadGetAttr()
        try:
            bga.h()
        except ValueError as err:
            text = cgitb.text(sys.exc_info())
            self.assertIn('ValueError', text)
            self.assertIn(str(err), text)

    def test_masking_repr_exception(self):
        # bpo 1047397: if examining the traceback provoked a
        # ValueError from a repr, the original exception was lost.
        br = BadRepr()
        try:
            br.fn()
        except ValueError as err:
            html = cgitb.html(sys.exc_info())
            self.assertIn('ValueError', html)
            self.assertIn(str(err), html)

    def test_masking_repr_exception(self):
        # bpo 1047397: if examining the traceback provoked a
        # ValueError from a repr, the original exception was lost.
        br = BadRepr()
        try:
            br.fn()
        except ValueError as err:
            text = cgitb.text(sys.exc_info())
            self.assertIn('ValueError', text)
            self.assertIn(str(err), text)


if __name__ == "__main__":
    unittest.main()
