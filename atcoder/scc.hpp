#ifndef ACL_PYTHON_SCC
#define ACL_PYTHON_SCC



#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <algorithm>
#include <vector>

#include "atcoder/utils"
#include "atcoder/internal_scc"

namespace atcoder_python {


/* scc_graph object ***********************************/

/* It calculates the strongly connected components
   of directed graphs.


reference: https://github.com/atcoder/ac-library/blob/master/atcoder/scc.hpp
*/

struct SCCGraphObject
{
    PyObject_HEAD
    internal::scc_graph internal;
};


static PyObject *
scc_graph_repr(SCCGraphObject *self) {
    PyObject *name = PyUnicode_FromString("SCCGraph");
    return name;
}


PyDoc_STRVAR(scc_graph_doc,
"It calculates the strongly connected components of directed graphs.\n\n"
"SCCGrarh(n)  (Constructor)\n"
"    Parameters\n"
"    ----------\n"
"    n : int\n"
"        number of vertices\n"
"    \n"
"    Returns\n"
"    -------\n"
"    scc_graph : SCCGraph\n"
"        SCCGraph object with n vertices\n"
"    \n"
"    Constraints\n"
"    -----------\n"
"    0 <= n <= 10^7\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(1)"
);




static void
scc_graph_add_edge_impl(SCCGraphObject *self, int from, int to) {
    self->internal.add_edge(from, to);
}


static PyObject *
scc_graph_add_edge(SCCGraphObject *self, PyObject *const *args, Py_ssize_t nargs) {
    long from, to;
    if (nargs != 2) NARGS_VIOLATION("SCCGraph.add_edge", 2);
    int n = self->internal.num_vertices();

    from = PyLong_AsLong(args[0]);
    CHECK_CONVERT(from);
    CHECK_INDEX_RANGE(from, n);

    to = PyLong_AsLong(args[1]);
    CHECK_CONVERT(to);
    CHECK_INDEX_RANGE(to, n);

    scc_graph_add_edge_impl(self, (int)from, (int)to);
    Py_RETURN_NONE;
}


PyDoc_STRVAR(scc_graph_add_edge_doc,
"add_edge(a, b)\n"
"--\n\n"
"It adds a directed edge from the vertex a to the vertex b.\n\n"
"Parameters\n"
"----------\n"
"a : int\n"
"    vertex id\n"
"b : int\n"
"    vertex id\n"
"\n"
"Returns\n"
"-------\n"
"Nothing\n"
"\n"
"Constraints\n"
"-----------\n"
"0 <= a < n\n"
"0 <= b < n\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(1) amortized"
);


#define ACL_PYTHON_SCC_GRAPH_ADD_EDGE_METHODDEF \
    {"add_edge", (PyCFunction)(void(*)(void))scc_graph_add_edge, METH_FASTCALL, scc_graph_add_edge_doc},


static std::vector<std::vector<int>>
scc_graph_scc_impl(SCCGraphObject *self) {
    return self->internal.scc();
}

static PyObject *
scc_graph_scc(SCCGraphObject *self, PyObject *args) {
    std::vector<std::vector<int>> res = scc_graph_scc_impl(self);
    return _PyList_FromVectorVectorInt(res);
}


PyDoc_STRVAR(scc_graph_scc_doc,
"scc()\n"
"--\n\n"
"It returns the list of the \"list of the vertices\" that\n"
"satisfies the following.\n\n"
" * Each vertex is in exactly one \"list of the vertices\".\n"
" * Each \"list of the vertices\" corresponds to the\n"
"   vertex set of a strongly connected component.\n"
"   The order of the vertices in the list is undefined.\n"
" * The list of \"list of the vertices\" are sorted in\n"
"   topological order, i.e., for two vertices u, v in\n"
"   different strongly connected components, if there is\n"
"   a directed path from u to v, the list containing u\n"
"   appears earlier than the list containing v.\n\n"
"Parameters\n"
"----------\n"
"Nothing\n"
"\n"
"Returns\n"
"-------\n"
"scc : list[list[int]]\n"
"    the list of the \"list of the vertices in a connected component\"\n"
"\n"
"Constraints\n"
"-----------\n"
"Nothing\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(n + m), where m is the number of added edges."
);


#define ACL_PYTHON_SCC_GRAPH_SCC_METHODDEF \
    {"scc", (PyCFunction)scc_graph_scc, METH_NOARGS, scc_graph_scc_doc},




static PyMethodDef scc_graph_methods[] = {
    ACL_PYTHON_SCC_GRAPH_ADD_EDGE_METHODDEF
    ACL_PYTHON_SCC_GRAPH_SCC_METHODDEF
    {NULL} /* Sentinel */
};



static int
scc_graph_init(SCCGraphObject *self, PyObject *args, PyObject *kwargs) {
    int n;
    if (!PyArg_ParseTuple(args, "i", &n)) return -1;
    self->internal = internal::scc_graph(n);
    return 0;
}


static PyObject *
scc_graph_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    SCCGraphObject *self;
    self = (SCCGraphObject *)type->tp_alloc(type, 0);
    if (self == NULL) return NULL;
    return (PyObject *)self;
}



PyTypeObject SCCGraphType = {
    .ob_base = {PyObject_HEAD_INIT(NULL) 0},
    .tp_name = "atcoder.SCCGraph",
    .tp_basicsize = sizeof(SCCGraphObject),
    .tp_itemsize = 0,
    .tp_repr = (reprfunc)scc_graph_repr,
    // .tp_getattro = PyObject_GenericGetAttr,
    // .tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = scc_graph_doc,
    .tp_methods = scc_graph_methods,
    .tp_init = (initproc)scc_graph_init,
    .tp_new = scc_graph_new,
    .tp_free = PyObject_Del,
};





}  // namespace atcoder_python


#endif  // ACL_PYTHON_SCC