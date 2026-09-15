#include "cpython_int.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Forward declaration of long slot methods */
static void long_dealloc(PyObject *op);
static PyObject *long_add_slot(PyObject *v, PyObject *w);
static PyObject *long_sub_slot(PyObject *v, PyObject *w);
static PyObject *long_mul_slot(PyObject *v, PyObject *w);
static PyObject *long_invert_slot(PyObject *v);

/*
 * Phase 1: PyTypeObject Declarations & Protocol Suites
 */

/* Float number protocol suite */
static void float_dealloc(PyObject *op) {
    free(op);
}

static PyObject *float_add(PyObject *v, PyObject *w) {
    if (Py_TYPE(v) != &PyFloat_Type || Py_TYPE(w) != &PyFloat_Type) {
        return NULL;
    }
    double a = ((PyFloatObject *)v)->ob_fval;
    double b = ((PyFloatObject *)w)->ob_fval;
    return PyFloat_FromDouble(a + b);
}

static PyObject *float_multiply(PyObject *v, PyObject *w) {
    if (Py_TYPE(v) != &PyFloat_Type || Py_TYPE(w) != &PyFloat_Type) {
        return NULL;
    }
    double a = ((PyFloatObject *)v)->ob_fval;
    double b = ((PyFloatObject *)w)->ob_fval;
    return PyFloat_FromDouble(a * b);
}

static PyNumberMethods float_as_number = {
    .nb_add = float_add,
    .nb_subtract = NULL,
    .nb_multiply = float_multiply,
    .nb_negative = NULL,
    .nb_invert = NULL,
};

PyTypeObject PyFloat_Type = {
    .tp_name = "float",
    .tp_basicsize = sizeof(PyFloatObject),
    .tp_itemsize = 0,
    .tp_dealloc = float_dealloc,
    .tp_repr = NULL,
    .tp_as_number = &float_as_number,
};

/* Long number protocol suite */
static PyNumberMethods long_as_number = {
    .nb_add = long_add_slot,
    .nb_subtract = long_sub_slot,
    .nb_multiply = long_mul_slot,
    .nb_negative = NULL,
    .nb_invert = long_invert_slot,
};

PyTypeObject PyLong_Type = {
    .tp_name = "int",
    .tp_basicsize = sizeof(PyLongObject),
    .tp_itemsize = sizeof(digit),
    .tp_dealloc = long_dealloc,
    .tp_repr = NULL,
    .tp_as_number = &long_as_number,
};

/*
 * Phase 1: PyObject Initialization Primitives
 */

PyObject *PyObject_Init(PyObject *op, PyTypeObject *type) {
    if (!op) return NULL;
    Py_REFCNT(op) = 1;
    Py_TYPE(op) = type;
    return op;
}

PyVarObject *PyObject_InitVar(PyVarObject *op, PyTypeObject *type, int64_t size) {
    if (!op) return NULL;
    PyObject_Init((PyObject *)op, type);
    Py_SIZE(op) = size;
    return op;
}

/*
 * Phase 2: PyFloatObject Constructors & Polymorphic Dispatchers
 */

PyObject *PyFloat_FromDouble(double v) {
    PyFloatObject *op = (PyFloatObject *)malloc(sizeof(PyFloatObject));
    if (!op) return NULL;
    PyObject_Init((PyObject *)op, &PyFloat_Type);
    op->ob_fval = v;
    return (PyObject *)op;
}

double PyFloat_AsDouble(const PyObject *op) {
    assert(Py_TYPE(op) == &PyFloat_Type);
    return ((const PyFloatObject *)op)->ob_fval;
}

PyObject *PyNumber_Add(PyObject *v, PyObject *w) {
    if (!v || !w) return NULL;
    /* Polymorphic dynamic dispatch via type slot */
    if (v->ob_type && v->ob_type->tp_as_number && v->ob_type->tp_as_number->nb_add) {
        return v->ob_type->tp_as_number->nb_add(v, w);
    }
    return NULL;
}

PyObject *PyNumber_Multiply(PyObject *v, PyObject *w) {
    if (!v || !w) return NULL;
    if (v->ob_type && v->ob_type->tp_as_number && v->ob_type->tp_as_number->nb_multiply) {
        return v->ob_type->tp_as_number->nb_multiply(v, w);
    }
    return NULL;
}

/*
 * Phase 3: PyLongObject & Arbitrary-Precision Math Engine
 */

