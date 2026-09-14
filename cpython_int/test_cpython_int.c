#include "cpython_int.h"
#include "test_framework.h"

/* Global tracker for testing custom deallocator invocation */
static int g_custom_dealloc_called = 0;

static void dummy_dealloc(PyObject *op) {
    g_custom_dealloc_called++;
    free(op);
}

static PyTypeObject Dummy_Type = {
    .tp_name = "dummy",
    .tp_basicsize = sizeof(PyObject),
    .tp_itemsize = 0,
    .tp_dealloc = dummy_dealloc,
    .tp_repr = NULL,
    .tp_as_number = NULL,
};

/*
 * Phase 1 Tests: PyObject Core Runtime, Reference Counting, and Destructors
 */
static void test_phase1_pyobject_refcount_and_dealloc(void) {
    g_custom_dealloc_called = 0;

    PyObject *obj = (PyObject *)malloc(sizeof(PyObject));
    ASSERT_NOT_NULL(obj);
    Py_REFCNT(obj) = 1;
    Py_TYPE(obj) = &Dummy_Type;

    /* Increment reference count */
    Py_INCREF(obj);
    ASSERT_EQ_INT(Py_REFCNT(obj), 2);

    /* First decrement should not trigger deallocation */
    Py_DECREF(obj);
    ASSERT_EQ_INT(Py_REFCNT(obj), 1);
    ASSERT_EQ_INT(g_custom_dealloc_called, 0);

    /* Second decrement reaches 0: must invoke custom tp_dealloc */
    Py_DECREF(obj);
    ASSERT_EQ_INT(g_custom_dealloc_called, 1);
}

/*
 * Phase 2 Tests: PyFloatObject, Method Slots, and Polymorphic Dispatch
 */
static void test_phase2_pyfloat_and_dispatch(void) {
    PyObject *f1 = PyFloat_FromDouble(3.5);
    PyObject *f2 = PyFloat_FromDouble(2.5);
    ASSERT_NOT_NULL(f1);
    ASSERT_NOT_NULL(f2);
    ASSERT_EQ_INT(Py_REFCNT(f1), 1);
    ASSERT_TRUE(Py_TYPE(f1) == &PyFloat_Type);

    /* Test polymorphic addition via PyNumber_Add */
    PyObject *fsum = PyNumber_Add(f1, f2);
    ASSERT_NOT_NULL(fsum);
    ASSERT_TRUE(Py_TYPE(fsum) == &PyFloat_Type);
    ASSERT_TRUE(PyFloat_AsDouble(fsum) == 6.0);

    /* Test polymorphic multiplication via PyNumber_Multiply */
    PyObject *fprod = PyNumber_Multiply(f1, f2);
    ASSERT_NOT_NULL(fprod);
    ASSERT_TRUE(Py_TYPE(fprod) == &PyFloat_Type);
    ASSERT_TRUE(PyFloat_AsDouble(fprod) == 8.75);

    /* Decref and ensure clean destruction */
    Py_DECREF(f1);
    Py_DECREF(f2);
    Py_DECREF(fsum);
    Py_DECREF(fprod);
}

/*
 * Phase 3 Tests: Polymorphic Dispatch with PyLongObject
 */
static void test_phase3_pylong_polymorphic_dispatch(void) {
    PyLongObject *l1 = pylong_from_int64(120);
    PyLongObject *l2 = pylong_from_int64(80);

    /* Pass PyLongObject pointers as generic PyObject* to PyNumber_Add */
    PyObject *res_sum = PyNumber_Add((PyObject *)l1, (PyObject *)l2);
    ASSERT_NOT_NULL(res_sum);
    ASSERT_TRUE(Py_TYPE(res_sum) == &PyLong_Type);

    int64_t val = 0;
    ASSERT_EQ_INT(pylong_to_int64((PyLongObject *)res_sum, &val), 0);
    ASSERT_EQ_INT(val, 200);

    Py_DECREF(l1);
    Py_DECREF(l2);
    Py_DECREF(res_sum);
}

/*
 * Bignum Layer 1 Tests: Allocation, Clamping, and Invariant Verification
 */
