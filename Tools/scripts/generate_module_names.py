# This script lists the names of standard library modules
# to update Python/module_names.h
import os.path
import re
import subprocess
import sys
import sysconfig


SRC_DIR = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
STDLIB_PATH = os.path.join(SRC_DIR, 'Lib')
MODULES_SETUP = os.path.join(SRC_DIR, 'Modules', 'Setup')
SETUP_PY = os.path.join(SRC_DIR, 'setup.py')

IGNORE = {
    '__init__',
    '__pycache__',
    'site-packages',

    # test modules
    '__phello__.foo',
    '_ctypes_test',
    '_testbuffer',
    '_testcapi',
    '_testconsole',
    '_testimportmultiple',
    '_testinternalcapi',
    '_testmultiphase',
    '_xxtestfuzz',
    'distutils.tests',
    'idlelib.idle_test',
    'lib2to3.tests',
    'test',
    'xxlimited',
    'xxlimited_35',
    'xxsubtype',
}

# Windows extension modules
WINDOWS_MODULES = (
    '_msi',
    '_testconsole',
    '_winapi',
    'msvcrt',
    'nt',
    'winreg',
    'winsound'
)


# Pure Python modules (Lib/*.py)
def list_python_modules(names):
    for filename in os.listdir(STDLIB_PATH):
        if not filename.endswith(".py"):
            continue
        name = filename.removesuffix(".py")
        names.add(name)


def _list_sub_packages(path, names, parent=None):
    for name in os.listdir(path):
        if name in IGNORE:
            continue
        package_path = os.path.join(path, name)
        if not os.path.isdir(package_path):
            continue
        if not any(package_file.endswith(".py")
                   for package_file in os.listdir(package_path)):
            continue
        if parent:
            qualname = f"{parent}.{name}"
        else:
            qualname = name
        if qualname in IGNORE:
            continue
        names.add(qualname)
        _list_sub_packages(package_path, names, qualname)


# Packages and sub-packages
def list_packages(names):
    _list_sub_packages(STDLIB_PATH, names)


# Extension modules built by setup.py
def list_setup_extensions(names):
    cmd = [sys.executable, SETUP_PY, "-q", "build", "--list-module-names"]
    output = subprocess.check_output(cmd)
    output = output.decode("utf8")
    extensions = output.splitlines()
    names |= set(extensions)


# Built-in and extension modules built by Modules/Setup
def list_modules_setup_extensions(names):
    assign_var = re.compile("^[A-Z]+=")

    with open(MODULES_SETUP, encoding="utf-8") as modules_fp:
        for line in modules_fp:
            # Strip comment
            line = line.partition("#")[0]
            line = line.rstrip()
            if not line:
                continue
            if assign_var.match(line):
                # Ignore "VAR=VALUE"
                continue
            if line in ("*disabled*", "*shared*"):
                continue
            parts = line.split()
            if len(parts) < 2:
                continue
            # "errno errnomodule.c" => write "errno"
            name = parts[0]
            names.add(name)


def list_modules():
    names = set(sys.builtin_module_names) | set(WINDOWS_MODULES)
    list_modules_setup_extensions(names)
    list_setup_extensions(names)
    list_packages(names)
    list_python_modules(names)
    names -= set(IGNORE)
    return names


def write_modules(fp, names):
    print("// Auto-generated by Tools/scripts/generate_module_names.py.", file=fp)
    print("// List used to create sys.module_names.", file=fp)
    print(file=fp)
    print("static const char* _Py_module_names[] = {", file=fp)
    for name in sorted(names):
        print(f'"{name}",', file=fp)
    print("};", file=fp)


def main():
    if not sysconfig.is_python_build():
        print(f"ERROR: {sys.executable} is not a Python build",
              file=sys.stderr)
        sys.exit(1)

    fp = sys.stdout
    names = list_modules()
    write_modules(fp, names)


if __name__ == "__main__":
    main()
