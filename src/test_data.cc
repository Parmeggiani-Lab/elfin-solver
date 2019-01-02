#include "test_data.h"

namespace elfin {

namespace tests {

V3fList const QUARTER_SNAKE_FREE_COORDINATES = {
    { -40.84488391876221, -42.66680717468262, 7.421331405639648},
    { -4.097461700439453, -37.30506181716919, 18.167617321014404},
    {12.520356178283691, -50.98127365112305, 13.686530590057373},
    {21.753182411193848, -63.435373306274414, -1.8994081020355225},
    {26.63567066192627, -57.53522872924805, -29.187021255493164},
    {54.139981269836426, -23.92446756362915, -35.15852928161621},
    {82.54197120666504, 3.187551498413086, -44.66012001037598}
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
    {"D79_aC2_04", TerminusType::C, "B", "A", "D79_aC2_04"},
    {"D79", TerminusType::C, "A", "A", "D79.001"},
    {"D79", TerminusType::C, "A", "A", "D79.002"},
    {"D79", TerminusType::C, "A", "A", "D79.003"},
    {"D79_j1_D54", TerminusType::C, "A", "A", "D79_j1_D54.001"},
    {"D54_j1_D79", TerminusType::C, "A", "A", "D54_j1_D79.003"},
    {"D79_j2_D14", TerminusType::C, "?", "?", "D79_j2_D14.003"}
};

Recipe const HALF_SNAKE_FREE_RECIPE {
    {"D54_j1_D79", TerminusType::N, "A", "A", "D54_j1_D79"},
    {"D54", TerminusType::N, "A", "A", "D54"},
    {"D14_j1_D54", TerminusType::N, "A", "A", "D14_j1_D54"},
    {"D14_j2_D14", TerminusType::N, "A", "A", "D14_j2_D14.001"},
    {"D79_j2_D14", TerminusType::N, "A", "A", "D79_j2_D14.002"},
    {"D79_aC2_04", TerminusType::C, "B", "A", "D79_aC2_04"},
    {"D79", TerminusType::C, "A", "A", "D79.001"},
    {"D79", TerminusType::C, "A", "A", "D79.002"},
    {"D79", TerminusType::C, "A", "A", "D79.003"},
    {"D79_j1_D54", TerminusType::C, "A", "A", "D79_j1_D54.001"},
    {"D54_j1_D79", TerminusType::C, "A", "A", "D54_j1_D79.003"},
    {"D79_j2_D14", TerminusType::C, "?", "?", "D79_j2_D14.003"}
};

Recipe const H_1H_RECIPE {
    {"D49_aC2_ext", TerminusType::N, "D", "A", "D49_aC2_ext.002"},
    {"D49", TerminusType::N, "A", "A", "D49.012"},
    {"D49", TerminusType::N, "A", "A", "D49.015"},
    {"D49", TerminusType::N, "A", "A", "D49.016"},
    {"D49", TerminusType::N, "?", "?", "D49.019"}
};

Recipe const HALF_SNAKE_1H_RECIPE {
    {"D79_j2_D14", TerminusType::C, "A", "A", "D79_j2_D14.003"},
    {"D54_j1_D79", TerminusType::C, "A", "A", "D54_j1_D79.003"},
    {"D79_j1_D54", TerminusType::C, "A", "A", "D79_j1_D54.001"},
    {"D79", TerminusType::C, "A", "A", "D79.003"},
    {"D79", TerminusType::C, "A", "A", "D79.002"},
    {"D79", TerminusType::C, "B", "A", "D79.001"},
    {"D79_aC2_04", TerminusType::N, "A", "A", "D79_aC2_04"},
    {"D79_j2_D14", TerminusType::N, "A", "A", "D79_j2_D14.002"},
    {"D14_j2_D14", TerminusType::N, "A", "A", "D14_j2_D14.001"},
    {"D14_j1_D54", TerminusType::N, "A", "A", "D14_j1_D54"},
    {"D54", TerminusType::N, "A", "A", "D54"},
    {"D54_j1_D79", TerminusType::N, "?", "?", "D54_j1_D79"}
};

Recipe const H_2H_RECIPE {
    {"D49_aC2_ext", TerminusType::C, "D", "A", "D49_aC2_ext.001"},
    {"D49", TerminusType::C, "A", "A", "D49.007"},
    {"D49", TerminusType::C, "A", "A", "D49.005"},
    {"D49", TerminusType::C, "A", "A", "D49.004"},
    {"D49", TerminusType::C, "A", "D", "D49.003"},
    {"D49_aC2_ext", TerminusType::N, "C", "A", "D49_aC2_ext"},
    {"D49", TerminusType::C, "?", "?", "D49"}
};

// Reversed version
Recipe const H_2H_RECIPE_REV {
    {"D49", TerminusType::C, "A", "C", "D49"},
    {"D49_aC2_ext", TerminusType::N, "D", "A", "D49_aC2_ext"},
    {"D49", TerminusType::N, "A", "A", "D49.003"},
    {"D49", TerminusType::N, "A", "A", "D49.004"},
    {"D49", TerminusType::N, "A", "A", "D49.005"},
    {"D49", TerminusType::N, "A", "D", "D49.007"},
    {"D49_aC2_ext", TerminusType::N, "?", "?", "D49_aC2_ext.001"},
};

}  /* tests */

}  /* elfin */