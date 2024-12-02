#!/usr/bin/env python3

import argparse
import contextlib
import functools
import os
import shutil
import subprocess
import sys
import sysconfig
import tempfile
from pathlib import Path
from textwrap import dedent

try:
    from os import process_cpu_count as cpu_count
except ImportError:
    from os import cpu_count


EMSCRIPTEN_DIR = Path(__file__).parent
CHECKOUT = EMSCRIPTEN_DIR.parent.parent.parent

CROSS_BUILD_DIR = CHECKOUT / "cross-build"
BUILD_DIR = CROSS_BUILD_DIR / "build"
HOST_TRIPLE = "wasm32-emscripten"
HOST_DIR = CROSS_BUILD_DIR / HOST_TRIPLE

LOCAL_SETUP = CHECKOUT / "Modules" / "Setup.local"
LOCAL_SETUP_MARKER = "# Generated by Tools/wasm/emscripten.py\n".encode("utf-8")


def updated_env(updates={}):
    """Create a new dict representing the environment to use.

    The changes made to the execution environment are printed out.
    """
    env_defaults = {}
    # https://reproducible-builds.org/docs/source-date-epoch/
    git_epoch_cmd = ["git", "log", "-1", "--pretty=%ct"]
    try:
        epoch = subprocess.check_output(git_epoch_cmd, encoding="utf-8").strip()
        env_defaults["SOURCE_DATE_EPOCH"] = epoch
    except subprocess.CalledProcessError:
        pass  # Might be building from a tarball.
    # This layering lets SOURCE_DATE_EPOCH from os.environ takes precedence.
    environment = env_defaults | os.environ | updates

    env_diff = {}
    for key, value in environment.items():
        if os.environ.get(key) != value:
            env_diff[key] = value

    print("🌎 Environment changes:")
    for key in sorted(env_diff.keys()):
        print(f"  {key}={env_diff[key]}")

    return environment


def subdir(working_dir, *, clean_ok=False):
    """Decorator to change to a working directory."""

    def decorator(func):
        @functools.wraps(func)
        def wrapper(context):
            try:
                tput_output = subprocess.check_output(
                    ["tput", "cols"], encoding="utf-8"
                )
                terminal_width = int(tput_output.strip())
            except subprocess.CalledProcessError:
                terminal_width = 80
            print("⎯" * terminal_width)
            print("📁", working_dir)
            if clean_ok and getattr(context, "clean", False) and working_dir.exists():
                print("🚮 Deleting directory (--clean)...")
                shutil.rmtree(working_dir)

            working_dir.mkdir(parents=True, exist_ok=True)

            with contextlib.chdir(working_dir):
                return func(context, working_dir)

        return wrapper

    return decorator


def call(command, *, quiet, **kwargs):
    """Execute a command.

    If 'quiet' is true, then redirect stdout and stderr to a temporary file.
    """
    print("❯", " ".join(map(str, command)))
    if not quiet:
        stdout = None
        stderr = None
    else:
        stdout = tempfile.NamedTemporaryFile(
            "w",
            encoding="utf-8",
            delete=False,
            prefix="cpython-emscripten-",
            suffix=".log",
        )
        stderr = subprocess.STDOUT
        print(f"📝 Logging output to {stdout.name} (--quiet)...")

    subprocess.check_call(command, **kwargs, stdout=stdout, stderr=stderr)


def build_platform():
    """The name of the build/host platform."""
    # Can also be found via `config.guess`.`
    return sysconfig.get_config_var("BUILD_GNU_TYPE")


def build_python_path():
    """The path to the build Python binary."""
    binary = BUILD_DIR / "python"
    if not binary.is_file():
        binary = binary.with_suffix(".exe")
        if not binary.is_file():
            raise FileNotFoundError("Unable to find `python(.exe)` in " f"{BUILD_DIR}")

    return binary


@subdir(BUILD_DIR, clean_ok=True)
def configure_build_python(context, working_dir):
    """Configure the build/host Python."""
    if LOCAL_SETUP.exists():
        print(f"👍 {LOCAL_SETUP} exists ...")
    else:
        print(f"📝 Touching {LOCAL_SETUP} ...")
        LOCAL_SETUP.write_bytes(LOCAL_SETUP_MARKER)

    configure = [os.path.relpath(CHECKOUT / "configure", working_dir)]
    if context.args:
        configure.extend(context.args)

    call(configure, quiet=context.quiet)


@subdir(BUILD_DIR)
def make_build_python(context, working_dir):
    """Make/build the build Python."""
    call(["make", "--jobs", str(cpu_count()), "all"], quiet=context.quiet)

    binary = build_python_path()
    cmd = [
        binary,
        "-c",
        "import sys; " "print(f'{sys.version_info.major}.{sys.version_info.minor}')",
    ]
    version = subprocess.check_output(cmd, encoding="utf-8").strip()

    print(f"🎉 {binary} {version}")


