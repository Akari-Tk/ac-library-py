#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>


#include "atcoder/dsu"
#include "atcoder/fenwicktree"
#include "atcoder/modint"
#include "atcoder/math"
#include "atcoder/bitset"
#include "atcoder/segtree"
#include "atcoder/lazysegtree"
#include "atcoder/scc"




namespace atcoder_python {

PyDoc_STRVAR(atcoder_doc,
"AtCoder Library for Python\n\
");




static struct PyModuleDef atcodermodule = {
    PyModuleDef_HEAD_INIT,
    "atcoder",
    atcoder_doc,
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


PyMODINIT_FUNC
PyInit_atcoder(void)
{
    int i;
    PyObject *m;
    const char *name;
    PyTypeObject *typelist[] = {
        &DsuType,
        &FenwickTreeType,
        &ModIntType,
        &BitSetType,
        &SegTreeType,
        &LazySegTreeType,
        &SCCGraphType,
        NULL
    };

    m = PyModule_Create(&atcodermodule);
    if (m == NULL)
        return NULL;

    for (i=0 ; typelist[i] != NULL ; i++) {
        if (PyType_Ready(typelist[i]) < 0)
            return NULL;
        name = _PyType_Name(typelist[i]);
        Py_INCREF(typelist[i]);
        PyModule_AddObject(m, name, (PyObject *)typelist[i]);
    }
    PyModule_AddFunctions(m, mathfunctions);
    return m;
}

} // namespace atcoder_python