static void test_layer1_alloc_and_clamp(void) {
    PyLongObject *z = pylong_alloc(0);
    ASSERT_NOT_NULL(z);
    ASSERT_EQ_INT(Py_SIZE(z), 0);
    ASSERT_EQ_INT(Py_REFCNT(z), 1);
    ASSERT_TRUE(pylong_verify_invariants(z));
    pylong_free(z);

    PyLongObject *a = pylong_alloc(3);
    ASSERT_NOT_NULL(a);
    a->ob_digit[0] = 42;
    a->ob_digit[1] = 0;
    a->ob_digit[2] = 0;
    Py_SIZE(a) = 3;

    ASSERT_FALSE(pylong_verify_invariants(a));

    pylong_clamp(a);
    ASSERT_EQ_INT(Py_SIZE(a), 1);
    ASSERT_EQ_INT(a->ob_digit[0], 42);
    ASSERT_TRUE(pylong_verify_invariants(a));
    pylong_free(a);

    PyLongObject *all_zeros = pylong_alloc(4);
    all_zeros->ob_digit[0] = 0;
    all_zeros->ob_digit[1] = 0;
    all_zeros->ob_digit[2] = 0;
    all_zeros->ob_digit[3] = 0;
    Py_SIZE(all_zeros) = -4;

    pylong_clamp(all_zeros);
    ASSERT_EQ_INT(Py_SIZE(all_zeros), 0);
    ASSERT_TRUE(pylong_verify_invariants(all_zeros));
    pylong_free(all_zeros);
}

/*
 * Bignum Layer 2 Tests: Conversions From / To Native 64-bit Hardware Integers
 */
static void test_layer2_conversions(void) {
    int64_t roundtrip = 0;

    PyLongObject *v0 = pylong_from_int64(0);
    ASSERT_NOT_NULL(v0);
    ASSERT_EQ_INT(Py_SIZE(v0), 0);
    ASSERT_TRUE(pylong_verify_invariants(v0));
    ASSERT_EQ_INT(pylong_to_int64(v0, &roundtrip), 0);
    ASSERT_EQ_INT(roundtrip, 0);
    pylong_free(v0);

    PyLongObject *v42 = pylong_from_int64(42);
    ASSERT_NOT_NULL(v42);
    ASSERT_EQ_INT(Py_SIZE(v42), 1);
    ASSERT_EQ_INT(v42->ob_digit[0], 42);
    ASSERT_TRUE(pylong_verify_invariants(v42));
    ASSERT_EQ_INT(pylong_to_int64(v42, &roundtrip), 0);
    ASSERT_EQ_INT(roundtrip, 42);
    pylong_free(v42);

    PyLongObject *vn42 = pylong_from_int64(-42);
    ASSERT_NOT_NULL(vn42);
    ASSERT_EQ_INT(Py_SIZE(vn42), -1);
    ASSERT_EQ_INT(vn42->ob_digit[0], 42);
    ASSERT_TRUE(pylong_verify_invariants(vn42));
    ASSERT_EQ_INT(pylong_to_int64(vn42, &roundtrip), 0);
    ASSERT_EQ_INT(roundtrip, -42);
    pylong_free(vn42);

    int64_t max_1digit = (1LL << PYLONG_BITS_IN_DIGIT) - 1;
    PyLongObject *v_boundary = pylong_from_int64(max_1digit);
    ASSERT_EQ_INT(Py_SIZE(v_boundary), 1);
    ASSERT_EQ_INT(v_boundary->ob_digit[0], BASE_MASK);
    ASSERT_TRUE(pylong_verify_invariants(v_boundary));
    ASSERT_EQ_INT(pylong_to_int64(v_boundary, &roundtrip), 0);
    ASSERT_EQ_INT(roundtrip, max_1digit);
    pylong_free(v_boundary);

    int64_t pow2_30 = 1LL << PYLONG_BITS_IN_DIGIT;
    PyLongObject *v_2digits = pylong_from_int64(pow2_30);
    ASSERT_EQ_INT(Py_SIZE(v_2digits), 2);
    ASSERT_EQ_INT(v_2digits->ob_digit[0], 0);
    ASSERT_EQ_INT(v_2digits->ob_digit[1], 1);
    ASSERT_TRUE(pylong_verify_invariants(v_2digits));
    ASSERT_EQ_INT(pylong_to_int64(v_2digits, &roundtrip), 0);
    ASSERT_EQ_INT(roundtrip, pow2_30);
    pylong_free(v_2digits);

    int64_t max64 = INT64_MAX;
    PyLongObject *v_max64 = pylong_from_int64(max64);
    ASSERT_EQ_INT(Py_SIZE(v_max64), 3);
    ASSERT_TRUE(pylong_verify_invariants(v_max64));
    ASSERT_EQ_INT(pylong_to_int64(v_max64, &roundtrip), 0);
    ASSERT_EQ_INT(roundtrip, max64);
    pylong_free(v_max64);

    int64_t min64 = INT64_MIN;
    PyLongObject *v_min64 = pylong_from_int64(min64);
    ASSERT_EQ_INT(Py_SIZE(v_min64), -3);
    ASSERT_TRUE(pylong_verify_invariants(v_min64));
    ASSERT_EQ_INT(pylong_to_int64(v_min64, &roundtrip), 0);
    ASSERT_EQ_INT(roundtrip, min64);
    pylong_free(v_min64);
}

