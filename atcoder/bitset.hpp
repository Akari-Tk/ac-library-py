#ifndef ACL_PYTHON_BITSET
#define ACL_PYTHON_BITSET



#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <algorithm>
#include <utility>
#include <vector>
#include <string>
#include <bitset>


#include "atcoder/utils"

namespace atcoder_python {

#define BITSET_W 64
#define BITSET_W_SHIFT 6
#define BITSET_ONE 1ULL
#define BITSET_ALL 18446744073709551615ULL // -1 


/* bitset object ***********************************/


extern PyTypeObject BitSetType;


#define BitSet_Check(v) PyObject_TypeCheck(v, &BitSetType)
#define BitSet_Length(v) (v->n)

#define CHECK_BINOP_BITSET(v, w)                           \
    do {                                                  \
        if (!BitSet_Check(v) || !BitSet_Check(w)){        \
            Py_RETURN_NOTIMPLEMENTED;                     \
        }                                                 \
        if (BitSet_Length(v) != BitSet_Length(w)){        \
            Py_RETURN_NOTIMPLEMENTED;                     \
        }                                                 \
    } while (0)

#define CHECK_BINOP_BITSET_TYPE(v, w)                      \
    do {                                                  \
        if (!BitSet_Check(v) || !BitSet_Check(w)){        \
            Py_RETURN_NOTIMPLEMENTED;                     \
        }                                                 \
    } while (0)

#define CHECK_BINOP_BITSET2(v, w)                          \
    do {                                                  \
        if (!BitSet_Check(v) || !PyLong_Check(w)){        \
            Py_RETURN_NOTIMPLEMENTED;                     \
        }                                                 \
    } while (0)


struct BitSetObject {
    PyObject_HEAD
    int n;
    int m;
    int q;
    int r;
    std::vector<unsigned long long> bs;
    static inline std::vector<unsigned long long> onehot;
    static inline std::vector<int> shift;
    static inline std::vector<unsigned long long> upper;
    static inline std::vector<unsigned long long> lower;
    static inline bool initialized;
    std::vector<int> major;
    std::vector<int> minor;
};

static BitSetObject *
bitset_copy(BitSetObject *self) {
    PyTypeObject *type = &BitSetType;
    BitSetObject *z = (BitSetObject *)type->tp_alloc(type, 0);
    z->n = self->n;
    z->m = self->m;
    z->q = self->q;
    z->r = self->r;
    z->bs.reserve(z->m);
    std::copy(self->bs.begin(), self->bs.end(), std::back_inserter(z->bs));
    z->major.reserve(z->n);
    std::copy(self->major.begin(), self->major.end(), std::back_inserter(z->major));
    z->minor.reserve(z->n);
    std::copy(self->minor.begin(), self->minor.end(), std::back_inserter(z->minor));
    return z;
}


static void
_bitset_get_index(BitSetObject *self, int idx, int *i, int *j) {
    if (self->major[idx] == -1) {
        self->major[idx] = (self->n - 1 - idx) / BITSET_W;
        self->minor[idx] = (self->n - 1 - idx) % BITSET_W;
    }
    *i = self->major[idx];
    *j = self->minor[idx];
}




static PyObject *
_bitset_tostring(BitSetObject *self) {
    PyObject *unicode;

    int n = self->n;
    unicode = PyUnicode_New(n, 127);
    for (int idx = 0; idx < n; ++idx) {
        int i, j;
        _bitset_get_index(self, idx, &i, &j);
        unsigned long long v = (self->bs[i] >> BitSetObject::shift[j]) & 1;
        Py_UCS4 c = (Py_UCS4)(v == 1ULL ? 49 : 48);
        if (PyUnicode_WriteChar(unicode, n - 1 - idx, c) == -1){
            return NULL;
        }
    }
    return unicode;
}

static PyObject *
bitset_repr(PyObject *self) {
    return _bitset_tostring((BitSetObject *)self);
}

static Py_ssize_t
bitset_length(BitSetObject *self) {
    return (Py_ssize_t)self->n;
}

static PyObject *
bitset_item(BitSetObject *self, Py_ssize_t idx) {
    if (0 > idx || idx >= self->n){
        PyErr_Format(PyExc_IndexError, "index %d is out of range", idx);
        return NULL;
    }
    int i, j;
    _bitset_get_index(self, idx, &i, &j);
    int x = self->bs[i] >> self->shift[j] & 1;
    return PyLong_FromLong((long)x);
}

static int
bitset_ass_item(BitSetObject *self, Py_ssize_t idx, PyObject *value) {
    int v = PyLong_AsLong(value);
    if (v != 0 && v != 1) {
        PyErr_Format(PyExc_ValueError, "assigned value must be 0 or 1 (not %d)", v);
        return -1;
    }
    int i, j;
    _bitset_get_index(self, idx, &i, &j);
    int x = self->bs[i] >> self->shift[j] & 1;
    if (x != v) self->bs[i] ^= self->onehot[j];
    return 0;
}

static PySequenceMethods bitset_as_sequence = {
    (lenfunc)bitset_length,                      /*sq_length*/
    0,                                           /*sq_concat*/
    0,                                           /*sq_repeat*/
    (ssizeargfunc)bitset_item,                   /*sq_item*/
    0,                                           /*sq_slice*/
    (ssizeobjargproc)bitset_ass_item,            /*sq_ass_item*/
    0,                                           /*sq_ass_slicce*/
    0,                                           /*sq_contains*/
    0,                                           /*sq_inplace_concat*/
    0,                                           /*sq_inplace_repeat*/
};


static void
_bitset_invert(BitSetObject *self) {
    for (int i = 0; i < self->m; ++i) {
        self->bs[i] ^= BITSET_ALL;
    }
}


static PyObject *
bitset_invert(BitSetObject *a) {
    BitSetObject *x = bitset_copy(a);
    _bitset_invert(x);
    return (PyObject *)x;
}


static void
_bitset_lshift(BitSetObject *self, int other) {
    if (other >= self->n) {
        std::fill(self->bs.begin(), self->bs.end(), 0);
        return;
    }
    int rot, sft;
    rot = other >> BITSET_W_SHIFT;
    sft = other - (rot << BITSET_W_SHIFT);
    int m = self->m;
    if (rot) {
        for (int i = 0; i < m-rot; ++i) {
            self->bs[i] = self->bs[i+rot];
        }
        std::fill(self->bs.begin()+(m-rot), self->bs.end(), 0);
    }
    m -= rot;
    if (sft) {
        for (int i = 0; i < m - 1; ++i) {
            self->bs[i] <<= sft;
            self->bs[i] |= (self->bs[i + 1] & BitSetObject::upper[sft]) >> (BITSET_W - sft);
        }
        self->bs[m-1] <<= sft;
        if (self->r) {
            self->bs[m-1] &= BitSetObject::upper[std::max(self->r-sft, 0)];
        } else {
            self->bs[m-1] &= BitSetObject::upper[BITSET_W-sft];
        }
    }
}

static PyObject *
bitset_lshift(PyObject *a, PyObject *b) {
    BitSetObject *z;

    CHECK_BINOP_BITSET2(a, b);

    int shift_num = _PyObject_AsPositiveInt(b);
    if (shift_num < 0) {
        PyErr_SetString(PyExc_ValueError, "negative shift count");
        return NULL;
    }
    z = bitset_copy((BitSetObject *)a);
    _bitset_lshift(z, shift_num);
    return (PyObject *)z;
}


static PyObject *
bitset_inplace_lshift(PyObject *a, PyObject *b) {

    CHECK_BINOP_BITSET2(a, b);

    int shift_num = _PyObject_AsPositiveInt(b);
    if (shift_num < 0) {
        PyErr_SetString(PyExc_ValueError, "negative shift count");
        return NULL;
    }
    _bitset_lshift((BitSetObject *)a, shift_num);
    Py_INCREF(a);
    return (PyObject *)a;
}


static void
_bitset_rshift(BitSetObject *self, int other) {
    if (other >= self->n) {
        std::fill(self->bs.begin(), self->bs.end(), 0);
        return;
    }
    int rot, sft;
    rot = other >> BITSET_W_SHIFT;
    sft = other - (rot << BITSET_W_SHIFT);
    int m = self->m;
    if (rot) {
        for (int i = m - 1; i >= rot; --i) {
            self->bs[i] = self->bs[i-rot];
        }
        std::fill(self->bs.begin(), self->bs.begin()+rot, 0);
    }
    if (sft) {
        for (int i = m - 1; i > rot; --i) {
            self->bs[i] >>= sft;
            self->bs[i] |= (self->bs[i - 1] & BitSetObject::lower[sft]) << (BITSET_W - sft);
        }
        self->bs[rot] >>= sft;
    }
}

static PyObject *
bitset_rshift(PyObject *a, PyObject *b) {
    BitSetObject *z;

    CHECK_BINOP_BITSET2(a, b);

    int shift_num = _PyObject_AsPositiveInt(b);
    if (shift_num < 0) {
        PyErr_SetString(PyExc_ValueError, "negative shift count");
        return NULL;
    }
    z = bitset_copy((BitSetObject *)a);
    _bitset_rshift(z, shift_num);
    return (PyObject *)z;
}


static PyObject *
bitset_inplace_rshift(PyObject *a, PyObject *b) {

    CHECK_BINOP_BITSET2(a, b);

    int shift_num = _PyObject_AsPositiveInt(b);
    if (shift_num < 0) {
        PyErr_SetString(PyExc_ValueError, "negative shift count");
        return NULL;
    }
    _bitset_rshift((BitSetObject *)a, shift_num);
    Py_INCREF(a);
    return (PyObject *)a;
}

static void
_bitset_and(BitSetObject *self, BitSetObject *other) {
    for (int i = 0; i < self->n; ++i) {
        self->bs[i] &= other->bs[i];
    }
}

static PyObject *
bitset_and(BitSetObject *a, BitSetObject *b) {
    BitSetObject *z;

    CHECK_BINOP_BITSET(a, b);

    z = bitset_copy(a);
    _bitset_and(z, b);
    return (PyObject *)z;
}

static PyObject *
bitset_inplace_and(BitSetObject *a, BitSetObject *b) {

    CHECK_BINOP_BITSET(a, b);

    _bitset_and(a, b);
    Py_INCREF(a);
    return (PyObject *)a;
}

static void
_bitset_xor(BitSetObject *self, BitSetObject *other) {
    for (int i = 0; i < self->n; ++i) {
        self->bs[i] ^= other->bs[i];
    }
}

static PyObject *
bitset_xor(BitSetObject *a, BitSetObject *b) {
    BitSetObject *z;

    CHECK_BINOP_BITSET(a, b);

    z = bitset_copy(a);
    _bitset_xor(z, b);
    return (PyObject *)z;
}

static PyObject *
bitset_inplace_xor(BitSetObject *a, BitSetObject *b) {

    CHECK_BINOP_BITSET(a, b);

    _bitset_xor(a, b);
    Py_INCREF(a);
    return (PyObject *)a;
}

static void
_bitset_or(BitSetObject *self, BitSetObject *other) {
    for (int i = 0; i < self->n; ++i) {
        self->bs[i] |= other->bs[i];
    }
}

static PyObject *
bitset_or(BitSetObject *a, BitSetObject *b) {
    BitSetObject *z;

    CHECK_BINOP_BITSET(a, b);

    z = bitset_copy(a);
    _bitset_or(z, b);
    return (PyObject *)z;
}

static PyObject *
bitset_inplace_or(BitSetObject *a, BitSetObject *b) {

    CHECK_BINOP_BITSET(a, b);

    _bitset_or(a, b);
    Py_INCREF(a);
    return (PyObject *)a;
}



static PyNumberMethods bitset_as_number = {
    0,                                /* nb_add */
    0,                                /* nb_subtract */
    0,                                /* nb_multiply */
    0,                                /* nb_remainder */
    0,                                /* nb_divmod */
    0,                                /* nb_power */
    0,                                /* nb_negative */
    0,                                /* tp_positive */
    0,                                /* tp_absolute */
    0,                                /* tp_bool */
    (unaryfunc)bitset_invert,         /* nb_invert */
    bitset_lshift,                    /* nb_lshift */
    bitset_rshift,                    /* nb_rshift */
    (binaryfunc)bitset_and,           /* nb_and */
    (binaryfunc)bitset_xor,           /* nb_xor */
    (binaryfunc)bitset_or,            /* nb_or */
    0,                                /* nb_int */
    0,                                /* nb_reserved */
    0,                                /* nb_float */
    0,                                /* nb_inplace_add */
    0,                                /* nb_inplace_subtract */
    0,                                /* nb_inplace_multiply */
    0,                                /* nb_inplace_remainder */
    0,                                /* nb_inplace_power */
    bitset_inplace_lshift,            /* nb_inplace_lshift */
    bitset_inplace_rshift,            /* nb_inplace_rshift */
    (binaryfunc)bitset_inplace_and,   /* nb_inplace_and */
    (binaryfunc)bitset_inplace_xor,   /* nb_inplace_xor */
    (binaryfunc)bitset_inplace_or,    /* nb_inplace_or */
    0,                                /* nb_floor_divide */
    0,                                /* nb_true_divide */
    0,                                /* nb_inplace_floor_divide */
    0,                                /* nb_inplace_true_divide */
    0,                                /* nb_index */
};



static PyObject *
bitset_flip(BitSetObject *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs == 0) {
        _bitset_invert(self);
        Py_RETURN_NONE;
    } else if (nargs == 1) {
        int idx = _PyObject_AsPositiveInt(args[0]);
        if (idx < 0 || idx >= self->n){
            PyErr_Format(PyExc_IndexError, "index %d is out of range", idx);
            return NULL;
        }
        int i;
        int j;
        _bitset_get_index(self, idx, &i, &j);
        self->bs[i] ^= BitSetObject::onehot[j];
        Py_RETURN_NONE;
    } else {
        NARGS_VIOLATION("BitSet.flip", 1);
    }
}

