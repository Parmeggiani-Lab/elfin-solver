#include "input_manager.h"

#include "test_stat.h"
#include "test_data.h"

namespace elfin {

void InputManager::setup_test(Args const& more_args) {
    Args args = test_args;
    args.insert(end(args), begin(more_args), end(more_args));
    parse(args, /*skip_xdb=*/true);
}

/* tests */
TestStat InputManager::test() {
    TestStat ts;

    // Test spec parsing.
    {
        ts.tests++;

        InputManager::setup_test({"--spec_file", "examples/quarter_snake_free.json"});
        Spec const spec(OPTIONS);


        auto const& wps = spec.work_packages();
        if (wps.size() != 1) {
            ts.errors++;
            JUtil.error("Spec parsing should get 1 Work Package but got %zu\n",
                        wps.size());
        }

        auto const &was = (*begin(wps))->work_areas();
        if (was.size() != 1) {
            ts.errors++;
            JUtil.error("Spec parsing should get 1 Work Area but got %zu\n",
                        was.size());
        }
        else {
            auto const& wa = *begin(was);

            // Test parsed points.
            TRACE_NOMSG(wa->path_map.size() != 2);
            auto pm_itr = begin(wa->path_map);
            auto const& [fwd_ui_key, fwd_test_points] = *pm_itr;
            auto const& [bwd_ui_key, bwd_test_points] = *(++pm_itr);

            auto const& expect_points = tests::QUARTER_SNAKE_FREE_COORDINATES;

            ts.tests++;
            if (fwd_test_points != expect_points and
                    bwd_test_points != expect_points ) {
                ts.errors++;
                JUtil.error("Work area point parsing test failed\n");
                std::ostringstream oss;
                oss << "Expected:\n";
                for (auto& p : tests::QUARTER_SNAKE_FREE_COORDINATES) {
                    oss << p.to_string() << "\n";
                }

                JUtil.error("But got:\n");
                for (auto& p : fwd_test_points) {
                    oss << p.to_string() << "\n";
                }

                JUtil.error(oss.str().c_str());
            }
        }
    }

    return ts;
}

}  /* elfin */