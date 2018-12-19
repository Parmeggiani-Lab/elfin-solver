#include "vector3f.h"

#include "debug_utils.h"

namespace elfin {

/* tests */
TestStat Vector3f::test() {
    TestStat ts;
    using floats = std::vector<float> const;

    Vector3f a(10.123, -209.9382, 9.00002);
    Vector3f a_mult_pi(31.80234243, -659.54030683, 28.27439671);
    Vector3f a_plus_pi(13.26459265, -206.79660735, 12.14161265);
    Vector3f a_minus_3pi(0.69822204, -219.36297796, -0.42475796);
    float const pi = 3.141592653589793;
    float const a_norm = 210.37472117210373;
    float const a_sq_norm = 44257.523308240394;

    /* Test Vector3f * float */
    Vector3f a_mult_pi_test = a * pi;
    ts.tests++;
    if (not a_mult_pi_test.is_approx(a_mult_pi)) {
        ts.errors++;
        err("a(%s) * PI should be (%s) but got (%s)\n",
            a.to_string().c_str(),
            a_mult_pi.to_string().c_str(),
            a_mult_pi_test.to_string().c_str());
    }

    /* Test Vector3f + Vector3f */
    Vector3f a_plus_pi_test = a + Vector3f(pi, pi, pi);
    ts.tests++;
    if (not a_plus_pi_test.is_approx(a_plus_pi)) {
        ts.errors++;
        err("a(%s) + (PI, PI, PI) should be (%s) but got (%s)\n",
            a.to_string().c_str(),
            a_plus_pi.to_string().c_str(),
            a_plus_pi_test.to_string().c_str());
    }

    /* Test Vector3f - float * Vector3f */
    Vector3f a_minus_3pi_test = a - 3 * Vector3f(pi, pi, pi);
    ts.tests++;
    if (not a_minus_3pi_test.is_approx(a_minus_3pi)) {
        ts.errors++;
        err("a(%s) - 3*(PI, PI, PI) should be (%s) but got (%s)\n",
            a.to_string().c_str(),
            a_minus_3pi.to_string().c_str(),
            a_minus_3pi_test.to_string().c_str());
    }

    /* Test norm */
    float const a_norm_test = a.dist_to(Vector3f());
    ts.tests++;
    if (not float_approximates_err(a_norm_test, a_norm, 1e-8)) {
        ts.errors++;
        err("a norm should be %f but got %f\n",
            a_norm, a_norm_test);
    }

    float const a_sq_norm_test = a.sq_dist_to(Vector3f());
    ts.tests++;
    if (not float_approximates_err(a_sq_norm_test, a_sq_norm, 1e-8)) {
        ts.errors++;
        err("a norm should be %f but got %f\n",
            a_sq_norm, a_sq_norm_test);
    }

    return ts;
}

}  /* elfin */