/*
 * Bignum Layer 3 Tests: Magnitude and Signed Comparisons
 */
static void test_layer3_comparisons(void) {
    PyLongObject *pos5 = pylong_from_int64(5);
    PyLongObject *neg5 = pylong_from_int64(-5);
    PyLongObject *pos10 = pylong_from_int64(10);
    PyLongObject *zero = pylong_from_int64(0);
    PyLongObject *large = pylong_from_int64(1LL << 35);

    ASSERT_EQ_INT(pylong_compare_magnitude(pos5, neg5), 0);
    ASSERT_EQ_INT(pylong_compare_magnitude(pos10, pos5), 1);
    ASSERT_EQ_INT(pylong_compare_magnitude(pos5, pos10), -1);
    ASSERT_EQ_INT(pylong_compare_magnitude(zero, pos5), -1);
    ASSERT_EQ_INT(pylong_compare_magnitude(large, pos10), 1);

    ASSERT_EQ_INT(pylong_compare(pos5, neg5), 1);
    ASSERT_EQ_INT(pylong_compare(neg5, pos5), -1);
    ASSERT_EQ_INT(pylong_compare(neg5, neg5), 0);
    ASSERT_EQ_INT(pylong_compare(pos5, pos5), 0);
    ASSERT_EQ_INT(pylong_compare(zero, pos5), -1);
    ASSERT_EQ_INT(pylong_compare(zero, neg5), 1);
    ASSERT_EQ_INT(pylong_compare(pos10, large), -1);

    pylong_free(pos5);
    pylong_free(neg5);
    pylong_free(pos10);
    pylong_free(zero);
    pylong_free(large);
}

/*
 * Bignum Layer 4 Tests: Magnitude Addition & Subtraction
 */
static void test_layer4_magnitude_arithmetic(void) {
    PyLongObject *max_digit = pylong_from_int64(BASE_MASK);
    PyLongObject *one = pylong_from_int64(1);

    PyLongObject *sum_mag = pylong_add_magnitude(max_digit, one);
    ASSERT_EQ_INT(Py_SIZE(sum_mag), 2);
    ASSERT_EQ_INT(sum_mag->ob_digit[0], 0);
    ASSERT_EQ_INT(sum_mag->ob_digit[1], 1);
    ASSERT_TRUE(pylong_verify_invariants(sum_mag));

    PyLongObject *diff_mag = pylong_sub_magnitude(sum_mag, one);
    ASSERT_EQ_INT(Py_SIZE(diff_mag), 1);
    ASSERT_EQ_INT(diff_mag->ob_digit[0], BASE_MASK);
    ASSERT_TRUE(pylong_verify_invariants(diff_mag));

    PyLongObject *zero_res = pylong_sub_magnitude(max_digit, max_digit);
    ASSERT_EQ_INT(Py_SIZE(zero_res), 0);
    ASSERT_TRUE(pylong_verify_invariants(zero_res));

    pylong_free(max_digit);
    pylong_free(one);
    pylong_free(sum_mag);
    pylong_free(diff_mag);
    pylong_free(zero_res);
}