@subdir(HOST_DIR, clean_ok=True)
def configure_emscripten_python(context, working_dir):
    """Configure the emscripten/host build."""
    config_site = os.fsdecode(
        CHECKOUT / "Tools" / "wasm" / "config.site-wasm32-emscripten"
    )

    emscripten_build_dir = working_dir.relative_to(CHECKOUT)

    python_build_dir = BUILD_DIR / "build"
    lib_dirs = list(python_build_dir.glob("lib.*"))
    assert (
        len(lib_dirs) == 1
    ), f"Expected a single lib.* directory in {python_build_dir}"
    lib_dir = os.fsdecode(lib_dirs[0])
    pydebug = lib_dir.endswith("-pydebug")
    python_version = lib_dir.removesuffix("-pydebug").rpartition("-")[-1]
    sysconfig_data = (
        f"{emscripten_build_dir}/build/lib.emscripten-wasm32-{python_version}"
    )
    if pydebug:
        sysconfig_data += "-pydebug"

    host_runner = context.host_runner
    env_additions = {"CONFIG_SITE": config_site, "HOSTRUNNER": host_runner}
    build_python = os.fsdecode(build_python_path())
    configure = [
        "emconfigure",
        os.path.relpath(CHECKOUT / "configure", working_dir),
        "CFLAGS=-DPY_CALL_TRAMPOLINE -sUSE_BZIP2",
        f"--host={HOST_TRIPLE}",
        f"--build={build_platform()}",
        f"--with-build-python={build_python}",
        "--without-pymalloc",
        "--disable-shared",
        "--disable-ipv6",
        "--enable-big-digits=30",
        "--enable-wasm-dynamic-linking",
        f"--prefix={HOST_DIR}",
    ]
    if pydebug:
        configure.append("--with-pydebug")
    if context.args:
        configure.extend(context.args)
    call(
        configure,
        env=updated_env(env_additions),
        quiet=context.quiet,
    )

    shutil.copy(EMSCRIPTEN_DIR / "node_entry.mjs", working_dir / "node_entry.mjs")

    node_entry = working_dir / "node_entry.mjs"
    exec_script = working_dir / "python.sh"
    exec_script.write_text(
        dedent(
            """\
            #!/bin/sh

            # Macs come with free BSD coreutils which doesn't have the -s option
            # so feature detect and work around it.
            if which grealpath > /dev/null; then
                # It has brew installed gnu core utils, use that
                REALPATH="grealpath -s"
            elif which realpath > /dev/null && realpath --version | grep GNU > /dev/null; then
                # realpath points to GNU realpath so use it.
                REALPATH="realpath -s"
            else
                # Shim for macs without GNU coreutils
                abs_path () {
                    echo "$(cd "$(dirname "$1")" || exit; pwd)/$(basename "$1")"
                }
                REALPATH=abs_path
            fi
            """
            f"""
            # We compute our own path, not following symlinks and pass it in so that
            # node_entry.mjs can set sys.executable correctly.
            # Intentionally allow word splitting on NODEFLAGS.
            exec {host_runner} {node_entry} ${NODEFLAGS} "$($REALPATH "$0")" "$@"
            """
        )
    )
    exec_script.chmod(0o755)
    print(f"🏃‍♀️ Created {exec_script} ... ")
    sys.stdout.flush()


@subdir(HOST_DIR)
def make_emscripten_python(context, working_dir):
    """Run `make` for the emscripten/host build."""
    call(
        ["make", "--jobs", str(cpu_count()), "all"],
        env=updated_env(),
        quiet=context.quiet,
    )

    exec_script = working_dir / "python.sh"
    subprocess.check_call([exec_script, "--version"])


def build_all(context):
    """Build everything."""
    steps = [
        configure_build_python,
        make_build_python,
        configure_emscripten_python,
        make_emscripten_python,
    ]
    for step in steps:
        step(context)


def clean_contents(context):
    """Delete all files created by this script."""
    if CROSS_BUILD_DIR.exists():
        print(f"🧹 Deleting {CROSS_BUILD_DIR} ...")
        shutil.rmtree(CROSS_BUILD_DIR)

    if LOCAL_SETUP.exists():
        with LOCAL_SETUP.open("rb") as file:
            if file.read(len(LOCAL_SETUP_MARKER)) == LOCAL_SETUP_MARKER:
                print(f"🧹 Deleting generated {LOCAL_SETUP} ...")


def main():
    default_host_runner = "node"

    parser = argparse.ArgumentParser()
    subcommands = parser.add_subparsers(dest="subcommand")
    build = subcommands.add_parser("build", help="Build everything")
    configure_build = subcommands.add_parser(
        "configure-build-python", help="Run `configure` for the " "build Python"
    )
    make_build = subcommands.add_parser(
        "make-build-python", help="Run `make` for the build Python"
    )
    configure_host = subcommands.add_parser(
        "configure-host",
        help="Run `configure` for the host/emscripten (pydebug builds are inferred from the build Python)",
    )
    make_host = subcommands.add_parser("make-host", help="Run `make` for the host/emscripten")
    clean = subcommands.add_parser(
        "clean", help="Delete files and directories created by this script"
    )
    for subcommand in build, configure_build, make_build, configure_host, make_host:
        subcommand.add_argument(
            "--quiet",
            action="store_true",
            default=False,
            dest="quiet",
            help="Redirect output from subprocesses to a log file",
        )
    for subcommand in configure_build, configure_host:
        subcommand.add_argument(
            "--clean",
            action="store_true",
            default=False,
            dest="clean",
            help="Delete any relevant directories before building",
        )
    for subcommand in build, configure_build, configure_host:
        subcommand.add_argument(
            "args", nargs="*", help="Extra arguments to pass to `configure`"
        )
    for subcommand in build, configure_host:
        subcommand.add_argument(
            "--host-runner",
            action="store",
            default=default_host_runner,
            dest="host_runner",
            help="Command template for running the emscripten host"
            f"`{default_host_runner}`)",
        )

    context = parser.parse_args()

    dispatch = {
        "configure-build-python": configure_build_python,
        "make-build-python": make_build_python,
        "configure-host": configure_emscripten_python,
        "make-host": make_emscripten_python,
        "build": build_all,
        "clean": clean_contents,
    }

    if not context.subcommand:
        # No command provided, display help and exit
        print("Expected one of", ", ".join(sorted(dispatch.keys())), file=sys.stderr)
        parser.print_help(sys.stderr)
        sys.exit(1)
    dispatch[context.subcommand](context)


if __name__ == "__main__":
    main()