PyLongObject *pylong_alloc(size_t size) {
    size_t total_bytes = sizeof(PyLongObject) + size * sizeof(digit);
    PyLongObject *op = (PyLongObject *)malloc(total_bytes);
    if (!op) {
        return NULL;
    }

    PyObject_InitVar((PyVarObject *)op, &PyLong_Type, (int64_t)size);
    if (size > 0) {
        memset(op->ob_digit, 0, size * sizeof(digit));
    }
    return op;
}

void pylong_free(PyLongObject *op) {
    Py_DECREF(op);
}

static void long_dealloc(PyObject *op) {
    free(op);
}

void pylong_clamp(PyLongObject *op) {
    if (!op) {
        return;
    }

    size_t n = (size_t)Py_ABS(Py_SIZE(op));
    while (n > 0 && op->ob_digit[n - 1] == 0) {
        n--;
    }

    if (n == 0) {
        Py_SIZE(op) = 0;
    } else if (Py_SIZE(op) < 0) {
        Py_SIZE(op) = -(int64_t)n;
    } else {
        Py_SIZE(op) = (int64_t)n;
    }
}

int pylong_verify_invariants(const PyLongObject *op) {
    if (!op) {
        return 0;
    }

    int64_t size = Py_SIZE(op);
    if (size == 0) {
        return 1;
    }

    size_t n = (size_t)Py_ABS(size);

    /* Invariant 1: No leading zero digits allowed */
    if (op->ob_digit[n - 1] == 0) {
        return 0;
    }

    /* Invariant 2: Digits must strictly conform to base 2^30 */
    for (size_t i = 0; i < n; i++) {
        if (op->ob_digit[i] > BASE_MASK) {
            return 0;
        }
    }

    return 1;
}

PyLongObject *pylong_from_int64(int64_t val) {
    if (val == 0) {
        return pylong_alloc(0);
    }

    int sign = (val < 0) ? -1 : 1;
    uint64_t uval = (val < 0) ? (0ULL - (uint64_t)val) : (uint64_t)val;

    PyLongObject *op = pylong_alloc(3);
    if (!op) {
        return NULL;
    }

    size_t count = 0;
    while (uval > 0) {
        op->ob_digit[count++] = (digit)(uval & BASE_MASK);
        uval >>= PYLONG_BITS_IN_DIGIT;
    }

    Py_SIZE(op) = (sign < 0) ? -(int64_t)count : (int64_t)count;
    pylong_clamp(op);
    return op;
}

int pylong_to_int64(const PyLongObject *op, int64_t *out) {
    if (!op || !out) {
        return -1;
    }

    if (Py_SIZE(op) == 0) {
        *out = 0;
        return 0;
    }

    size_t n = (size_t)Py_ABS(Py_SIZE(op));
    if (n > 3) {
        return -1;
    }

    uint64_t uval = 0;
    for (size_t i = n; i > 0; i--) {
        uval = (uval << PYLONG_BITS_IN_DIGIT) | (uint64_t)op->ob_digit[i - 1];
    }

    if (Py_SIZE(op) > 0) {
        if (uval > (uint64_t)INT64_MAX) {
            return -1;
        }
        *out = (int64_t)uval;
    } else {
        uint64_t max_neg = (uint64_t)1ULL << 63;
        if (uval > max_neg) {
            return -1;
        }
        if (uval == max_neg) {
            *out = INT64_MIN;
        } else {
            *out = -(int64_t)uval;
        }
    }

    return 0;
}

int pylong_compare_magnitude(const PyLongObject *a, const PyLongObject *b) {
    size_t na = (size_t)Py_ABS(Py_SIZE(a));
    size_t nb = (size_t)Py_ABS(Py_SIZE(b));

    if (na > nb) return 1;
    if (na < nb) return -1;

    for (size_t i = na; i > 0; i--) {
        if (a->ob_digit[i - 1] > b->ob_digit[i - 1]) return 1;
        if (a->ob_digit[i - 1] < b->ob_digit[i - 1]) return -1;
    }

    return 0;
}

int pylong_compare(const PyLongObject *a, const PyLongObject *b) {
    int sign_a = (Py_SIZE(a) > 0) ? 1 : ((Py_SIZE(a) < 0) ? -1 : 0);
    int sign_b = (Py_SIZE(b) > 0) ? 1 : ((Py_SIZE(b) < 0) ? -1 : 0);

    if (sign_a > sign_b) return 1;
    if (sign_a < sign_b) return -1;
    if (sign_a == 0) return 0;

    if (sign_a > 0) {
        return pylong_compare_magnitude(a, b);
    }
    return pylong_compare_magnitude(b, a);
}