PyDoc_STRVAR(bitset_flip_doc,"");

#define ACL_PYTHON_BITSET_FLIP_METHODDEF \
    {"flip", (PyCFunction)(void(*)(void))bitset_flip, METH_FASTCALL, bitset_flip_doc}


static PyObject *
bitset_count(BitSetObject *self, PyObject *args) {
    int cnt = 0;
    std::vector<unsigned long long> bs = self->bs;
    for (int i = 0; i < self->q; ++i) {
        cnt += __builtin_popcountll(bs[i]);
    }
    if (self->r) {
        cnt += __builtin_popcountll(bs[self->q] >> (BITSET_W - self->r));
    }
    return PyLong_FromLong((long)cnt);
}

PyDoc_STRVAR(bitset_count_doc,"");

#define ACL_PYTHON_BITSET_COUNT_METHODDEF \
    {"count", (PyCFunction)bitset_count, METH_NOARGS, bitset_count_doc}


static PyObject *
bitset_all(BitSetObject *self, PyObject *args) {
    for (int i = 0; i < self->q; ++i) {
        if (self->bs[i] != BITSET_ALL) Py_RETURN_FALSE;
    }
    if (self->r) {
        unsigned long long u = BitSetObject::upper[self->r];
        if ((self->bs[self->q] & u) != u) Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

PyDoc_STRVAR(bitset_all_doc,"");

#define ACL_PYTHON_BITSET_ALL_METHODDEF \
    {"all", (PyCFunction)bitset_all, METH_NOARGS, bitset_all_doc}


static PyObject *
bitset_any(BitSetObject *self, PyObject *args) {
    for (int i = 0; i < self->q; ++i) {
        if (self->bs[i]) Py_RETURN_TRUE;
    }
    if (self->r) {
        unsigned long long u = BitSetObject::upper[self->r];
        if (self->bs[self->q] & u) Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyDoc_STRVAR(bitset_any_doc,"");

#define ACL_PYTHON_BITSET_ANY_METHODDEF \
    {"any", (PyCFunction)bitset_any, METH_NOARGS, bitset_any_doc}


static PyObject *
bitset_none(BitSetObject *self, PyObject *args) {
    for (int i = 0; i < self->q; ++i) {
        if (self->bs[i]) Py_RETURN_FALSE;
    }
    if (self->r) {
        unsigned long long u = BitSetObject::upper[self->r];
        if (self->bs[self->q] & u) Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

PyDoc_STRVAR(bitset_none_doc,"");

#define ACL_PYTHON_BITSET_NONE_METHODDEF \
    {"none", (PyCFunction)bitset_none, METH_NOARGS, bitset_none_doc}


static PyObject *
bitset_fill(BitSetObject *self, PyObject *args) {
    int value;
    value = _PyObject_AsPositiveInt(args);
    if (!((value == 0) || (value == 1))) {
        const char * msg = "fill value must be 0 or 1";
        PyErr_SetString(PyExc_ValueError, msg);
        return NULL;
    }
    std::fill(self->bs.begin(), self->bs.end(), value ? BITSET_ALL : 0);
    if (value && (self->r)) {
        self->bs[self->q] ^= BitSetObject::lower[BITSET_W - self->r];
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(bitset_fill_doc,"");

#define ACL_PYTHON_BITSET_FILL_METHODDEF \
    {"fill", (PyCFunction)(void(*)(void))bitset_fill, METH_O, bitset_fill_doc}


static PyObject *
bitset_tostring(BitSetObject *self, PyObject *args) {
    return _bitset_tostring(self);
}

PyDoc_STRVAR(bitset_tostring_doc,"");

#define ACL_PYTHON_BITSET_TOSTRING_METHODDEF \
    {"tostring", (PyCFunction)bitset_tostring, METH_NOARGS, bitset_tostring_doc}


static PyObject *
bitset_toint(BitSetObject *self, PyObject *args) {
    PyObject *unicode = _bitset_tostring(self);
    return PyLong_FromUnicodeObject(unicode, 2);
}

PyDoc_STRVAR(bitset_toint_doc,"");

#define ACL_PYTHON_BITSET_TOINT_METHODDEF \
    {"toint", (PyCFunction)bitset_toint, METH_NOARGS, bitset_toint_doc}




static PyMethodDef bitset_methods[] = {
    ACL_PYTHON_BITSET_FLIP_METHODDEF,
    ACL_PYTHON_BITSET_COUNT_METHODDEF,
    ACL_PYTHON_BITSET_ALL_METHODDEF,
    ACL_PYTHON_BITSET_ANY_METHODDEF,
    ACL_PYTHON_BITSET_NONE_METHODDEF,
    ACL_PYTHON_BITSET_FILL_METHODDEF,
    ACL_PYTHON_BITSET_TOSTRING_METHODDEF,
    ACL_PYTHON_BITSET_TOINT_METHODDEF,
    {NULL} /*sentinel*/
};

static int
bitset_compare(BitSetObject *a, BitSetObject *b) {
    if (a->n != b->n) return 1;
    for (int i = 0; i < a->q; ++i) {
        if (a->bs[i] != b->bs[i]) return 1;
    }
    if (a->r) {
        unsigned long long x, y;
        x = a->bs[a->q] & BitSetObject::upper[a->r];
        y = b->bs[a->q] & BitSetObject::upper[a->r];
        if (x != y) return 1;
    }
    return 0;
}

static PyObject *
bitset_richcompare(PyObject *self, PyObject *other, int op) {
    int result;

    CHECK_BINOP_BITSET_TYPE(self, other);

    if (!(op == Py_EQ || op == Py_NE)) Py_RETURN_NOTIMPLEMENTED;
    result = bitset_compare((BitSetObject *)self, (BitSetObject *)other);
    Py_RETURN_RICHCOMPARE(result, 0, op);
}


static int
bitset_init(BitSetObject *self, PyObject *args, PyObject *kwargs) { 
    int n = 0;
    PyObject *o;


    if (!PyArg_ParseTuple(args, "O", &o)) return -1;
    if (PyLong_Check(o)) {
        n = (int)PyLong_AsLong(o);
        self->bs = std::vector<unsigned long long>(n);

    } else if (PyUnicode_Check(o)) {
        if (PyUnicode_READY(o) == -1) return -1;
        Py_ssize_t m = PyUnicode_GET_LENGTH(o);
        n = (int)m;
        self->bs = std::vector<unsigned long long>(n);
        int kind = PyUnicode_KIND(o);
        void *data = PyUnicode_DATA(o);
        unsigned long long v = 0;
        int i = 0, j = 0;
        for (Py_ssize_t idx = 0; idx < m; ++idx) {
            v <<= 1;
            Py_UCS4 c = PyUnicode_READ(kind, data, idx);
            if (c == 49) {
                v |= 1;
            }
            ++j;
            if (j == BITSET_W) {
                self->bs[i] = v;
                v = 0;
                ++i;
                j = 0;
            } else if (idx == m - 1) {
                self->bs[i] = v << (BITSET_W - j);
            }
        }
    } else {
        return -1;
    }
    self->n = n;
    self->m = (n + BITSET_W - 1) / BITSET_W;
    self->r = n % BITSET_W;
    self->q = self->r ? self->m - 1 : self->m;
    if (!BitSetObject::initialized) {
        self->onehot = std::vector<unsigned long long>(BITSET_W);
        self->shift = std::vector<int>(BITSET_W);
        self->upper = std::vector<unsigned long long>(BITSET_W + 1);
        self->lower = std::vector<unsigned long long>(BITSET_W + 1);
        for(int i = 0; i < BITSET_W; ++i){
            self->onehot[i] = BITSET_ONE << (BITSET_W - i - 1);
            self->shift[i] = BITSET_W - i - 1;
            self->lower[i + 1] = self->lower[i] << 1 | 1;
        }
        for(int i = 1; i <= BITSET_W; ++i){
            self->upper[i] = self->lower[BITSET_W - i] ^ BITSET_ALL;
        }
        BitSetObject::initialized = true;
    }
    self->major = std::vector<int>(n, -1);
    self->minor = std::vector<int>(n, -1);
    


    return 0;
}

static PyObject *
bitset_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    BitSetObject *self;
    self = (BitSetObject *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}


PyTypeObject BitSetType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "atcoder.BitSet",                           /* tp_name */
    sizeof(BitSetObject),                       /* tp_basicsize */
    0,                                          /* tp_itemsize */
    0,                                          /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    bitset_repr,                                /* tp_repr */
    &bitset_as_number,                          /* tp_as_number */
    &bitset_as_sequence,                        /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    PyObject_GenericSetAttr,                    /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    0,                                 /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    bitset_richcompare,                         /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    bitset_methods,                             /* tp_methods */
    0,                                          /* tp_members */
    0,                             /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)bitset_init,                      /* tp_init */
    0,                                          /* tp_alloc */
    bitset_new,                                 /* tp_new */
    PyObject_Del,                               /* tp_free */
};



} // namespace atcoder_python

#endif   // ACL_PYTHON_BITSET