/*
 * Bignum Layer 5 Tests: Signed Arithmetic
 */
static void test_layer5_signed_arithmetic(void) {
    int64_t res = 0;
    PyLongObject *a100 = pylong_from_int64(100);
    PyLongObject *a200 = pylong_from_int64(200);
    PyLongObject *an50 = pylong_from_int64(-50);
    PyLongObject *zero = pylong_from_int64(0);

    PyLongObject *s1 = pylong_add(a100, a200);
    ASSERT_EQ_INT(pylong_to_int64(s1, &res), 0);
    ASSERT_EQ_INT(res, 300);
    ASSERT_TRUE(pylong_verify_invariants(s1));

    PyLongObject *s2 = pylong_add(a100, an50);
    ASSERT_EQ_INT(pylong_to_int64(s2, &res), 0);
    ASSERT_EQ_INT(res, 50);
    ASSERT_TRUE(pylong_verify_invariants(s2));

    PyLongObject *s3 = pylong_add(an50, a100);
    ASSERT_EQ_INT(pylong_to_int64(s3, &res), 0);
    ASSERT_EQ_INT(res, 50);

    PyLongObject *s4 = pylong_add(an50, an50);
    ASSERT_EQ_INT(pylong_to_int64(s4, &res), 0);
    ASSERT_EQ_INT(res, -100);

    PyLongObject *an100 = pylong_from_int64(-100);
    PyLongObject *s5 = pylong_add(a100, an100);
    ASSERT_EQ_INT(Py_SIZE(s5), 0);
    ASSERT_EQ_INT(pylong_to_int64(s5, &res), 0);
    ASSERT_EQ_INT(res, 0);
    ASSERT_TRUE(pylong_verify_invariants(s5));

    PyLongObject *s6 = pylong_add(a100, zero);
    ASSERT_EQ_INT(pylong_to_int64(s6, &res), 0);
    ASSERT_EQ_INT(res, 100);

    PyLongObject *d1 = pylong_sub(a100, a200);
    ASSERT_EQ_INT(pylong_to_int64(d1, &res), 0);
    ASSERT_EQ_INT(res, -100);

    PyLongObject *d2 = pylong_sub(a100, an50);
    ASSERT_EQ_INT(pylong_to_int64(d2, &res), 0);
    ASSERT_EQ_INT(res, 150);

    pylong_free(a100);
    pylong_free(a200);
    pylong_free(an50);
    pylong_free(an100);
    pylong_free(zero);
    pylong_free(s1);
    pylong_free(s2);
    pylong_free(s3);
    pylong_free(s4);
    pylong_free(s5);
    pylong_free(s6);
    pylong_free(d1);
    pylong_free(d2);
}

/*
 * Bignum Layer 6 Tests: Multiplication
 */
