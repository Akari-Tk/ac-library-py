#ifndef ACL_PYTHON_UTILS
#define ACL_PYTHON_UTILS

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <vector>

namespace atcoder_python {


#define ERROR(x) fprintf(stderr, x);
#define ERRORF(x, y) fprintf(stderr, x, y);

#define NOT_INT_ERROR                                   \
    do {                                               \
        const char *msg = "int only";                  \
        PyErr_SetString(PyExc_TypeError, msg);         \
        return NULL;                                   \
    } while (0)

#define NARGS_VIOLATION(name, n)                                \
    do {                                                       \
        const char *msg = "%s() takes %d argument(s)";         \
        return PyErr_Format(PyExc_TypeError , msg, name, n);   \
    } while (0)


static int
_PyObject_AsPositiveInt(PyObject *o) {
    PyLongObject *pa;
    pa = (PyLongObject *)o;
    Py_ssize_t size_a;
    if (pa == NULL) return -1;
    Py_INCREF(pa);
    size_a = Py_SIZE(pa);
    if (size_a < 0 || size_a > 1) {
        Py_DECREF(pa);
        return -1;
    }
    int res = (int)PyLong_AsLong((PyObject *)pa);
    Py_DECREF(pa);
    return res;
}

static long long
_PyObject_AsPositiveLongLong(PyObject *o) {
    PyLongObject *pa;
    pa = (PyLongObject *)o;
    Py_ssize_t size_a;
    if (pa == NULL) return -1LL;
    Py_INCREF(pa);
    size_a = Py_SIZE(pa);
    if (size_a < 0 || size_a > 2) {
        Py_DECREF(pa);
        return -1LL;
    }
    long long res = PyLong_AsLongLong((PyObject *)pa);
    Py_DECREF(pa);
    return res;
}



#define CANNOT_CONVERT(name, n, type)                                      \
    do {                                                                  \
        const char *msg = "%s().arg[%d] could not be converted to %s";    \
        return PyErr_Format(PyExc_ValueError , msg, name, n, type);       \
    } while (0)

#define CONVERT_LONG_LONG(x, i, overflow, name)                             \
    do {                                                                  \
        overflow = 0;                                                      \
        x = PyLong_AsLongLongAndOverflow(args[i], &overflow);               \
        if (overflow || PyErr_Occurred()) {                                \
            CANNOT_CONVERT(name, i, "long long");                         \
        }                                                                 \
    } while (0)



/* vector -> list */
static PyObject *
_PyList_FromVector(std::vector<int> &v) {
    int n = v.size();
    PyObject *list = PyList_New(n);
    for (int i = 0; i < n; i++) {
        PyList_SetItem(list, i, PyLong_FromLong((long)v[i]));
    }
    return list;
}

/* unused
static PyObject *
_PyList_FromVector(std::vector<long long> &v) {
    int n = v.size();
    PyObject *list = PyList_New(n);
    for (int i = 0; i < n; i++) {
        PyList_SetItem(list, i, PyLong_FromLongLong(v[i]));
    }
    return list;
}
*/



/* vecor<vector<int>> -> list[list] */
static PyObject *
_PyList_FromVectorVectorInt(std::vector<std::vector<int>> &v) {
    int n = v.size();
    PyObject *list = PyList_New(n);
    for (int i = 0; i < n; i++) {
        int m = v[i].size();
        PyObject *li = PyList_New(m);
        for (int j = 0; j < m; j++) {
            PyList_SetItem(li, j, PyLong_FromLong((long)v[i][j]));
        }
        PyList_SetItem(list, i, li);
    }
    return list;
}



/* list[int] -> vector<long long> */
static std::vector<long long>
_PyList_AsVectorLongLong(PyObject *list, int *err) {
    int n = PyList_Size(list);
    int overflow;
    std::vector<long long> res(n);
    for (int i = 0; i < n; i++) {
        PyObject *v = PyList_GetItem(list, i);
        if (!PyLong_Check(v)) {
            PyErr_SetString(PyExc_TypeError, "required : list[int]");
            Py_DECREF(v);
            *err = 1;
            return res;
        }
        long long x = PyLong_AsLongLongAndOverflow(v, &overflow);
        if (overflow) {
            PyErr_SetString(PyExc_OverflowError, "overflow");
            Py_DECREF(v);
            *err = 1;
            return res;
        }
        res[i] = x;
        Py_DECREF(v);
    }
    return res;
}




/* repr function for sequential object
reference : https://github.com/python/cpython/blob/3.11/Objects/listobject.c
            379: list_repr

*/
static PyObject *
sequential_object_repr(PyListObject *v, PyObject *name)
{
    Py_ssize_t i;
    PyObject *s;
    _PyUnicodeWriter writer;

    if (Py_SIZE(v) == 0) {
        return PyUnicode_Concat(name, PyUnicode_FromFormat("([])"));
    }

    i = Py_ReprEnter((PyObject*)v);
    if (i != 0) {
        return i > 0 ? PyUnicode_Concat(name, PyUnicode_FromFormat("([...])")) : NULL;
    }

    _PyUnicodeWriter_Init(&writer);
    writer.overallocate = 1;
    /* name + "(" + "[" + "1" + ", 2" * (len - 1) + "]" + ")" */
    writer.min_length = PyUnicode_GetLength(name) + 1 + 1 + 1 + (2 + 1) * (Py_SIZE(v) - 1) + 1 + 1;

    if (_PyUnicodeWriter_WriteStr(&writer, name) < 0)
        goto error;

    if (_PyUnicodeWriter_WriteASCIIString(&writer, "([", 2) < 0)
        goto error;

    /* Do repr() on each element.  Note that this may mutate the list,
       so must refetch the list size on each iteration. */
    for (i = 0; i < Py_SIZE(v); ++i) {
        if (i > 0) {
            if (_PyUnicodeWriter_WriteASCIIString(&writer, ", ", 2) < 0)
                goto error;
        }

        s = PyObject_Repr(v->ob_item[i]);
        if (s == NULL)
            goto error;

        if (_PyUnicodeWriter_WriteStr(&writer, s) < 0) {
            Py_DECREF(s);
            goto error;
        }
        Py_DECREF(s);
    }

    writer.overallocate = 0;
    if (_PyUnicodeWriter_WriteASCIIString(&writer, "])", 2) < 0)
        goto error;

    Py_ReprLeave((PyObject *)v);
    return _PyUnicodeWriter_Finish(&writer);

error:
    _PyUnicodeWriter_Dealloc(&writer);
    Py_ReprLeave((PyObject *)v);
    return NULL;
}



} // namespace atcoder_python


#endif  // ACL_PYTHON_UTILS