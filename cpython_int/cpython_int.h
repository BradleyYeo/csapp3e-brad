#ifndef CPYTHON_INT_H
#define CPYTHON_INT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PYLONG_BITS_IN_DIGIT 30
#define BASE_MASK            ((digit)((1U << PYLONG_BITS_IN_DIGIT) - 1U)) /* 0x3FFFFFFF */
#define BASE                 ((twodigits)(1ULL << PYLONG_BITS_IN_DIGIT))   /* 1,073,741,824 */

typedef uint32_t digit;
typedef int32_t  sdigit;
typedef uint64_t twodigits;
typedef int64_t  stwodigits;

/* Forward declarations */
typedef struct _typeobject PyTypeObject;
typedef struct _object PyObject;
typedef struct _varobject PyVarObject;
typedef struct _number_methods PyNumberMethods;

/*
 * Phase 1: PyObject Core Runtime & Protocol Suites
 */

/* Number protocol table (vtable slot suite for numeric operations) */
struct _number_methods {
    PyObject *(*nb_add)(PyObject *, PyObject *);
    PyObject *(*nb_subtract)(PyObject *, PyObject *);
    PyObject *(*nb_multiply)(PyObject *, PyObject *);
    PyObject *(*nb_negative)(PyObject *);
    PyObject *(*nb_invert)(PyObject *);
};

/* Type descriptor (Virtual Method Table) */
struct _typeobject {
    const char *tp_name;
    size_t tp_basicsize;
    size_t tp_itemsize;
    void (*tp_dealloc)(PyObject *);
    void (*tp_repr)(const PyObject *, char *buf, size_t bufsize);
    PyNumberMethods *tp_as_number;
};

/* Base object header for all Python objects */
struct _object {
    int64_t ob_refcnt;
    PyTypeObject *ob_type;
};

/* Base object header for variable-length Python objects */
struct _varobject {
    PyObject ob_base;
    int64_t ob_size; /* Dual role: count of items AND sign flag */
};

/*
 * Reference Counting & Object Accessor Macros
 */
#define Py_REFCNT(op)       (((PyObject *)(op))->ob_refcnt)
#define Py_TYPE(op)         (((PyObject *)(op))->ob_type)
#define Py_SIZE(op)         (((PyVarObject *)(op))->ob_size)

#define Py_INCREF(op) do { \
    if ((op) != NULL) { \
        ((PyObject *)(op))->ob_refcnt++; \
    } \
} while (0)

#define Py_DECREF(op) do { \
    PyObject *_py_op = (PyObject *)(op); \
    if (_py_op != NULL) { \
        if (--_py_op->ob_refcnt == 0) { \
            if (_py_op->ob_type && _py_op->ob_type->tp_dealloc) { \
                _py_op->ob_type->tp_dealloc(_py_op); \
            } \
        } \
    } \
} while (0)

/*
 * Phase 2: Minimal Concrete Type (PyFloatObject) & Dynamic Dispatch
 */
typedef struct {
    PyObject ob_base;
    double ob_fval;
} PyFloatObject;

extern PyTypeObject PyFloat_Type;

/* Float constructor and extraction */
PyObject *PyFloat_FromDouble(double v);
double PyFloat_AsDouble(const PyObject *op);

/* Polymorphic runtime dispatchers */
PyObject *PyNumber_Add(PyObject *v, PyObject *w);
PyObject *PyNumber_Multiply(PyObject *v, PyObject *w);

/*
 * Phase 3: Variable-Length Arbitrary-Precision Number (PyLongObject)
 */
typedef struct {
    PyVarObject ob_base;
    digit ob_digit[];
} PyLongObject;

extern PyTypeObject PyLong_Type;

#define Py_ABS(x)           ((x) < 0 ? -(x) : (x))
#define Py_DIGITS(op)       (((PyLongObject *)(op))->ob_digit)
#define PYLONG_IS_ZERO(op)  (Py_SIZE(op) == 0)

/* PyLong constructors and destructors */
PyLongObject *pylong_alloc(size_t size);
void pylong_free(PyLongObject *op);
void pylong_clamp(PyLongObject *op);
int pylong_verify_invariants(const PyLongObject *op);

PyLongObject *pylong_from_int64(int64_t val);
int pylong_to_int64(const PyLongObject *op, int64_t *out);

/* Comparisons */
int pylong_compare_magnitude(const PyLongObject *a, const PyLongObject *b);
int pylong_compare(const PyLongObject *a, const PyLongObject *b);

/* Low-level magnitude arithmetic */
PyLongObject *pylong_add_magnitude(const PyLongObject *a, const PyLongObject *b);
PyLongObject *pylong_sub_magnitude(const PyLongObject *a, const PyLongObject *b);

/* Signed public arithmetic */
PyLongObject *pylong_neg(const PyLongObject *a);
PyLongObject *pylong_add(const PyLongObject *a, const PyLongObject *b);
PyLongObject *pylong_sub(const PyLongObject *a, const PyLongObject *b);
PyLongObject *pylong_mul(const PyLongObject *a, const PyLongObject *b);
PyLongObject *pylong_invert(const PyLongObject *a);
PyLongObject *pylong_and(const PyLongObject *a, const PyLongObject *b);

#endif /* CPYTHON_INT_H */
