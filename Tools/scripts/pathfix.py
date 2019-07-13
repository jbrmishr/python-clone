#!/usr/bin/env python3

# Change the #! line (shebang) occurring in Python scripts.  The new interpreter
# pathname must be given with a -i option.
#
# Command line arguments are files or directories to be processed.
# Directories are searched recursively for files whose name looks
# like a python module.
# Symbolic links are always ignored (except as explicit directory
# arguments).
# The original file is kept as a back-up (with a "~" attached to its name),
# -n flag can be used to disable this.

# Sometimes you may find shebangs with flags such as `#! /usr/bin/env python -si`.
# Normally, pathfix overwrites the entire line, including the flags.
# To keep flags from the original shebang line, use -f "".
# To add a flag, pass it to the -f option. For example, -f s adds the `s` flag
# if not already present.
# The new flag will be added before any existing flags.
# Adding multiple flags, multi-letter flags, or options with arguments
# is not supported.


# Undoubtedly you can do this using find and sed or perl, but this is
# a nice example of Python code that recurses down a directory tree
# and uses regular expressions.  Also note several subtleties like
# preserving the file's mode and avoiding to even write a temp file
# when no changes are needed for a file.
#
# NB: by changing only the function fixfile() you can turn this
# into a program for a different change to Python programs...

import sys
import re
import os
from stat import *
import getopt

err = sys.stderr.write
dbg = err
rep = sys.stdout.write

new_interpreter = None
preserve_timestamps = False
create_backup = True
add_flag = None


def main():
    global new_interpreter
    global preserve_timestamps
    global create_backup
    global add_flag
    usage = ('usage: %s -i /interpreter -p -n -f flags-to-add file-or-directory ...\n' %
             sys.argv[0])
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'i:f:pn')
    except getopt.error as msg:
        err(str(msg) + '\n')
        err(usage)
        sys.exit(2)
    for o, a in opts:
        if o == '-i':
            new_interpreter = a.encode()
        if o == '-p':
            preserve_timestamps = True
        if o == '-n':
            create_backup = False
        if o == '-f':
            if len(a) > 1:
                err('-f: just one literal flags can be added')
                sys.exit(2)
            add_flag = a.encode()
    if not new_interpreter or not new_interpreter.startswith(b'/') or \
           not args:
        err('-i option or file-or-directory missing\n')
        err(usage)
        sys.exit(2)
    bad = 0
    for arg in args:
        if os.path.isdir(arg):
            if recursedown(arg): bad = 1
        elif os.path.islink(arg):
            err(arg + ': will not process symbolic links\n')
            bad = 1
        else:
            if fix(arg): bad = 1
    sys.exit(bad)

ispythonprog = re.compile(r'^[a-zA-Z0-9_]+\.py$')
def ispython(name):
    return bool(ispythonprog.match(name))

def recursedown(dirname):
    dbg('recursedown(%r)\n' % (dirname,))
    bad = 0
    try:
        names = os.listdir(dirname)
    except OSError as msg:
        err('%s: cannot list directory: %r\n' % (dirname, msg))
        return 1
    names.sort()
    subdirs = []
    for name in names:
        if name in (os.curdir, os.pardir): continue
        fullname = os.path.join(dirname, name)
        if os.path.islink(fullname): pass
        elif os.path.isdir(fullname):
            subdirs.append(fullname)
        elif ispython(name):
            if fix(fullname): bad = 1
    for fullname in subdirs:
        if recursedown(fullname): bad = 1
    return bad

def fix(filename):
##  dbg('fix(%r)\n' % (filename,))
    try:
        f = open(filename, 'rb')
    except IOError as msg:
        err('%s: cannot open: %r\n' % (filename, msg))
        return 1
    with f:
        line = f.readline()
        fixed = fixline(line)
        if line == fixed:
            rep(filename+': no change\n')
            return
        head, tail = os.path.split(filename)
        tempname = os.path.join(head, '@' + tail)
        try:
            g = open(tempname, 'wb')
        except IOError as msg:
            err('%s: cannot create: %r\n' % (tempname, msg))
            return 1
        with g:
            rep(filename + ': updating\n')
            g.write(fixed)
            BUFSIZE = 8*1024
            while 1:
                buf = f.read(BUFSIZE)
                if not buf: break
                g.write(buf)

    # Finishing touch -- move files

    mtime = None
    atime = None
    # First copy the file's mode to the temp file
    try:
        statbuf = os.stat(filename)
        mtime = statbuf.st_mtime
        atime = statbuf.st_atime
        os.chmod(tempname, statbuf[ST_MODE] & 0o7777)
    except OSError as msg:
        err('%s: warning: chmod failed (%r)\n' % (tempname, msg))
    # Then make a backup of the original file as filename~
    if create_backup:
        try:
            os.rename(filename, filename + '~')
        except OSError as msg:
            err('%s: warning: backup failed (%r)\n' % (filename, msg))
    else:
        try:
            os.remove(filename)
        except OSError as msg:
            err('%s: warning: removing failed (%r)\n' % (filename, msg))
    # Now move the temp file to the original file
    try:
        os.rename(tempname, filename)
    except OSError as msg:
        err('%s: rename failed (%r)\n' % (filename, msg))
        return 1
    if preserve_timestamps:
        if atime and mtime:
            try:
                os.utime(filename, (atime, mtime))
            except OSError as msg:
                err('%s: reset of timestamp failed (%r)\n' % (filename, msg))
                return 1
    # Return success
    return 0


def parse_shebang(shebangline):
    """takes shebangline and returns tuple with string of shebang's flags
    and string of shebang's args
    parse_shebang(b'#!/usr/bin/python -f sfj') --> (b'f',b'sfj')
    """
    end = len(shebangline)
    start = shebangline.find(b' -', 0, end)  # .find() returns index at space
    if start == -1:
        return b'', b''

    start += 2
    flags = shebangline[start:end].split()
    args = b''
    if len(flags) > 1:
        args = b' '.join(flags[1:])
    flags = flags[0]
    return flags, args



def fixline(line):
    if not line.startswith(b'#!'):
        return line

    if b"python" not in line:
        return line

    if add_flag is not None:
        flags, args = parse_shebang(line)
        if args:
            args = b' ' + args

        if flags == b'' and add_flag == b'':
            flags = b''

        elif add_flag in flags:
            flags = b' -' + flags + args

        else:
            flags = b' -' + add_flag + flags + args
    else:
        flags = b''
    return b'#! ' + new_interpreter + flags + b'\n'


if __name__ == '__main__':
    main()
