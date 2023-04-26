#ifndef ACL_PYTHON_MODINT
#define ACL_PYTHON_MODINT


#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <utility>


#include "atcoder/internal_math"



namespace atcoder_python {


/* modint object **************************************/

/* It is the struct that treats the modular arithmetic.
reference: https://github.com/atcoder/ac-library/blob/master/atcoder/modint.hpp
           https://github.com/atcoder/ac-library/blob/master/atcoder/internal_math.hpp


    >>> ModInt.set_mod(7)
    >>> a = ModInt(12)
    >>> a
    5               # 12 == 5  (mod 7)
    >>> a += 6
    >>> a
    4               # 5 + 6 == 4  (mod 7)
    >>> a *= 3
    >>> a
    5               # 4 * 3 == 5  (mod 7)
    >>> ModInt(4).inv
    2               # 4 * 2 == 1  (mod 7)
    >>> a //= 4
    >>> a
    3               # equivalent to `a *= ModInt(4).inv`
    >>> a **= 2
    >>> a
    2               # 3 ** 2 == 2  (mod 7)


*/

extern PyTypeObject ModIntType;


#define ModInt_Check(v) PyObject_TypeCheck(v, &ModIntType)

#define CHECK_BINOP_MODINT(v, w)                           \
    do {                                                  \
        if ((!PyLong_Check(v) && !ModInt_Check(v))        \
            || (!PyLong_Check(w) && !ModInt_Check(w))){   \
            Py_RETURN_NOTIMPLEMENTED;                     \
        }                                                 \
    } while (0)

struct ModIntObject
{
    PyObject_HEAD
    unsigned int v;
    static inline unsigned int mod;
    static inline unsigned long long im;
};


static ModIntObject *
ModInt_FromUnsignedInt(unsigned int v)
{
    PyTypeObject* type = &ModIntType;
    ModIntObject* z = (ModIntObject*)type->tp_alloc(type, 0);
    if (v >= ModIntObject::mod) v %= ModIntObject::mod;
    z->v = v;
    return z;
}


static unsigned int
ModInt_AsUnsignedInt(PyObject *vv)
{
    ModIntObject *v;

    if (!ModInt_Check(vv)) return -1;
    v = (ModIntObject *)vv;
    return v->v;
}


static unsigned int
UnsignedInt_FromPyObject(PyObject *o)
{
    long v;
    if (PyLong_Check(o)){
        v = PyLong_AsLong(PyNumber_Remainder(o, PyLong_FromUnsignedLong((unsigned long)ModIntObject::mod)));
    } else {
        v = (long)ModInt_AsUnsignedInt(o);
    }
    if (v >= ModIntObject::mod) v %= ModIntObject::mod;
    return (unsigned int)v;
}

static PyObject *
ModInt_to_decimal_string(PyObject *vv)
{
    ModIntObject *v;

    if (!ModInt_Check(vv)) return NULL;
    v = (ModIntObject *)vv;
    return PyUnicode_FromFormat("%u", v->v);

}


static PyObject *
ModInt_add(PyObject *a, PyObject *b)
{
    ModIntObject *z;

    CHECK_BINOP_MODINT(a, b);

    unsigned int _a, _b, result;
    _a = UnsignedInt_FromPyObject(a);
    _b = UnsignedInt_FromPyObject(b);
    result = _a + _b;
    if (result >= ModIntObject::mod) result -= ModIntObject::mod;
    z = ModInt_FromUnsignedInt(result);
    return (PyObject *)z;
}

static PyObject *
ModInt_sub(PyObject *a, PyObject *b)
{
    ModIntObject *z;

    CHECK_BINOP_MODINT(a, b);

    unsigned int _a, _b, result;
    _a = UnsignedInt_FromPyObject(a);
    _b = UnsignedInt_FromPyObject(b);
    result = _a - _b;
    if (result >= ModIntObject::mod) result += ModIntObject::mod;
    z = ModInt_FromUnsignedInt(result);
    return (PyObject *)z;
}

static unsigned int
_ModInt_mul(unsigned int a, unsigned int b)
{
    unsigned long long z = a;
    z *= b;
    unsigned long long x = (unsigned long long)(((unsigned __int128)(z)*ModIntObject::im) >> 64);
    // unsigned int result = (unsigned int)(z - x * ModIntObject::mod);
    // if (result >= ModIntObject::mod) result += ModIntObject::mod;
    unsigned long long y = x * ModIntObject::mod;
    return (unsigned int)(z - y + (z < y ? ModIntObject::mod : 0));
}

static PyObject *
ModInt_mul(PyObject *a, PyObject *b)
{
    ModIntObject *z;

    CHECK_BINOP_MODINT(a, b);

    unsigned int _a, _b;
    _a = UnsignedInt_FromPyObject(a);
    _b = UnsignedInt_FromPyObject(b);
    unsigned int result = _ModInt_mul(_a, _b);
    z = ModInt_FromUnsignedInt(result);
    return (PyObject *)z;
}

static unsigned int
_ModInt_pow(unsigned int a, unsigned long long n)
{
    unsigned int result = 1;
    while (n) {
        if (n & 1) result = _ModInt_mul(result, a);
        a = _ModInt_mul(a, a);
        n >>= 1;
    }
    return result;
}


static PyObject *
ModInt_pow(PyObject *v, PyObject *w, PyObject *x)
{
    ModIntObject *z;

    if (!ModInt_Check(v) || !PyLong_Check(w)) Py_RETURN_NOTIMPLEMENTED;

    int overflow;
    unsigned int result = 1;
    unsigned int a = ModInt_AsUnsignedInt(v);
    long long n = PyLong_AsLongLongAndOverflow(w, &overflow);
    if (overflow) {
        PyErr_SetString(PyExc_OverflowError, "exponent -> long long");
        return NULL;
    }
    if (n < 0) {
        auto eg = inv_gcd(a, ModIntObject::mod);
        if (eg.first != 1){
            const char* msg = "There is no inverse element of %u in mod %u";
            return PyErr_Format(PyExc_ValueError, msg, a, ModIntObject::mod);
        }
        a = (unsigned int)eg.second;
        n = -n;
    }
    result = _ModInt_pow(a, (unsigned long long)n);
    z = ModInt_FromUnsignedInt(result);
    return (PyObject *)z;
}

static PyObject *
ModInt_neg(PyObject *v)
{
    ModIntObject *z;
    int a = (int)ModInt_AsUnsignedInt(v);
    a = -a + ModIntObject::mod;
    z = ModInt_FromUnsignedInt((unsigned int)a);
    return (PyObject *)z;
}

static PyObject *
ModInt_pos(PyObject *v)
{
    Py_INCREF(v);
    return v;
}

static int
ModInt_bool(PyObject *v)
{
    return ModInt_AsUnsignedInt(v) != 0;
}

static PyObject *
ModInt_AsPyLong(PyObject *v)
{
    return (PyObject *)PyLong_FromUnsignedLong((unsigned long)ModInt_AsUnsignedInt(v));
}



static PyObject *
ModInt_floor_div(PyObject *a, PyObject *b)
{
    ModIntObject *z;

    CHECK_BINOP_MODINT(a, b);

    unsigned int _a, _b;
    _a = UnsignedInt_FromPyObject(a);
    _b = UnsignedInt_FromPyObject(b);
    auto eg = inv_gcd(_b, ModIntObject::mod);
    if (eg.first != 1){
        const char* msg = "There is no inverse element of %u in mod %u";
        return PyErr_Format(PyExc_ValueError, msg, _b, ModIntObject::mod);
    }
    z = ModInt_FromUnsignedInt(_ModInt_mul(_a, (unsigned int)eg.second));
    return (PyObject *)z;
}




static PyNumberMethods ModInt_as_number = {
    (binaryfunc)ModInt_add,     /* nb_add */
    (binaryfunc)ModInt_sub,     /* nb_subtract */
    (binaryfunc)ModInt_mul,     /* nb_multiply */
    0,                          /* nb_remainder */
    0,                          /* nb_divmod */
    ModInt_pow,                 /* nb_power */
    ModInt_neg,                 /* nb_negative */
    ModInt_pos,                 /* tp_positive */
    0,                          /* tp_absolute */
    ModInt_bool,                /* tp_bool */
    0,                          /* nb_invert */
    0,                          /* nb_lshift */
    0,                          /* nb_rshift */
    0,                          /* nb_and */
    0,                          /* nb_xor */
    0,                          /* nb_or */
    ModInt_AsPyLong,            /* nb_int */
    0,                          /* nb_reserved */
    0,                          /* nb_float */
    0,                          /* nb_inplace_add */
    0,                          /* nb_inplace_subtract */
    0,                          /* nb_inplace_multiply */
    0,                          /* nb_inplace_remainder */
    0,                          /* nb_inplace_power */
    0,                          /* nb_inplace_lshift */
    0,                          /* nb_inplace_rshift */
    0,                          /* nb_inplace_and */
    0,                          /* nb_inplace_xor */
    0,                          /* nb_inplace_or */
    ModInt_floor_div,            /* nb_floor_divide */
    0,                          /* nb_true_divide */
    0,                          /* nb_inplace_floor_divide */
    0,                          /* nb_inplace_true_divide */
    ModInt_AsPyLong,            /* nb_index */
};


PyDoc_STRVAR(ModInt_doc,
u8"It is the struct that treats the modular arithmetic\n"
"implemented based on AtCoder Library.\n\n"
"The following operations between (ModInt/int) and (ModInt/int)\n"
"are supported:\n"
"    '+', '+=', '-', '-=', '*', '*=', '//', '//=',\n"
"    '==', '!='\n\n"
"And the following operations between (ModInt) and (int)\n"
"are supported:\n"
"    '**', '**='\n\n"
"You must first set the mod using the method ModInt.set_mod().\n\n"
"ModInt(n)  (Constructor)\n"
"    Parameters\n"
"    ----------\n"
"    n : int\n"
"    \n"
"    Returns\n"
"    -------\n"
"    x : ModInt\n"
"    \n"
"    Constraints\n"
"    -----------\n"
"    mod is already set\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(1)\n\n"
"Operations ('+', '+=', '-', '-=', '*', '*=', '==', '!=')\n"
"    Constraints\n"
"    -----------\n"
"    Nothing\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(1)\n\n"
"Operations ('//', '//=')\n"
"    Constraints\n"
"    -----------\n"
"    gcd(rhs, mod) = 1\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(log(mod))\n\n"
"Operations ('**', '**=')\n"
"    If rhs is negative, lhs is replaced by inverse element of lhs\n"
"    (and rhs is replaced by -rhs).\n"
"    If you want the inverse element of lhs, use 'inv' attribute\n"
"    insted of this.\n\n"
"    Constraints\n"
"    -----------\n"
"    -2^63 <= rhs < 2^63\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(log(rhs))\n\n"
"Attribute ('inv')\n"
"    Get the inverse element\n"
"    Note that the result is calculated each time you access.\n\n"
"    Constraints\n"
"    -----------\n"
"    gcd(self, mod) = 1\n"
"    \n"
"    Complexity\n"
"    ----------\n"
"    \u039F(log(mod))"
);


static int
ModInt_compare(unsigned int self, unsigned int other)
{
    return self == other ? 0 : 1;
}

static PyObject *
ModInt_richcompare(PyObject *self, PyObject *other, int op)
{
    int result;

    CHECK_BINOP_MODINT(self, other);

    if (!(op == Py_EQ || op == Py_NE)) Py_RETURN_NOTIMPLEMENTED;
    result = ModInt_compare(UnsignedInt_FromPyObject(self), UnsignedInt_FromPyObject(other));
    Py_RETURN_RICHCOMPARE(result, 0, op);
}




static PyObject *
ModInt_get_mod(ModIntObject *self, PyObject *args) {
    return PyLong_FromUnsignedLong((unsigned long)self->mod);
}

PyDoc_STRVAR(modint_get_mod_doc,
u8"get_mod()\n"
"--\n\n"
"Get the mod\n\n"
"Parameters\n"
"----------\n"
"Nothing\n"
"\n"
"Returns\n"
"-------\n"
"mod : int\n"
"    the mod value of ModInt\n"
"\n"
"Constraints\n"
"-----------\n"
"Nothing\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(1)"
);

#define ACL_PYTHON_MODINT_GET_MOD_METHODDEF \
    {"get_mod",                            \
     (PyCFunction)ModInt_get_mod,          \
     METH_NOARGS | METH_CLASS,             \
     modint_get_mod_doc},




static PyObject *
ModInt_set_mod(ModIntObject *self, PyObject *args) {
    unsigned int m;
    if (!PyArg_ParseTuple(args, "I", &m)) return NULL;
    ModIntObject::mod = m;
    ModIntObject::im = (unsigned long long)(-1) / m + 1;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(modint_set_mod_doc,
u8"set_mod(mod)\n"
"--\n\n"
"Set the mod\n\n"
"Parameters\n"
"----------\n"
"mod : int\n"
"    the mod value\n"
"\n"
"Returns\n"
"-------\n"
"None\n"
"\n"
"Constraints\n"
"-----------\n"
"1 <= mod <= 2 * 10^9 + 1000\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(1)"
);

#define ACL_PYTHON_MODINT_SET_MOD_METHODDEF \
    {"set_mod",                            \
     (PyCFunction)ModInt_set_mod,          \
     METH_VARARGS | METH_CLASS,            \
     modint_set_mod_doc},

static PyMethodDef ModInt_methods[] = {
    ACL_PYTHON_MODINT_GET_MOD_METHODDEF
    ACL_PYTHON_MODINT_SET_MOD_METHODDEF
    {NULL} /* Sentinel */
};

// static PyMemberDef ModInt_members[] = {
//     {"v", T_UINT, offsetof(ModIntObject, v), READONLY},
//     {NULL} /* Sentinel */
// };

static PyObject *
ModInt_get_inv(PyObject *self, void *closure)
{
    ModIntObject *v;
    v = (ModIntObject *)self;
    auto eg = inv_gcd(v->v, ModIntObject::mod);
    assert(eg.first == 1);
    return (PyObject *)ModInt_FromUnsignedInt(eg.second);
}

PyDoc_STRVAR(modint_inv_doc,
u8"Get the inverse element\n\n"
"Parameters\n"
"----------\n"
"Nothing\n"
"\n"
"Returns\n"
"-------\n"
" x : int\n"
"    inverse element\n"
"\n"
"Constraints\n"
"-----------\n"
"gcd(self, mod) = 1\n"
"\n"
"Complexity\n"
"----------\n"
"\u039F(log(mod))"
);

static PyGetSetDef ModInt_getsets[] = {
    {"inv", (getter)ModInt_get_inv, NULL, modint_inv_doc, NULL},
    {NULL} /* Sentinel */
};

static int
ModInt_init(ModIntObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *o = nullptr;
    long v = 0;

    if (!PyArg_ParseTuple(args, "|O", &o)) return -1;
    if (o == nullptr) {
        v = 0;
    } else if (PyLong_Check(o)){
        if (Py_ABS(Py_SIZE(o)) > 1 || Py_SIZE(o) < 0) {
            o = PyNumber_Remainder(o, PyLong_FromUnsignedLong((unsigned long)self->mod));
        }
        v = PyLong_AsLong(o);
    } else if (ModInt_Check(o)) {
        v = (long)ModInt_AsUnsignedInt(o);
    } else {
        PyErr_SetString(PyExc_TypeError, "required: 'int' or 'ModInt'");
        return -1;
    }

    if (v >= self->mod) v %= (long)self->mod;
    if (v < 0) v += self->mod;

    self->v = v;
    return 0;
}

static PyObject *
ModInt_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    ModIntObject *self;
    self = (ModIntObject *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}


PyTypeObject ModIntType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "atcoder.ModInt",                           /* tp_name */
    sizeof(ModIntObject),                       /* tp_basicsize */
    0,                                          /* tp_itemsize */
    0,                                          /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    ModInt_to_decimal_string,                   /* tp_repr */
    &ModInt_as_number,                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    PyObject_GenericSetAttr,                    /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    ModInt_doc,                                 /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    ModInt_richcompare,                         /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    ModInt_methods,                             /* tp_methods */
    0,                                          /* tp_members */
    ModInt_getsets,                             /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)ModInt_init,                      /* tp_init */
    0,                                          /* tp_alloc */
    ModInt_new,                                 /* tp_new */
    PyObject_Del,                               /* tp_free */
};




} // namespace atcoder_python


#endif  // ACL_PYTHON_MODINT