static void test_layer6_multiplication(void) {
    int64_t res = 0;
    PyLongObject *z = pylong_from_int64(0);
    PyLongObject *n123 = pylong_from_int64(123);
    PyLongObject *n456 = pylong_from_int64(456);
    PyLongObject *neg5 = pylong_from_int64(-5);
    PyLongObject *neg6 = pylong_from_int64(-6);

    PyLongObject *m0 = pylong_mul(z, n123);
    ASSERT_EQ_INT(Py_SIZE(m0), 0);
    ASSERT_TRUE(pylong_verify_invariants(m0));

    PyLongObject *m1 = pylong_mul(n123, n456);
    ASSERT_EQ_INT(pylong_to_int64(m1, &res), 0);
    ASSERT_EQ_INT(res, 56088);
    ASSERT_TRUE(pylong_verify_invariants(m1));

    PyLongObject *pos6 = pylong_from_int64(6);
    PyLongObject *m2 = pylong_mul(neg5, pos6);
    ASSERT_EQ_INT(pylong_to_int64(m2, &res), 0);
    ASSERT_EQ_INT(res, -30);

    PyLongObject *m3 = pylong_mul(neg5, neg6);
    ASSERT_EQ_INT(pylong_to_int64(m3, &res), 0);
    ASSERT_EQ_INT(res, 30);

    PyLongObject *dmax = pylong_from_int64(BASE_MASK);
    PyLongObject *sq = pylong_mul(dmax, dmax);
    ASSERT_EQ_INT(Py_SIZE(sq), 2);
    ASSERT_EQ_INT(sq->ob_digit[0], 1);
    ASSERT_EQ_INT(sq->ob_digit[1], (BASE_MASK - 1));
    ASSERT_TRUE(pylong_verify_invariants(sq));

    PyLongObject *pow2_32 = pylong_from_int64(1LL << 32);
    PyLongObject *pow2_64 = pylong_mul(pow2_32, pow2_32);
    ASSERT_EQ_INT(Py_SIZE(pow2_64), 3);
    ASSERT_EQ_INT(pow2_64->ob_digit[0], 0);
    ASSERT_EQ_INT(pow2_64->ob_digit[1], 0);
    ASSERT_EQ_INT(pow2_64->ob_digit[2], 16);
    ASSERT_TRUE(pylong_verify_invariants(pow2_64));

    pylong_free(z);
    pylong_free(n123);
    pylong_free(n456);
    pylong_free(neg5);
    pylong_free(neg6);
    pylong_free(pos6);
    pylong_free(dmax);
    pylong_free(pow2_32);
    pylong_free(m0);
    pylong_free(m1);
    pylong_free(m2);
    pylong_free(m3);
    pylong_free(sq);
    pylong_free(pow2_64);
}

/*
 * Bignum Layer 7 Tests: Two's Complement Inversion
 */
static void test_layer7_twos_complement(void) {
    int64_t res = 0;

    PyLongObject *v5 = pylong_from_int64(5);
    PyLongObject *inv5 = pylong_invert(v5);
    ASSERT_EQ_INT(pylong_to_int64(inv5, &res), 0);
    ASSERT_EQ_INT(res, -6);
    ASSERT_TRUE(pylong_verify_invariants(inv5));

    PyLongObject *vn5 = pylong_from_int64(-5);
    PyLongObject *invn5 = pylong_invert(vn5);
    ASSERT_EQ_INT(pylong_to_int64(invn5, &res), 0);
    ASSERT_EQ_INT(res, 4);
    ASSERT_TRUE(pylong_verify_invariants(invn5));

    PyLongObject *v0 = pylong_from_int64(0);
    PyLongObject *inv0 = pylong_invert(v0);
    ASSERT_EQ_INT(pylong_to_int64(inv0, &res), 0);
    ASSERT_EQ_INT(res, -1);

    PyLongObject *vn1 = pylong_from_int64(-1);
    PyLongObject *invn1 = pylong_invert(vn1);
    ASSERT_EQ_INT(pylong_to_int64(invn1, &res), 0);
    ASSERT_EQ_INT(res, 0);
    ASSERT_EQ_INT(Py_SIZE(invn1), 0);

    pylong_free(v5);
    pylong_free(inv5);
    pylong_free(vn5);
    pylong_free(invn5);
    pylong_free(v0);
    pylong_free(inv0);
    pylong_free(vn1);
    pylong_free(invn1);
}

int main(void) {
    TEST_SUITE_START();

    /* Phase 1: PyObject core & reference counting */
    RUN_TEST(test_phase1_pyobject_refcount_and_dealloc);

    /* Phase 2: PyFloatObject & polymorphic dynamic dispatch */
    RUN_TEST(test_phase2_pyfloat_and_dispatch);

    /* Phase 3: PyLongObject polymorphic dynamic dispatch */
    RUN_TEST(test_phase3_pylong_polymorphic_dispatch);

    /* Bignum algorithmic layers */
    RUN_TEST(test_layer1_alloc_and_clamp);
    RUN_TEST(test_layer2_conversions);
    RUN_TEST(test_layer3_comparisons);
    RUN_TEST(test_layer4_magnitude_arithmetic);
    RUN_TEST(test_layer5_signed_arithmetic);
    RUN_TEST(test_layer6_multiplication);
    RUN_TEST(test_layer7_twos_complement);

    TEST_SUITE_END();
}
