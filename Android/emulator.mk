# Android/emulator.mk

EMULATOR_CONSOLE_PORT := 5554
avd_name := $(BUILD_TYPE)
native_python_exe := $(native_build_dir)/python -B
export ADB := $(ANDROID_SDK_ROOT)/platform-tools/adb


# Rules.
emulator: emulator_checks _emulator adb_shell

emulator_install: wipe_data := -wipe-data
emulator_install: emulator_checks dist _emulator
	@echo "---> Install Python on the avd and start the emulator."
	$(native_python_exe) $(py_srcdir)/Android/tools/install.py

adb_shell:
	@echo "---> Run an adb shell."
	$(native_python_exe) $(py_srcdir)/Android/tools/python_shell.py
	@$(ADB) shell

install: emulator_install
	@echo "---> Run an adb shell."
	$(native_python_exe) $(py_srcdir)/Android/tools/python_shell.py
	@$(ADB) shell

ifneq ($(PYTHON_ARGS), )
python: emulator_install
	@echo "---> Run <python $(PYTHON_ARGS)>."
	-$(native_python_exe) $(py_srcdir)/Android/tools/python_shell.py "$(PYTHON_ARGS)"
	$(MAKE) kill_emulator
endif

buildbottest: TESTRUNNER := $(native_python_exe) $(py_srcdir)/Android/tools/python_shell.py
buildbottest: TESTRUNNER += $(TESTPYTHONOPTS) $(SYS_EXEC_PREFIX)/bin/run_tests.py
buildbottest: export PY_SRCDIR := $(py_srcdir)
buildbottest: export PATH := $(native_build_dir):$(PATH)
buildbottest: emulator_install
	@echo "---> Run buildbottest."
	-$(MAKE) -C $(py_host_dir) TESTRUNNER="$(TESTRUNNER)" buildbottest
	$(MAKE) kill_emulator

gdb: export APP_ABI := $(APP_ABI)
gdb: export PY_HOST_DIR := $(py_host_dir)
gdb: export LIB_DYNLOAD = $(py_host_dir)/$(shell cat $(py_host_dir)/pybuilddir.txt)
gdb:
	@echo "---> Start gdb."
	$(py_srcdir)/Android/tools/ndk_gdb.py $(ANDROID_API) $(py_srcdir) python

$(avd_dir)/$(avd_name):
	@echo "---> Create the avd."
	mkdir -p $(avd_dir)
	echo "no" | $(ANDROID_SDK_ROOT)/tools/bin/avdmanager create avd --force --name $(avd_name) \
	    --package "system-images;android-$(ANDROID_API);default;$(APP_ABI)" \
	    --abi default/$(APP_ABI) --path $(avd_dir)/$(avd_name)

kill_emulator:
	@echo "---> Kill the emulator."
	$(native_python_exe) $(py_srcdir)/Android/tools/kill_emulator.py

emulator_checks:
	@echo "---> Check that an emulator is not currently running."
	@if test -f "$(native_python_exe)"; then \
	    cd $(py_srcdir)/Android/tools; \
	        $(native_python_exe) -c "import sys, android_utils; \
                    sys.exit(android_utils.emulator_listens($(EMULATOR_CONSOLE_PORT)))"; \
	fi

_emulator: $(avd_dir)/$(avd_name)
	@echo "---> Start the emulator."
	$(ANDROID_SDK_ROOT)/emulator/emulator -avd $(avd_name) $(wipe_data) \
	    $(EMULATOR_CMD_LINE_OPTIONS) &
	@ echo "---> Waiting for device to be ready."
	@$(ADB) wait-for-device shell getprop init.svc.bootanim; \
	    echo "---> Device ready."

avdclean:
	@echo "---> Remove the AVD."
	if test -d "$(avd_dir)/$(avd_name)"; then \
	    $(ANDROID_SDK_ROOT)/tools/bin/avdmanager delete avd --name $(avd_name); \
	    rm -rf $(avd_dir)/$(avd_name); \
	fi
	-rmdir $(avd_dir)

.PHONY: emulator adb_shell install python buildbottest gdb \
        avdclean _emulator kill_emulator emulator_checks emulator_install
