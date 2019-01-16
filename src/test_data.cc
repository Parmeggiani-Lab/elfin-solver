#include "test_data.h"

namespace elfin {

namespace tests {

V3fList const QUARTER_SNAKE_FREE_COORDINATES = {
    { -40.844879150390625, -42.66680717468262, 7.421332597732544},
    { -4.097459316253662, -37.30506420135498, 18.16762089729309},
    {12.520357370376587, -50.98127365112305, 13.686529397964478},
    {21.753182411193848, -63.435373306274414, -1.899409294128418},
    {26.63567066192627, -57.53522872924805, -29.187021255493164},
    {54.139976501464844, -23.92446994781494, -35.15853404998779},
    {82.54196166992188, 3.187546730041504, -44.66012477874756}
};

V3fList const QUARTER_SNAKE_FREE_COORDINATES_ORIGIN = {
    { -2.384185791015625e-06, -4.76837158203125e-06, 5.960464477539062e-07},
    { -6.416528224945068, -34.18703556060791, 16.872470378875732},
    { -24.268782138824463, -39.349234104156494, 28.61638069152832},
    { -46.18825912475586, -37.89566516876221, 29.449806213378906},
    { -65.34646987915039, -42.002596855163574, 8.971515893936157},
    { -65.39548873901367, -82.83100128173828, -6.993079781532288},
    { -71.71829223632812, -120.26065826416016, -20.81545352935791}
};

Recipe const QUARTER_SNAKE_FREE_RECIPE {
    {"D79_aC2_04", TermType::C, "B", "A", "D79_aC2_04"},
    {"D79", TermType::C, "A", "A", "D79.001"},
    {"D79", TermType::C, "A", "A", "D79.002"},
    {"D79", TermType::C, "A", "A", "D79.003"},
    {"D79_j1_D54", TermType::C, "A", "A", "D79_j1_D54.001"},
    {"D54_j1_D79", TermType::C, "A", "A", "D54_j1_D79.003"},
    {"D79_j2_D14", TermType::C, "?", "?", "D79_j2_D14.003"}
};

Recipe const HALF_SNAKE_FREE_RECIPE {
    {"D54_j1_D79", TermType::N, "A", "A", "D54_j1_D79"},
    {"D54", TermType::N, "A", "A", "D54"},
    {"D14_j1_D54", TermType::N, "A", "A", "D14_j1_D54"},
    {"D14_j2_D14", TermType::N, "A", "A", "D14_j2_D14.001"},
    {"D79_j2_D14", TermType::N, "A", "A", "D79_j2_D14.002"},
    {"D79_aC2_04", TermType::C, "B", "A", "D79_aC2_04"},
    {"D79", TermType::C, "A", "A", "D79.001"},
    {"D79", TermType::C, "A", "A", "D79.002"},
    {"D79", TermType::C, "A", "A", "D79.003"},
    {"D79_j1_D54", TermType::C, "A", "A", "D79_j1_D54.001"},
    {"D54_j1_D79", TermType::C, "A", "A", "D54_j1_D79.003"},
    {"D79_j2_D14", TermType::C, "?", "?", "D79_j2_D14.003"}
};

Recipe const H_1H_RECIPE {
    {"D49_aC2_ext", TermType::N, "D", "A", "D49_aC2_ext.002"},
    {"D49", TermType::N, "A", "A", "D49.012"},
    {"D49", TermType::N, "A", "A", "D49.015"},
    {"D49", TermType::N, "A", "A", "D49.016"},
    {"D49", TermType::N, "?", "?", "D49.019"}
};

Recipe const HALF_SNAKE_1H_RECIPE {
    {"D79_j2_D14", TermType::C, "A", "A", "D79_j2_D14.003"},
    {"D54_j1_D79", TermType::C, "A", "A", "D54_j1_D79.003"},
    {"D79_j1_D54", TermType::C, "A", "A", "D79_j1_D54.001"},
    {"D79", TermType::C, "A", "A", "D79.003"},
    {"D79", TermType::C, "A", "A", "D79.002"},
    {"D79", TermType::C, "B", "A", "D79.001"},
    {"D79_aC2_04", TermType::N, "A", "A", "D79_aC2_04"},
    {"D79_j2_D14", TermType::N, "A", "A", "D79_j2_D14.002"},
    {"D14_j2_D14", TermType::N, "A", "A", "D14_j2_D14.001"},
    {"D14_j1_D54", TermType::N, "A", "A", "D14_j1_D54"},
    {"D54", TermType::N, "A", "A", "D54"},
    {"D54_j1_D79", TermType::N, "?", "?", "D54_j1_D79"}
};

Recipe const H_2H_RECIPE {
    {"D49_aC2_ext", TermType::C, "D", "A", "D49_aC2_ext.001"},
    {"D49", TermType::C, "A", "A", "D49.007"},
    {"D49", TermType::C, "A", "A", "D49.005"},
    {"D49", TermType::C, "A", "A", "D49.004"},
    {"D49", TermType::C, "A", "D", "D49.003"},
    {"D49_aC2_ext", TermType::N, "C", "A", "D49_aC2_ext"},
    {"D49", TermType::C, "?", "?", "D49"}
};

// Reversed version
Recipe const H_2H_RECIPE_REV {
    {"D49", TermType::C, "A", "C", "D49"},
    {"D49_aC2_ext", TermType::N, "D", "A", "D49_aC2_ext"},
    {"D49", TermType::N, "A", "A", "D49.003"},
    {"D49", TermType::N, "A", "A", "D49.004"},
    {"D49", TermType::N, "A", "A", "D49.005"},
    {"D49", TermType::N, "A", "D", "D49.007"},
    {"D49_aC2_ext", TermType::N, "?", "?", "D49_aC2_ext.001"},
};

}  /* tests */

}  /* elfin */