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
        ts.tests++;
        Vector3f const a_mult_pi_test = vec_a * pi;
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
        ts.tests++;
        Vector3f const a_plus_pi_test = vec_a + Vector3f(pi, pi, pi);
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
        ts.tests++;
        Vector3f const a_minus_3pi_test = vec_a - 3 * Vector3f(pi, pi, pi);
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
        ts.tests++;
        float const a_norm_test = vec_a.dist_to(Vector3f());
        if (not JUtil.float_approximates(a_norm_test, a_norm, 1e-8)) {
            ts.errors++;
            JUtil.error("vec_a norm should be %f but got %f\n",
                        a_norm, a_norm_test);
        }
    }

    // Test squared norm.
    {
        ts.tests++;
        float const a_sq_norm_test = vec_a.sq_dist_to(Vector3f());
        if (not JUtil.float_approximates(a_sq_norm_test, a_sq_norm, 1e-8)) {
            ts.errors++;
            JUtil.error("vec_a norm should be %f but got %f\n",
                        a_sq_norm, a_sq_norm_test);
        }
    }

    // Test hash function.
    {
        ts.tests++;
        auto const a = std::hash<Vector3f>()(Vector3f(1, 2, 3));
        auto const b = std::hash<Vector3f>()(Vector3f(1, 3, 2));
        auto const c = std::hash<Vector3f>()(Vector3f(2, 1, 3));
        if (a == b or b == c or a == c) {
            ts.errors++;
            JUtil.error("Vector3f bad hash function: a=0x%x, b=0x%x, c=0x%x\n", a, b, c);
        }

        auto const d = std::hash<Vector3f>()(Vector3f(1, 2.5, 1));
        auto const f = std::hash<Vector3f>()(Vector3f(1, 2.5, 1));
        if (d != f) {
            ts.errors++;
            JUtil.error("Vector3f bad hash function: d=0x%x, f=0x%x\n", d, f);
        }

        auto const g = std::hash<Vector3f>()(Vector3f(0.2000002, 2.5, 1));
        auto const h = std::hash<Vector3f>()(Vector3f(0.2, 2.5, 1));
        if (g == h) {
            ts.errors++;
            JUtil.error("Vector3f bad hash function: g=0x%x, h=0x%x\n", g, h);
        }

        auto const i = std::hash<Vector3f>()(Vector3f(0, 2.599999, 0));
        auto const j = std::hash<Vector3f>()(Vector3f(0, 0, 2.599999));
        if (i == j) {
            ts.errors++;
            JUtil.error("Vector3f bad hash function: i=0x%x, j=0x%x\n", i, j);
        }
    }

    return ts;
}

}  /* elfin */