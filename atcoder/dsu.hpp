#ifndef ACL_PYTHON_DSU
#define ACL_PYTHON_DSU



#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <algorithm>
#include <utility>
#include <vector>


#include "atcoder/utils"

namespace atcoder_python {

/* dsu object *****************************************/

/* Data structures and algorithms for disjoint set union problems.
reference: https://github.com/atcoder/ac-library/blob/master/atcoder/dsu.hpp

    >>> d = DSU(5)  # construct dsu object with 5 vertices

        0       1

            2

        3       4

    >>> d.merge(0, 1)
    True
    >>> d.merge(1, 2)
    True

        0 ----- 1
              /
            2

        3       4

    >>> d.merge(0, 2)
    False           # 0 and 2 are already in the same group

    >>> d.same(0, 2)
    True
    >>> d.same(0, 3)
    False

    >>> d.size(1)
    3
    >>> d.size(3)
    1

    >>> d.groups()
    [[0, 1, 2], [3], [4]]


*/



struct DsuObject
{
    PyObject_HEAD
    int _n;
    std::vector<int> parent_or_size;
};


static PyObject *
dsu_repr(DsuObject *self) {
    PyObject *list = _PyList_FromVector(self->parent_or_size);
    PyObject *name = PyUnicode_FromString("DSU");
    return sequential_object_repr((PyListObject *)list, name);
}


PyDoc_STRVAR(dsu_doc,
"Data structures and algorithms for disjoint set union problems\n\n"
"DSU(n)  (Constructor)\n"
"    Parameters\n"
"    ----------\n"
"    n : int\n"
"        number of vertices\n"
"    \n"
"    Returns\n"
"    -------\n"
"    dsu : DSU\n"
"        DSU object with n vertices\n"
"    \n"
"    Constraints\n"
"    -----------\n"
"    0 <= n <= 10^7\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(n)"
);


static Py_ssize_t
dsu_length(DsuObject *self) {
    return (Py_ssize_t)self->_n;
}

static PySequenceMethods dsu_as_sequence = {
    .sq_length = (lenfunc)dsu_length,
};


static int
_dsu_leader(DsuObject *self, int a) {
    if (self->parent_or_size[a] < 0) return a;
    return self->parent_or_size[a] = _dsu_leader(self, self->parent_or_size[a]);
}

static PyObject *
dsu_leader(DsuObject *self, PyObject *arg) {
    int a;
    a = _PyObject_AsPositiveInt(arg);
    if (a == -1) CANNOT_CONVERT("DSU.leader", 0, "non-negative int");
    // assert(0 <= a && a < self->_n);
    if (0 > a || a >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", a);
        return NULL;
    }
    return PyLong_FromLong((long)_dsu_leader(self, a));
}


PyDoc_STRVAR(dsu_leader_doc,
u8"leader(a)\n"
"--\n\n"
"Get the representative of the connected component\n"
"that contains the vertex a.\n\n"
"Parameters\n"
"----------\n"
"a : int\n"
"    vertex id\n"
"\n"
"Returns\n"
"-------\n"
"x : int\n"
"    the representative vertex id of the connected\n"
"    component that contains the vertex a\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= a < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(\u03B1(n)) amortized"
);

#define ACL_PYTHON_DSU_LEADER_METHODDEF \
    {"leader", (PyCFunction)(void(*)(void))dsu_leader, METH_O, dsu_leader_doc},



static bool
_dsu_merge(DsuObject *self, int a, int b) {
    a = _dsu_leader(self, a);
    b = _dsu_leader(self, b);
    if (a == b) return false;
    if (-self->parent_or_size[a] < -self->parent_or_size[b]) std::swap(a, b);
    self->parent_or_size[a] += self->parent_or_size[b];
    self->parent_or_size[b] = a;
    return true;
}

static PyObject *
dsu_merge(DsuObject *self, PyObject *const *args, Py_ssize_t nargs) {
    int a, b;
    if (nargs != 2) NARGS_VIOLATION("DSU.merge", 2);
    a = _PyObject_AsPositiveInt(args[0]);
    b = _PyObject_AsPositiveInt(args[1]);
    if (a == -1) CANNOT_CONVERT("DSU.merge", 0, "non-negative int");
    if (b == -1) CANNOT_CONVERT("DSU.merge", 1, "non-negative int");
    // assert(0 <= a && a < self->_n);
    if (0 > a || a >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", a);
        return NULL;
    }
    // assert(0 <= b && b < self->_n);
    if (0 > b || b >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", b);
        return NULL;
    }
    if (_dsu_merge(self, a, b)) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

PyDoc_STRVAR(dsu_merge_doc,
"merge(a, b)\n"
"--\n\n"
"It adds an edge (a, b).\n\n"
"If the vertices a and b were in the same connected component,\n"
"it returns False. Otherwise, it returns True.\n"
"(Different from original ACL)\n\n"
"Parameters\n"
"----------\n"
"a : int\n"
"    vertex id\n"
"b : int\n"
"    vertex id\n"
"\n"
"Returns\n"
"-------\n"
"merged : bool\n"
"    Whether a merging process has taken place\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= a < n\n"
"0 <= b < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(\u03B1(n)) amortized"
);

#define ACL_PYTHON_DSU_MERGE_METHODDEF \
    {"merge", (PyCFunction)(void(*)(void))dsu_merge, METH_FASTCALL, dsu_merge_doc},


static PyObject *
dsu_same(DsuObject *self, PyObject *const *args, Py_ssize_t nargs) {
    int a, b;
    if (nargs != 2) NARGS_VIOLATION("DSU.same", 2);
    a = _PyObject_AsPositiveInt(args[0]);
    b = _PyObject_AsPositiveInt(args[1]);
    if (a == -1) CANNOT_CONVERT("DSU.same", 0, "non-negative int");
    if (b == -1) CANNOT_CONVERT("DSU.same", 1, "non-negative int");
    // assert(0 <= a && a < self->_n);
    if (0 > a || a >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", a);
        return NULL;
    }
    // assert(0 <= b && b < self->_n);
    if (0 > b || b >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", b);
        return NULL;
    }
    a = _dsu_leader(self, a);
    b = _dsu_leader(self, b);
    if (a == b) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

PyDoc_STRVAR(dsu_same_doc,
"same(a, b)\n"
"--\n\n"
"It returns whether the vertices a and b are in the\n"
"same connected component.\n\n"
"Parameters\n"
"----------\n"
"a : int\n"
"    vertex id\n"
"b : int\n"
"    vertex id\n"
"\n"
"Returns\n"
"-------\n"
"same : bool\n"
"    Whether the vertices a and b are in the same connected component\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= a < n\n"
"0 <= b < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(\u03B1(n)) amortized"
);


#define ACL_PYTHON_DSU_SAME_METHODDEF \
    {"same", (PyCFunction)(void(*)(void))dsu_same, METH_FASTCALL, dsu_same_doc},


static PyObject *
dsu_size(DsuObject *self, PyObject *arg) {
    int a;
    a = _PyObject_AsPositiveInt(arg);
    if (a == -1) CANNOT_CONVERT("DSU.size", 0, "non-negative int");
    // assert(0 <= a && a < self->_n);
    if (0 > a || a >= self->_n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", a);
        return NULL;
    }
    a = _dsu_leader(self, a);
    return PyLong_FromLong((long)(-self->parent_or_size[a]));
}

PyDoc_STRVAR(dsu_size_doc,
"size(a)\n"
"--\n\n"
"It returns the size of the connected component that contains\n"
"the vertex a.\n\n"
"Parameters\n"
"----------\n"
"a : int\n"
"    vertex id\n"
"\n"
"Returns\n"
"-------\n"
"size : int\n"
"    the size of the connected component that contains the vertex a\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= a < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(\u03B1(n)) amortized"
);

#define ACL_PYTHON_DSU_SIZE_METHODDEF \
    {"size", (PyCFunction)(void(*)(void))dsu_size, METH_O, dsu_size_doc},


static std::vector<std::vector<int>>
_dsu_groups(DsuObject *self) {
    int n = self->_n;
    std::vector<int> leader_buf(n), group_size(n);
    for (int i = 0; i < n; i++) {
        leader_buf[i] = _dsu_leader(self, i);
        group_size[leader_buf[i]]++;
    }
    std::vector<std::vector<int>> result(n);
    for (int i = 0; i < n; i++) {
        result[i].reserve(group_size[i]);
    }
    for (int i = 0; i < n; i++) {
        result[leader_buf[i]].push_back(i);
    }
    result.erase(
        std::remove_if(result.begin(), result.end(),
                        [&](const std::vector<int>& v) { return v.empty(); }),
        result.end());
    return result;
}

static PyObject *
dsu_groups(DsuObject *self, PyObject *args) {
    std::vector<std::vector<int>> res = _dsu_groups(self);
    return _PyList_FromVectorVectorInt(res);
}

PyDoc_STRVAR(dsu_groups_doc,
"groups()\n"
"--\n\n"
"It divides the graph into connected components and\n"
"returns the list of them.\n\n"
"More precisely, it returns the list of the \"list of the vertices\n"
"in a connected component\".\n"
"Both of the orders of the connected components and\n"
"the vertices are undefined.\n\n"
"Parameters\n"
"----------\n"
"Nothing\n"
"\n"
"Returns\n"
"-------\n"
"groups : list[list[int]]\n"
"    the list of the \"list of the vertices in a connected component\"\n"
"\n"
"Constraints\n"
"-----------\n"
"Nothing\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(n)"
);

#define ACL_PYTHON_DSU_GROUPS_METHODDEF \
    {"groups", (PyCFunction)dsu_groups, METH_NOARGS, dsu_groups_doc},





static PyMethodDef dsu_methods[] = {
    ACL_PYTHON_DSU_LEADER_METHODDEF
    ACL_PYTHON_DSU_MERGE_METHODDEF
    ACL_PYTHON_DSU_SAME_METHODDEF
    ACL_PYTHON_DSU_SIZE_METHODDEF
    ACL_PYTHON_DSU_GROUPS_METHODDEF
    {NULL} /* Sentinel */
};

// static PyMemberDef dsu_members[] = {
//     {"_n", T_INT, offsetof(DsuObject, _n), READONLY},
//     {NULL} /* Sentinel */
// };

// static int
// dsu_init(DsuObject *self, PyObject *args, PyObject *kwargs)
// {
//     int n;
//     if (!PyArg_ParseTuple(args, "i", &n)) return -1;
//     self->_n = n;
//     self->parent_or_size.assign(n, -1);
//     return 0;
// }


static PyObject *
dsu_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    DsuObject *self;
    self = (DsuObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        int n;
        if (!PyArg_ParseTuple(args, "i", &n)) return NULL;
        self->_n = n;
        self->parent_or_size = std::vector<int>(n, -1);
    }
    return (PyObject *)self;
}


PyTypeObject DsuType = {
    .ob_base = {PyObject_HEAD_INIT(NULL) 0},
    .tp_name = "atcoder.DSU",
    .tp_basicsize = sizeof(DsuObject),
    .tp_itemsize = sizeof(int),
    .tp_repr = (reprfunc)dsu_repr,
    .tp_as_sequence = &dsu_as_sequence,
    // .tp_getattro = PyObject_GenericGetAttr,
    // .tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = dsu_doc,
    .tp_methods = dsu_methods,
    // .tp_members = dsu_members,
    // .tp_init = (initproc)dsu_init,
    .tp_new = dsu_new,
    .tp_free = PyObject_Del,
};

} // namespace atcoder_python

#endif  // ACL_PYTHON_DSU