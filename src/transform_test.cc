#include "transform.h"

#include "test_stat.h"
#include "test_data.h"

namespace elfin {

/* test data */
Transform const a_world( {
    {   "rot", {
            {0.05532475933432579, 0.017428744584321976, -0.08145802468061447},
            { -0.049986086785793304, 0.08517082780599594, -0.015726475045084953},
            {0.0666375458240509, 0.049418311566114426, 0.055832501500844955}
        }
    },
    {"tran", {8.054840087890625, 1.978692889213562, 3.4368598461151123}}
});

Transform const a_to_b( {
    {   "rot", {
            {0.6502372026443481, 0.3104279935359955, 0.6934162378311157},
            {0.756941020488739, -0.3428647220134735, -0.5563129186630249},
            {0.06505285948514938, 0.8866105675697327, -0.45791906118392944}
        }
    },
    {"tran", { -12.447150230407715, -10.713072776794434, -29.79046058654785}}
});

Transform const b_world( {
    {   "rot", {
            { -0.015099741518497467, 0.08121803402900696, 0.05635272338986397},
            { -0.016968388110399246, -0.058289747685194016, 0.07946307212114334},
            {0.0973862037062645, 0.0024365708231925964, 0.022582994773983955}
        }
    },
    {"tran", {10.415760040283203, 3.510263681411743, 5.347901344299316}}
});

Transform const c_to_a( {
    {   "rot", {
            {0.555465579032898, 0.8273841142654419, -0.08302728086709976},
            { -0.08186252415180206, 0.1537732481956482, 0.9847092628479004},
            {0.8275001645088196, -0.5401752591133118, 0.15314750373363495}
        }
    },
    {"tran", { -11.876138687133789, -42.8249397277832, 10.272198677062988}}
});

Transform const c_world( {
    {   "rot", {
            { -0.03810230642557144, 0.09245651960372925, 9.370781481266022e-05},
            { -0.047751497477293015, -0.01976567879319191, 0.08561023324728012},
            {0.07917074859142303, 0.03257473558187485, 0.0516805462539196}
        }
    },
    {"tran", {5.814658164978027, -1.2366465330123901, 1.102649211883545}}
});

/* tests */
TestStat Transform::test() {
    TestStat ts;

    // Frame Shift Tests

    // Test C-term extrude "raise".
    {
        Transform b_test = a_world * a_to_b.inversed();
        ts.tests++;
        if (not b_test.is_approx(b_world)) {
            ts.errors++;
            err("Frame raise test failed:\n"
                "b_test does not approximately equal to b_world\n");
            err("Expected b_world: %s\n", b_world.to_string().c_str());
            err("Got b_test: %s\n", b_test.to_string().c_str());
        }
    }

    // Test N-term extrude "drop".
    {
        Transform c_test = a_world * c_to_a;
        ts.tests++;
        if (not c_test.is_approx(c_world)) {
            ts.errors++;
            err("Frame drop test failed:\n"
                "c_test does not approximately equal to c_world\n");
            err("Expected c_world: %s\n", c_world.to_string().c_str());
            err("Got c_test: %s\n", c_test.to_string().c_str());
        }
    }

    return ts;
}

}  /* elfin */