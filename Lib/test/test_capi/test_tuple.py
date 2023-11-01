import unittest
import sys
from collections import namedtuple
from test.support import import_helper
_testcapi = import_helper.import_module('_testcapi')

NULL = None
PY_SSIZE_T_MIN = _testcapi.PY_SSIZE_T_MIN
PY_SSIZE_T_MAX = _testcapi.PY_SSIZE_T_MAX

class TupleSubclass(tuple):
    pass

class CAPITest(unittest.TestCase):
    def test_check(self):
        # Test PyTuple_Check()
        check = _testcapi.tuple_check
        self.assertTrue(check((1, 2)))
        self.assertTrue(check(()))
        self.assertFalse(check({1: 2}))
        self.assertFalse(check([1, 2]))
        self.assertFalse(check(42))
        self.assertFalse(check(object()))

        # CRASHES check(NULL)


    def test_tuple_checkexact(self):
        # Test PyTuple_CheckExact()
        check = _testcapi.tuple_checkexact
        self.assertTrue(check((1, 2)))
        self.assertTrue(check(()))
        self.assertFalse(check(TupleSubclass((1, 2))))
        self.assertFalse(check({1: 2}))
        self.assertFalse(check(object()))

        # CRASHES check(NULL)

    # def test_tuple_new(self):
    #     # Test PyTuple_New()
    #     list_new = _testcapi.list_new
    #     lst = list_new(0)
    #     self.assertEqual(lst, [])
    #     self.assertIs(type(lst), list)
    #     lst2 = list_new(0)
    #     self.assertIsNot(lst2, lst)
    #     self.assertRaises(SystemError, list_new, NULL)
    #     self.assertRaises(SystemError, list_new, -1)

    # def test_tuple_size(self):
    #     # Test PyTuple_Size()
    #     size = _testcapi.list_size
    #     self.assertEqual(size([1, 2]), 2)
    #     self.assertEqual(size(ListSubclass([1, 2])), 2)
    #     self.assertRaises(SystemError, size, UserList())
    #     self.assertRaises(SystemError, size, {})
    #     self.assertRaises(SystemError, size, 23)
    #     self.assertRaises(SystemError, size, object())
    #     # CRASHES size(NULL)

    # def test_tuple_get_size(self):
    #     # Test PyTuple_GET_SIZE()
    #     size = _testcapi.list_get_size
    #     self.assertEqual(size([1, 2]), 2)
    #     self.assertEqual(size(ListSubclass([1, 2])), 2)
    #     # CRASHES size(object())
    #     # CRASHES size(23)
    #     # CRASHES size({})
    #     # CRASHES size(UserList())
    #     # CRASHES size(NULL)


    # def test_tuple_getitem(self):
    #     # Test PyTuple_GetItem()
    #     getitem = _testcapi.list_getitem
    #     lst = [1, 2, 3]
    #     self.assertEqual(getitem(lst, 0), 1)
    #     self.assertEqual(getitem(lst, len(lst)-1), 3)
    #     self.assertRaises(IndexError, getitem, lst, -1)
    #     self.assertRaises(IndexError, getitem, lst, PY_SSIZE_T_MIN)
    #     self.assertRaises(IndexError, getitem, lst, PY_SSIZE_T_MAX)
    #     self.assertRaises(IndexError, getitem, lst, len(lst))
    #     self.assertRaises(SystemError, getitem, 42, 1)

    #     # CRASHES getitem(NULL, 1)

    # def test_tuple_get_item(self):
    #     # Test PyTuple_GET_ITEM()
    #     get_item = _testcapi.list_get_item
    #     lst = [1, 2, [1, 2, 3]]
    #     self.assertEqual(get_item(lst, 0), 1)
    #     self.assertEqual(get_item(lst, 2), [1, 2, 3])

    #     # CRASHES for get_item(lst, PY_SSIZE_T_MIN)
    #     # CRASHES for get_item(lst, PY_SSIZE_T_MAX)
    #     # CRASHES for out of index: get_item(lst, 3)
    #     # CRASHES get_item(21, 2)
    #     # CRASHES get_item(Null,1)


    # def test_tuple_setitem(self):
    #     # Test PyTuple_SetItem()
    #     setitem = _testcapi.list_setitem
    #     lst = [1, 2, 3]
    #     setitem(lst, 0, 10)
    #     self.assertEqual(lst[0], 10)
    #     setitem(lst, len(lst)-1, 12)
    #     self.assertEqual(lst[-1], 12)
    #     self.assertRaises(IndexError, setitem, lst, PY_SSIZE_T_MIN, 5)
    #     self.assertRaises(IndexError, setitem, lst, PY_SSIZE_T_MAX, 5)
    #     self.assertRaises(IndexError, setitem, lst, -1, 5)
    #     self.assertRaises(IndexError, setitem, lst, len(lst) , 5)
    #     self.assertRaises(TypeError, setitem, lst, 1.5, 10)
    #     self.assertRaises(TypeError, setitem, 23, 'a', 5)
    #     self.assertRaises(SystemError, setitem, {}, 0, 5)

    #     # CRASHES setitem(NULL, 'a', 5)

    # def test_tuple_set_item(self):
    #     # Test PyTuple_SET_ITEM()
    #     set_item = _testcapi.list_set_item
    #     lst = [1, 2, 3]
    #     set_item(lst, 1, 10)
    #     set_item(lst, 2, [1, 2, 3])
    #     self.assertEqual(lst[2], [1, 2, 3])

    #     # CRASHES for set_item([1], PY_SSIZE_T_MIN, 5)
    #     # CRASHES for set_item([1], PY_SSIZE_T_MAX, 5)
    #     # CRASHES for set_item([1], -1, 5)
    #     # CRASHES for set_item([], 0, 1)
    #     # CRASHES for set_item(NULL, 0, 1)


    # def test_tuple_insert(self):
    #     # Test PyTuple_Insert()
    #     insert = _testcapi.list_insert
    #     lst = [1, 2, 3]
    #     insert(lst, 0, 23)
    #     self.assertEqual(lst[0], 23)
    #     insert(lst, -1, 22)
    #     self.assertEqual(lst[-2], 22)
    #     insert(lst, PY_SSIZE_T_MIN, 1)
    #     self.assertEqual(lst[0], 1)
    #     insert(lst, len(lst), 123)
    #     self.assertEqual(lst[-1], 123)
    #     insert(lst, len(lst)-1, 124)
    #     self.assertEqual(lst[-2], 124)
    #     insert(lst, PY_SSIZE_T_MAX, 223)
    #     self.assertEqual(lst[-1], 223)

    #     self.assertRaises(TypeError, insert, lst, 1.5, 10)
    #     self.assertRaises(TypeError, insert, 23, 'a', 5)
    #     self.assertRaises(SystemError, insert, {}, 0, 5)

    #     # CRASHES insert(NULL, 'a', 5)

    # def test_tuple_append(self):
    #     # Test PyTuple_Append()
    #     append = _testcapi.list_append
    #     lst = [1, 2, 3]
    #     append(lst, 1)
    #     self.assertEqual(lst[-1], 1)
    #     append(lst, [4,5,6])
    #     self.assertEqual(lst[-1], [4,5,6])
    #     self.assertRaises(SystemError, append, lst, NULL)
    #     # CRASHES append(NULL, 0)

    # def test_tuple_getslice(self):
    #     # Test PyTuple_GetSlice()
    #     getslice = _testcapi.list_getslice
    #     lst = [1,2,3]

    #     # empty
    #     self.assertEqual(getslice(lst, PY_SSIZE_T_MIN, 0), [])
    #     self.assertEqual(getslice(lst, -1, 0), [])
    #     self.assertEqual(getslice(lst, 3, PY_SSIZE_T_MAX), [])

    #     # slice
    #     self.assertEqual(getslice(lst, 1, 3), [2, 3])

    #     # whole
    #     self.assertEqual(getslice(lst, 0, len(lst)), lst)
    #     self.assertEqual(getslice(lst, 0, 100), lst)
    #     self.assertEqual(getslice(lst, -100, 100), lst)

    #     self.assertRaises(TypeError, lst, 'a', '2')

    #     # CRASHES getslice(NULL, 0, 0)

    # def test_tuple_setslice(self):
    #     # Test PyTuple_SetSlice()
    #     def set_slice(lst, low, high, value):
    #         lst = lst.copy()
    #         self.assertEqual(_testcapi.list_setslice(lst, low, high, value), 0)
    #         return lst

    #     # insert items
    #     self.assertEqual(set_slice([], 0, 0, list("abc")), list("abc"))
    #     self.assertEqual(set_slice([], PY_SSIZE_T_MIN, PY_SSIZE_T_MIN, list("abc")), list("abc"))
    #     self.assertEqual(set_slice([], PY_SSIZE_T_MAX, PY_SSIZE_T_MAX, list("abc")), list("abc"))
    #     lst = list("abc")
    #     self.assertEqual(set_slice(lst, 0, 0, ["X"]), list("Xabc"))
    #     self.assertEqual(set_slice(lst, 1, 1, list("XY")), list("aXYbc"))
    #     self.assertEqual(set_slice(lst, len(lst), len(lst), ["X"]), list("abcX"))
    #     # self.assertEqual(set_slice(lst, PY_SSIZE_T_MAX, PY_SSIZE_T_MAX, ["X"]), list("abcX"))

    #     # replace items
    #     lst = list("abc")
    #     self.assertEqual(set_slice(lst, -100, 1, list("X")), list("Xbc"))
    #     self.assertEqual(set_slice(lst, 1, 2, list("X")), list("aXc"))
    #     self.assertEqual(set_slice(lst, 1, 3, list("XY")), list("aXY"))
    #     self.assertEqual(set_slice(lst, 0, 3, list("XYZ")), list("XYZ"))

    #     # delete items
    #     lst = list("abcdef")
    #     self.assertEqual(set_slice(lst, 0, len(lst), []), [])
    #     self.assertEqual(set_slice(lst, -100, 100, []), [])
    #     self.assertEqual(set_slice(lst, 1, 5, []), list("af"))
    #     self.assertEqual(set_slice(lst, 3, len(lst), []), list("abc"))

    #     # delete items with NULL
    #     lst = list("abcdef")
    #     self.assertEqual(set_slice(lst, 0, len(lst), NULL), [])
    #     self.assertEqual(set_slice(lst, 3, len(lst), NULL), list("abc"))

    #     # CRASHES PyTuple_SetSlice(NULL, 0, 0, ["x"])

    # def test_tuple_sort(self):
    #     # Test PyTuple_Sort()
    #     sort = _testcapi.list_sort
    #     lst = [4, 6, 7, 3, 1, 5, 9, 2, 0, 8]
    #     sort(lst)
    #     self.assertEqual(lst, list(range(10)))

    #     lst2 = ListSubclass([4, 6, 7, 3, 1, 5, 9, 2, 0, 8])
    #     sort(lst2)
    #     self.assertEqual(lst2, list(range(10)))

    #     self.assertRaises(SystemError, sort, object())
    #     self.assertRaises(SystemError, sort, NULL)


    # def test_tuple_reverse(self):
    #     # Test PyTuple_Reverse()
    #     reverse = _testcapi.list_reverse
    #     def list_reverse(lst):
    #         self.assertEqual(reverse(lst), 0)
    #         return lst

    #     self.assertEqual(list_reverse([]), [])
    #     self.assertEqual(list_reverse([2, 5, 10]), [10, 5, 2])

    #     self.assertRaises(SystemError, reverse, object())
    #     self.assertRaises(SystemError, reverse, NULL)

    # def test_tuple_astuple(self):
    #     # Test PyTuple_AsTuple()
    #     astuple = _testcapi.list_astuple
    #     self.assertEqual(astuple([]), ())
    #     self.assertEqual(astuple([2, 5, 10]), (2, 5, 10))

    #     self.assertRaises(SystemError, astuple, object())
    #     self.assertRaises(SystemError, astuple, NULL)
