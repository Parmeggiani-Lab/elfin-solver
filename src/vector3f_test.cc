#include "vector3f.h"

#include "test_stat.h"


namespace elfin {

/* test data */
Vector3f const vec_a {10.123, -209.9382, 9.00002};
Vector3f const vec_a_mult_pi {31.80234243, -659.54030683, 28.27439671};
Vector3f const vec_a_plus_pi {13.26459265, -206.79660735, 12.14161265};
Vector3f const vec_a_minus_3pi {0.69822204, -219.36297796, -0.42475796};

/* tests */
TestStat Vector3f::test() {
    TestStat ts;
    
    // Expected results
    float const pi = 3.141592653589793;
    float const a_norm = 210.37472117210373;
    float const a_sq_norm = 44257.523308240394;

    // Test Vector3f * float
    {
        Vector3f const a_mult_pi_test = vec_a * pi;
        ts.tests++;
        if (not a_mult_pi_test.is_approx(vec_a_mult_pi)) {
            ts.errors++;
            JUtil.error("vec_a(%s) * PI should be (%s) but got (%s)\n",
                vec_a.to_string().c_str(),
                vec_a_mult_pi.to_string().c_str(),
                a_mult_pi_test.to_string().c_str());
        }
    }

    // Test Vector3f + Vector3f.
    {
        Vector3f const a_plus_pi_test = vec_a + Vector3f(pi, pi, pi);
        ts.tests++;
        if (not a_plus_pi_test.is_approx(vec_a_plus_pi)) {
            ts.errors++;
            JUtil.error("vec_a(%s) + (PI, PI, PI) should be (%s) but got (%s)\n",
                vec_a.to_string().c_str(),
                vec_a_plus_pi.to_string().c_str(),
                a_plus_pi_test.to_string().c_str());
        }
    }

    // Test Vector3f - float * Vector3f.
    {
        Vector3f const a_minus_3pi_test = vec_a - 3 * Vector3f(pi, pi, pi);
        ts.tests++;
        if (not a_minus_3pi_test.is_approx(vec_a_minus_3pi)) {
            ts.errors++;
            JUtil.error("vec_a(%s) - 3*(PI, PI, PI) should be (%s) but got (%s)\n",
                vec_a.to_string().c_str(),
                vec_a_minus_3pi.to_string().c_str(),
                a_minus_3pi_test.to_string().c_str());
        }
    }

    // Test norm.
    {
        float const a_norm_test = vec_a.dist_to(Vector3f());
        ts.tests++;
        if (not JUtil.float_approximates(a_norm_test, a_norm, 1e-8)) {
            ts.errors++;
            JUtil.error("vec_a norm should be %f but got %f\n",
                a_norm, a_norm_test);
        }
    }

    // Test squared norm.
    {
        float const a_sq_norm_test = vec_a.sq_dist_to(Vector3f());
        ts.tests++;
        if (not JUtil.float_approximates(a_sq_norm_test, a_sq_norm, 1e-8)) {
            ts.errors++;
            JUtil.error("vec_a norm should be %f but got %f\n",
                a_sq_norm, a_sq_norm_test);
        }
    }

    return ts;
}

}  /* elfin */