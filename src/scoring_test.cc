#include "scoring.h"

#include "test_stat.h"
#include "test_data.h"
#include "input_manager.h"

namespace elfin {

namespace scoring {

/* test data */
V3fList const points10a = {
    {4.7008892286345, 42.938597096873, 14.4318130193692},
    { -20.3679194392227, 27.5712678608402, -12.1390617339732},
    {24.4692807074156, -1.32083675968276, 31.1580458282477},
    { -31.1044984967455, -6.41414114190809, 3.28255887994549},
    {18.6775433365315, -5.32162505701938, -14.9272896423117},
    { -31.648884426273, -19.3650527983443, 43.9001561999887},
    { -13.1515403509663, 0.850865538112699, 37.5942811492984},
    {12.561856072969, 1.07715641721097, 5.01563428984222},
    {28.0227435151377, 31.7627708322262, 12.2475086001227},
    { -41.8874231134215, 29.4831416883453, 8.70447045314168},
};

V3fList points10b = {
    { -29.2257707266972, -18.8897713349587, 9.48960740086143},
    { -19.8753669720509, 42.3379642103244, -23.7788252219155},
    { -2.90766514824093, -6.9792608670416, 10.2843089382083},
    { -26.9511839788441, -31.5183679875864, 21.1215780433683},
    {34.4308792695389, 40.4880968679893, -27.825326598276},
    { -30.5235710432951, 47.9748378356085, -38.2582349144194},
    { -27.4078219027601, -6.11300268738968, -20.3324126781673},
    { -32.9291952852141, -38.8880776559401, -18.1221698074118},
    { -27.2335702183446, -24.1935304087933, -7.58332402861928},
    { -6.43013158961009, -9.12801538874479, 0.785828466111815},
};

V3fList const points10ab_rot = {
    {0.523673403299203, -0.276948392922051, -0.805646171923458},
    { -0.793788382691122, -0.501965361762521, -0.343410511043611},
    { -0.309299482996081, 0.819347522879342, -0.482704326238996},
};

Vector3f const points10ab_tran {
    -1.08234396236629,
    5.08395199432057,
    -13.0170407784248
};

/* tests */
TestStat test_basics() {
    TestStat ts;

    // kabsch() return variables
    elfin::Mat3f rot;
    Vector3f tran;
    float rms;

    // Test that kabsch computation doesn't fail TRACE assertions.
    {
        ts.tests++;
        _rosetta_kabsch_align(points10a, points10b, rot, tran, rms);
    }

    // Test kabsch() rotation.
    {
        ts.tests++;
        for (size_t i = 0; i < 3; i++) {
            auto const& row = rot[i];
            Vector3f row_vec(row);
            if (not row_vec.is_approx(points10ab_rot[i])) {
                ts.errors++;
                JUtil.error("Rotation test failed: "
                            "row %zu does not approximate actual rotation row.\n"
                            "Expeced: %s\nGot: %s\n",
                            i,
                            points10ab_rot[i].to_string().c_str(),
                            row_vec.to_string().c_str());
                break;
            }
        }
    }

    // Test kabsch() translation.
    {
        ts.tests++;
        if (not tran.is_approx(points10ab_tran)) {
            ts.errors++;
            JUtil.error("Translation test failed: "
                        "does not approximate actual translation.\n"
                        "Expected: %s\nGot: %s\n",
                        points10ab_tran.to_string().c_str(),
                        tran.to_string().c_str());
        }
    }

    return ts;
}

TestStat test_resample() {
    TestStat ts;

    // Test upsampling a_fewer to B.size()
    {
        V3fList a_fewer(points10a);

        // Erase half of the points.
        a_fewer.erase(begin(a_fewer) + (a_fewer.size() / 2),
                      begin(a_fewer) + (a_fewer.size() / 2) + 1);
        assert(a_fewer.size() != points10a.size());

        a_fewer = _resample(points10a, a_fewer);

        ts.tests++;
        if (a_fewer.size() != points10a.size()) {
            ts.errors++;
            JUtil.error("Upsampling failed.\nSizes: a_fewer=%zu points10a=%zu\n",
                        a_fewer.size(), points10a.size());
        }
    }

    return ts;
}

TestStat test_score() {
    TestStat ts;

    // Test identical points Kabsch score == 0
    {
        {
            ts.tests++;
            float const score = score_aligned(points10a, points10a);
            if (not scoring::almost_eq(score, 0)) {
                ts.errors++;
                JUtil.error("Kabsch aligned score failed to produce 0 for"
                            " identical points.\n"
                            "Got %f\n", score);
            }
        }

        {
            ts.tests++;
            float const score = score_unaligned(points10a, points10a);
            if (not scoring::almost_eq(score, 0)) {
                ts.errors++;
                JUtil.error("Kabsch unaligned score failed to produce 0 for"
                            " identical points.\n"
                            "Got %f\n", score);
            }
        }
    }

    // Test unrelated point Kabsch score > 0
    {
        ts.tests++;
        auto const print_points = [&]() {
            std::ostringstream a_oss;
            a_oss << "points10a:\n";
            for (auto const& point : points10a) {
                a_oss << point.to_string() << "\n";
            }
            JUtil.error(a_oss.str().c_str());

            std::ostringstream b_oss;
            b_oss << "points10b:\n";
            for (auto const& point : points10b) {
                b_oss << point.to_string() << "\n";
            }

            JUtil.error(b_oss.str().c_str());
        };

        {
            ts.tests++;
            float const score = score_aligned(points10a, points10b);
            if (score < 1.0) {
                ts.errors++;
                JUtil.error("Kabsch aligned score failed to produce > 1.0 for"
                            " unrelated points.\n"
                            "Expected >> 0\nGot %f\n", score);
                print_points();
            }
        }

        {
            ts.tests++;
            float const score = score_unaligned(points10a, points10b);
            if (score < 1.0) {
                ts.errors++;
                JUtil.error("Kabsch unaligned score failed to produce > 1.0 for"
                            " unrelated points.\n"
                            "Expected >> 0\nGot %f\n", score);
                print_points();
            }
        }
    }

    // Set up test fragment for repeating similar tests.
    InputManager::setup_test({
        "--spec_file",
        "examples/quarter_snake_free.json"
    });
    Spec const spec(OPTIONS);

    TRACE_NOMSG(spec.work_packages().size() != 1);
    auto& [wp_name, wp] = *begin(spec.work_packages());

    TRACE_NOMSG(wp->work_areas().size() != 1);
    auto& wa = *begin(wp->work_areas());

    auto const& [fwd_ui_key, fwd_input_points] = *begin(wa->path_map);
    auto const& [bwd_ui_key, bwd_input_points] = *(++begin(wa->path_map));

    auto score_test_fragment =
        [&](V3fList const & test_points,
            std::string const & err_msg,
    float (*scoring_func)(V3fList const&, V3fList const&) = score_aligned) {
        ts.tests++;

        float const kscore =
            std::min(scoring_func(test_points, fwd_input_points),
                     scoring_func(test_points, bwd_input_points));

        if (not scoring::almost_eq(kscore, 0)) {
            ts.errors++;

            JUtil.error((err_msg + "Expected 0\nGot %f\n").c_str(), kscore);
            {
                std::ostringstream oss;
                oss << "Expected points (hardcoded):\n";
                for (auto const& point : test_points) {
                    oss << point.to_string() << "\n";
                }
                JUtil.error(oss.str().c_str());
            }

            {
                std::ostringstream oss;
                oss << "Input file points:\n";
                for (auto const& point : fwd_input_points) {
                    oss << point.to_string() << "\n";
                }

                JUtil.error(oss.str().c_str());
            }
        }
    };

    // Identity (no transform) score 0.
    score_test_fragment(tests::QUARTER_SNAKE_FREE_COORDINATES,
                        "Kabsch identity aligned score test failed.\n");
    score_test_fragment(tests::QUARTER_SNAKE_FREE_COORDINATES,
                        "Kabsch identity unaligned score test failed.\n",
                        /*scoring_func=*/score_unaligned);

    // Test translation score 0.
    {
        Transform trans_tx({
            {   "rot", {
                    {1, 0, 0},
                    {0, 1, 0},
                    {0, 0, 1}
                }
            },
            {"tran", { -7.7777, -30, 150.12918}}
        });

        V3fList test_points = tests::QUARTER_SNAKE_FREE_COORDINATES;
        for (auto& point : test_points) {
            point = trans_tx * point;
        }

        score_test_fragment(test_points,
                            "Kabsch translation score test failed.\n");
    }

    // Test rotation score 0.
    {
        Transform rot_tx({
            {   "rot", {
                    {0.28878074884414673, -0.9471790194511414, -0.13949079811573029},
                    { -0.5077904462814331, -0.27504783868789673, 0.8163931369781494},
                    { -0.8116370439529419, -0.16492657363414764, -0.5603969693183899}
                }
            },
            {"tran", {0, 0, 0}}
        });

        V3fList test_points = tests::QUARTER_SNAKE_FREE_COORDINATES;
        for (auto& point : test_points) {
            point = rot_tx * point;
        }

        score_test_fragment(test_points,
                            "Kabsch rotation score test failed.\n");
    }

    // Test random transformation score 0.
    {
        // This tx is produced by taking the matrix_world of a transformed
        // Blender object.
        Transform random_tx({
            {   "rot", {
                    {0.2617338001728058, 0.08983021974563599, 0.9609506130218506},
                    {0.9230813384056091, 0.26742106676101685, -0.27641811966896057},
                    { -0.2818091809749603, 0.959383487701416, -0.012927504256367683}
                }
            },
            {"tran", {3.15165638923645, -5.339916229248047, 3.290015935897827}}
        });

        V3fList test_points = tests::QUARTER_SNAKE_FREE_COORDINATES;
        for (auto& point : test_points) {
            point = random_tx * point;
        }

        score_test_fragment(test_points,
                            "Kabsch random transform score test failed.\n");
    }

    // Test Blender origin transform score 0.
    score_test_fragment(tests::QUARTER_SNAKE_FREE_COORDINATES_ORIGIN,
                        "Kabsch Blender origin transform score test failed.\n");

    return ts;
}

TestStat test() {
    TestStat ts;

    ts += test_basics();
    ts += test_resample();
    ts += test_score();

    return ts;
}

}  /* kabsch */

}  /* elfin */