PyLongObject *pylong_add_magnitude(const PyLongObject *a, const PyLongObject *b) {
    size_t na = (size_t)Py_ABS(Py_SIZE(a));
    size_t nb = (size_t)Py_ABS(Py_SIZE(b));

    if (na < nb) {
        return pylong_add_magnitude(b, a);
    }

    PyLongObject *z = pylong_alloc(na + 1);
    if (!z) return NULL;

    twodigits carry = 0;
    size_t i = 0;

    for (; i < nb; i++) {
        twodigits sum = (twodigits)a->ob_digit[i] + (twodigits)b->ob_digit[i] + carry;
        z->ob_digit[i] = (digit)(sum & BASE_MASK);
        carry = sum >> PYLONG_BITS_IN_DIGIT;
    }

    for (; i < na; i++) {
        twodigits sum = (twodigits)a->ob_digit[i] + carry;
        z->ob_digit[i] = (digit)(sum & BASE_MASK);
        carry = sum >> PYLONG_BITS_IN_DIGIT;
    }

    if (carry > 0) {
        z->ob_digit[i++] = (digit)carry;
    }

    Py_SIZE(z) = (int64_t)i;
    pylong_clamp(z);
    return z;
}

PyLongObject *pylong_sub_magnitude(const PyLongObject *a, const PyLongObject *b) {
    size_t na = (size_t)Py_ABS(Py_SIZE(a));
    size_t nb = (size_t)Py_ABS(Py_SIZE(b));

    assert(pylong_compare_magnitude(a, b) >= 0);

    PyLongObject *z = pylong_alloc(na);
    if (!z) return NULL;

    sdigit borrow = 0;
    size_t i = 0;

    for (; i < nb; i++) {
        stwodigits diff = (stwodigits)a->ob_digit[i] - (stwodigits)b->ob_digit[i] - borrow;
        if (diff < 0) {
            diff += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        z->ob_digit[i] = (digit)diff;
    }

    for (; i < na; i++) {
        stwodigits diff = (stwodigits)a->ob_digit[i] - borrow;
        if (diff < 0) {
            diff += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        z->ob_digit[i] = (digit)diff;
    }

    assert(borrow == 0);
    Py_SIZE(z) = (int64_t)na;
    pylong_clamp(z);
    return z;
}

PyLongObject *pylong_neg(const PyLongObject *a) {
    size_t n = (size_t)Py_ABS(Py_SIZE(a));
    PyLongObject *z = pylong_alloc(n);
    if (!z) return NULL;

    if (n > 0) {
        memcpy(z->ob_digit, a->ob_digit, n * sizeof(digit));
    }
    Py_SIZE(z) = -Py_SIZE(a);
    return z;
}

PyLongObject *pylong_add(const PyLongObject *a, const PyLongObject *b) {
    if (Py_SIZE(a) == 0) {
        size_t nb = (size_t)Py_ABS(Py_SIZE(b));
        PyLongObject *z = pylong_alloc(nb);
        if (!z) return NULL;
        if (nb > 0) memcpy(z->ob_digit, b->ob_digit, nb * sizeof(digit));
        Py_SIZE(z) = Py_SIZE(b);
        return z;
    }
    if (Py_SIZE(b) == 0) {
        size_t na = (size_t)Py_ABS(Py_SIZE(a));
        PyLongObject *z = pylong_alloc(na);
        if (!z) return NULL;
        if (na > 0) memcpy(z->ob_digit, a->ob_digit, na * sizeof(digit));
        Py_SIZE(z) = Py_SIZE(a);
        return z;
    }

    if ((Py_SIZE(a) > 0 && Py_SIZE(b) > 0) || (Py_SIZE(a) < 0 && Py_SIZE(b) < 0)) {
        PyLongObject *z = pylong_add_magnitude(a, b);
        if (!z) return NULL;
        if (Py_SIZE(a) < 0) {
            Py_SIZE(z) = -Py_SIZE(z);
        }
        return z;
    }

    int cmp = pylong_compare_magnitude(a, b);
    if (cmp == 0) {
        return pylong_alloc(0);
    }

    if (cmp > 0) {
        PyLongObject *z = pylong_sub_magnitude(a, b);
        if (!z) return NULL;
        if (Py_SIZE(a) < 0) {
            Py_SIZE(z) = -Py_SIZE(z);
        }
        return z;
    } else {
        PyLongObject *z = pylong_sub_magnitude(b, a);
        if (!z) return NULL;
        if (Py_SIZE(b) < 0) {
            Py_SIZE(z) = -Py_SIZE(z);
        }
        return z;
    }
}

PyLongObject *pylong_sub(const PyLongObject *a, const PyLongObject *b) {
    PyLongObject *neg_b = pylong_neg(b);
    if (!neg_b) return NULL;

    PyLongObject *res = pylong_add(a, neg_b);
    pylong_free(neg_b);
    return res;
}

PyLongObject *pylong_mul(const PyLongObject *a, const PyLongObject *b) {
    if (Py_SIZE(a) == 0 || Py_SIZE(b) == 0) {
        return pylong_alloc(0);
    }

    size_t na = (size_t)Py_ABS(Py_SIZE(a));
    size_t nb = (size_t)Py_ABS(Py_SIZE(b));

    PyLongObject *z = pylong_alloc(na + nb);
    if (!z) return NULL;

    for (size_t i = 0; i < na; i++) {
        twodigits carry = 0;
        twodigits ai = (twodigits)a->ob_digit[i];
        size_t j = 0;

        for (; j < nb || carry > 0; j++) {
            twodigits prod = (twodigits)z->ob_digit[i + j] + carry;
            if (j < nb) {
                prod += ai * (twodigits)b->ob_digit[j];
            }
            z->ob_digit[i + j] = (digit)(prod & BASE_MASK);
            carry = prod >> PYLONG_BITS_IN_DIGIT;
        }
    }

    int sign = ((Py_SIZE(a) < 0) ^ (Py_SIZE(b) < 0)) ? -1 : 1;
    Py_SIZE(z) = (sign < 0) ? -(int64_t)(na + nb) : (int64_t)(na + nb);
    pylong_clamp(z);
    return z;
}

PyLongObject *pylong_invert(const PyLongObject *a) {
    PyLongObject *one = pylong_from_int64(1);
    if (!one) return NULL;

    PyLongObject *plus_one = pylong_add(a, one);
    pylong_free(one);
    if (!plus_one) return NULL;

    PyLongObject *res = pylong_neg(plus_one);
    pylong_free(plus_one);
    return res;
}

PyLongObject *pylong_and(const PyLongObject *a, const PyLongObject *b) {
    if (Py_SIZE(a) >= 0 && Py_SIZE(b) >= 0) {
        size_t na = (size_t)Py_SIZE(a);
        size_t nb = (size_t)Py_SIZE(b);
        size_t min_n = (na < nb) ? na : nb;

        PyLongObject *z = pylong_alloc(min_n);
        if (!z) return NULL;

        for (size_t i = 0; i < min_n; i++) {
            z->ob_digit[i] = a->ob_digit[i] & b->ob_digit[i];
        }

        Py_SIZE(z) = (int64_t)min_n;
        pylong_clamp(z);
        return z;
    }
    return NULL;
}

/*
 * Long slot adapter methods for PyLong_Type
 */

static PyObject *long_add_slot(PyObject *v, PyObject *w) {
    if (Py_TYPE(v) != &PyLong_Type || Py_TYPE(w) != &PyLong_Type) {
        return NULL;
    }
    return (PyObject *)pylong_add((const PyLongObject *)v, (const PyLongObject *)w);
}

static PyObject *long_sub_slot(PyObject *v, PyObject *w) {
    if (Py_TYPE(v) != &PyLong_Type || Py_TYPE(w) != &PyLong_Type) {
        return NULL;
    }
    return (PyObject *)pylong_sub((const PyLongObject *)v, (const PyLongObject *)w);
}

static PyObject *long_mul_slot(PyObject *v, PyObject *w) {
    if (Py_TYPE(v) != &PyLong_Type || Py_TYPE(w) != &PyLong_Type) {
        return NULL;
    }
    return (PyObject *)pylong_mul((const PyLongObject *)v, (const PyLongObject *)w);
}

static PyObject *long_invert_slot(PyObject *v) {
    if (Py_TYPE(v) != &PyLong_Type) {
        return NULL;
    }
    return (PyObject *)pylong_invert((const PyLongObject *)v);
}
