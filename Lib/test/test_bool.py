# Test properties of bool promised by PEP 285

from test_support import verbose, TestFailed, TESTFN, vereq

def veris(a, b):
    if a is not b:
        raise TestFailed, "%r is %r" % (a, b)

def verisnot(a, b):
    if a is b:
        raise TestFailed, "%r is %r" % (a, b)

try:
    class C(bool):
        pass
except TypeError:
    pass
else:
    raise TestFailed, "bool should not be subclassable"

try:
    int.__new__(bool, 0)
except TypeError:
    pass
else:
    raise TestFailed, "should not be able to create new bool instances"

vereq(int(False), 0)
verisnot(int(False), False)
vereq(int(True), 1)
verisnot(int(True), True)

vereq(+False, 0)
verisnot(+False, False)
vereq(-False, 0)
verisnot(-False, False)
vereq(abs(False), 0)
verisnot(abs(False), False)
vereq(+True, 1)
verisnot(+True, True)
vereq(-True, -1)
vereq(abs(True), 1)
verisnot(abs(True), True)
vereq(~False, -1)
vereq(~True, -2)

vereq(False+2, 2)
vereq(True+2, 3)
vereq(2+False, 2)
vereq(2+True, 3)

vereq(False+False, 0)
verisnot(False+False, False)
vereq(False+True, 1)
verisnot(False+True, True)
vereq(True+False, 1)
verisnot(True+False, True)
vereq(True+True, 2)

vereq(True-True, 0)
verisnot(True-True, False)
vereq(False-False, 0)
verisnot(False-False, False)
vereq(True-False, 1)
verisnot(True-False, True)
vereq(False-True, -1)

vereq(True*1, 1)
vereq(False*1, 0)
verisnot(False*1, False)

vereq(True/1, 1)
verisnot(True/1, True)
vereq(False/1, 0)
verisnot(False/1, False)

for b in False, True:
    for i in 0, 1, 2:
        vereq(b**i, int(b)**i)
        verisnot(b**i, bool(int(b)**i))

for a in False, True:
    for b in False, True:
        veris(a&b, bool(int(a)&int(b)))
        veris(a|b, bool(int(a)|int(b)))
        veris(a^b, bool(int(a)^int(b)))
        vereq(a&int(b), int(a)&int(b))
        verisnot(a&int(b), bool(int(a)&int(b)))
        vereq(a|int(b), int(a)|int(b))
        verisnot(a|int(b), bool(int(a)|int(b)))
        vereq(a^int(b), int(a)^int(b))
        verisnot(a^int(b), bool(int(a)^int(b)))
        vereq(int(a)&b, int(a)&int(b))
        verisnot(int(a)&b, bool(int(a)&int(b)))
        vereq(int(a)|b, int(a)|int(b))
        verisnot(int(a)|b, bool(int(a)|int(b)))
        vereq(int(a)^b, int(a)^int(b))
        verisnot(int(a)^b, bool(int(a)^int(b)))

veris(1==1, True)
veris(1==0, False)
# XXX <, <=, >, >=, !=

x = [1]
veris(x is x, True)
veris(x is not x, False)

veris(1 in x, True)
veris(0 in x, False)
veris(1 not in x, False)
veris(0 not in x, True)

veris(not True, False)
veris(not False, True)

veris(bool(10), True)
veris(bool(1), True)
veris(bool(-1), True)
veris(bool(0), False)
veris(bool("hello"), True)
veris(bool(""), False)

veris(hasattr([], "append"), True)
veris(hasattr([], "wobble"), False)

veris(callable(len), True)
veris(callable(1), False)

veris(isinstance(True, bool), True)
veris(isinstance(False, bool), True)
veris(isinstance(True, int), True)
veris(isinstance(False, int), True)
veris(isinstance(1, bool), False)
veris(isinstance(0, bool), False)

veris(issubclass(bool, int), True)
veris(issubclass(int, bool), False)

veris({}.has_key(1), False)
veris({1:1}.has_key(1), True)

veris("xyz".endswith("z"), True)
veris("xyz".endswith("x"), False)
veris("xyz0123".isalnum(), True)
veris("@#$%".isalnum(), False)
veris("xyz".isalpha(), True)
veris("@#$%".isalpha(), False)
veris("0123".isdigit(), True)
veris("xyz".isdigit(), False)
veris("xyz".islower(), True)
veris("XYZ".islower(), False)
veris(" ".isspace(), True)
veris("XYZ".isspace(), False)
veris("X".istitle(), True)
veris("x".istitle(), False)
veris("XYZ".isupper(), True)
veris("xyz".isupper(), False)
veris("xyz".startswith("x"), True)
veris("xyz".startswith("z"), False)

veris(u"xyz".endswith(u"z"), True)
veris(u"xyz".endswith(u"x"), False)
veris(u"xyz0123".isalnum(), True)
veris(u"@#$%".isalnum(), False)
veris(u"xyz".isalpha(), True)
veris(u"@#$%".isalpha(), False)
veris(u"0123".isdecimal(), True)
veris(u"xyz".isdecimal(), False)
veris(u"0123".isdigit(), True)
veris(u"xyz".isdigit(), False)
veris(u"xyz".islower(), True)
veris(u"XYZ".islower(), False)
veris(u"0123".isnumeric(), True)
veris(u"xyz".isnumeric(), False)
veris(u" ".isspace(), True)
veris(u"XYZ".isspace(), False)
veris(u"X".istitle(), True)
veris(u"x".istitle(), False)
veris(u"XYZ".isupper(), True)
veris(u"xyz".isupper(), False)
veris(u"xyz".startswith(u"x"), True)
veris(u"xyz".startswith(u"z"), False)

f = file(TESTFN, "w")
veris(f.closed, False)
f.close()
veris(f.closed, True)
import os
os.remove(TESTFN)

import operator
veris(operator.truth(0), False)
veris(operator.truth(1), True)
veris(operator.isCallable(0), False)
veris(operator.isCallable(len), True)
veris(operator.isNumberType(None), False)
veris(operator.isNumberType(0), True)
veris(operator.not_(1), False)
veris(operator.not_(0), True)
veris(operator.isSequenceType(0), False)
veris(operator.isSequenceType([]), True)
veris(operator.contains([], 1), False)
veris(operator.contains([1], 1), True)
veris(operator.isMappingType([]), False)
veris(operator.isMappingType({}), True)
veris(operator.lt(0, 0), False)
veris(operator.lt(0, 1), True)

import marshal
veris(marshal.loads(marshal.dumps(True)), True)
veris(marshal.loads(marshal.dumps(False)), False)

import pickle
veris(pickle.loads(pickle.dumps(True)), True)
veris(pickle.loads(pickle.dumps(False)), False)

import cPickle
veris(cPickle.loads(cPickle.dumps(True)), True)
veris(cPickle.loads(cPickle.dumps(False)), False)

veris(pickle.loads(cPickle.dumps(True)), True)
veris(pickle.loads(cPickle.dumps(False)), False)

veris(cPickle.loads(pickle.dumps(True)), True)
veris(cPickle.loads(pickle.dumps(False)), False)

if verbose:
    print "All OK"
