# This script generates a Python interface for an Apple Macintosh Manager.
# It uses the "bgen" package to generate C code.
# The function specifications are generated by scanning the mamager's header file,
# using the "scantools" package (customized for this particular manager).

import string

# Declarations that change for each manager
MACHEADERFILE = 'OSA.h'		# The Apple header file
MODNAME = '_OSA'				# The name of the module

# The following is *usually* unchanged but may still require tuning
MODPREFIX = 'OSA'			# The prefix for module-wide routines
OBJECTPREFIX = 'OSAObj'	# The prefix for object methods
INPUTFILE = string.lower(MODPREFIX) + 'gen.py' # The file generated by the scanner
OUTPUTFILE = MODNAME + "module.c"	# The file generated by this program

from macsupport import *

# Create the type objects

includestuff = includestuff + """
#if PY_VERSION_HEX < 0x02040000
PyObject *PyMac_GetOSErrException(void);
#endif
#include <Carbon/Carbon.h>

#ifdef USE_TOOLBOX_OBJECT_GLUE
extern PyObject *_OSAObj_New(ComponentInstance);
extern int _OSAObj_Convert(PyObject *, ComponentInstance *);

#define OSAObj_New _OSAObj_New
#define OSAObj_Convert _OSAObj_Convert
#endif
"""

initstuff = initstuff + """
/*
	PyMac_INIT_TOOLBOX_OBJECT_NEW(ComponentInstance, OSAObj_New);
	PyMac_INIT_TOOLBOX_OBJECT_CONVERT(ComponentInstance, OSAObj_Convert);
*/
"""

ComponentInstance = OpaqueByValueType('ComponentInstance', OBJECTPREFIX)
OSAError = OSErrType("OSAError", "l")
# OSALocalOrGlobal = Type("OSALocalOrGlobal", "l")
OSAID = Type("OSAID", "l")
OSADebugCallFrameRef = Type("OSADebugCallFrameRef", "l")
OSADebugSessionRef = Type("OSADebugSessionRef", "l")
OSADebugStepKind = Type("OSADebugStepKind", "l")
DescType = OSTypeType("DescType")
AEDesc = OpaqueType('AEDesc')
AEDesc_ptr = OpaqueType('AEDesc')
AEAddressDesc = OpaqueType('AEAddressDesc', 'AEDesc')
AEAddressDesc_ptr = OpaqueType('AEAddressDesc', 'AEDesc')
AEDescList = OpaqueType('AEDescList', 'AEDesc')
AEDescList_ptr = OpaqueType('AEDescList', 'AEDesc')
AERecord = OpaqueType('AERecord', 'AEDesc')
AERecord_ptr = OpaqueType('AERecord', 'AEDesc')
AppleEvent = OpaqueType('AppleEvent', 'AEDesc')
AppleEvent_ptr = OpaqueType('AppleEvent', 'AEDesc')

# NOTE: at the moment OSA.ComponentInstance is not a subclass
# of Cm.ComponentInstance. If this is a problem it can be fixed.
class MyObjectDefinition(PEP253Mixin, GlobalObjectDefinition):
	def outputCheckNewArg(self):
		Output("""if (itself == NULL) {
					PyErr_SetString(OSA_Error,"NULL ComponentInstance");
					return NULL;
				}""")

# Create the generator groups and link them
module = MacModule(MODNAME, MODPREFIX, includestuff, finalstuff, initstuff)
object = MyObjectDefinition('ComponentInstance', OBJECTPREFIX,
		'ComponentInstance')
module.addobject(object)

# Create the generator classes used to populate the lists
Function = OSErrWeakLinkFunctionGenerator
Method = OSErrWeakLinkMethodGenerator

# Test which types we are still missing.
execfile(string.lower(MODPREFIX) + 'typetest.py')

# Create and populate the lists
functions = []
methods = []
execfile(INPUTFILE)

# add the populated lists to the generator groups
# (in a different wordl the scan program would generate this)
for f in functions: module.add(f)
for f in methods: object.add(f)

# generate output (open the output file as late as possible)
SetOutputFileName(OUTPUTFILE)
module.generate()

