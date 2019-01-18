#include "test_data.h"

namespace elfin {

namespace tests {

V3fList const QUARTER_SNAKE_FREE_COORDINATES = {
    {82.54197120666504, 3.187551498413086, -44.66012001037598},
    {54.139981269836426, -23.92446756362915, -35.15852928161621},
    {26.63567066192627, -57.53522872924805, -29.187021255493164},
    {21.753182411193848, -63.435373306274414, -1.8994081020355225},
    {12.520356178283691, -50.98127365112305, 13.686530590057373},
    { -4.097461700439453, -37.30506181716919, 18.167617321014404},
    { -41.04451656341553, -42.51813888549805, 7.601149082183838}
};

V3fList const QUARTER_SNAKE_FREE_COORDINATES_ORIGIN = {
    {0.0, 0.0, 0.0},
    { -23.528871536254883, -7.555261850357056, -31.95817470550537},
    { -44.80151176452637, -21.024374961853027, -67.84531116485596},
    { -72.77134418487549, -23.643441200256348, -64.09124374389648},
    { -87.70882606506348, -8.344345688819885, -58.984341621398926},
    { -94.58349227905273, 12.095375061035156, -63.253703117370605},
    { -98.03109169006348, 27.798352241516113, -98.54451179504395}
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
    {"D49_aC2_ext", TermType::N, "C", "A", "D49_aC2_ext.004"},
    {"D49", TermType::N, "A", "A", "D49.012"},
    {"D49", TermType::N, "A", "A", "D49.015"},
    {"D49", TermType::N, "A", "A", "D49.016"},
    {"D49_aC2_24", TermType::N, "?", "?", "D49_aC2_24.002"}
};

Recipe const H_1H_CHAIN_CHANGED_RECIPE {
    {"D49_aC2_ext", TermType::N, "D", "A", "D49_aC2_ext.004"},
    {"D49", TermType::N, "A", "A", "?"},
    {"D49", TermType::N, "A", "A", "?"},
    {"D49", TermType::N, "A", "A", "?"},
    {"D49_aC2_24", TermType::N, "?", "?", "D49_aC2_24.002"}
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
    {"D49_aC2_ext", TermType::C, "D", "A", "D49_aC2_ext.003"},
    {"D49", TermType::C, "A", "A", "?"},
    {"D49", TermType::C, "A", "A", "?"},
    {"D49", TermType::C, "A", "A", "?"},
    {"D49", TermType::C, "A", "D", "?"},
    {"D49_aC2_ext", TermType::N, "C", "A", "?"},
    {"D49", TermType::C, "?", "?", "D49"}
};

// Reversed version
Recipe const H_2H_RECIPE_REV {
    {"D49", TermType::C, "A", "C", "D49"},
    {"D49_aC2_ext", TermType::N, "D", "A", "?"},
    {"D49", TermType::N, "A", "A", "?"},
    {"D49", TermType::N, "A", "A", "?"},
    {"D49", TermType::N, "A", "A", "?"},
    {"D49", TermType::N, "A", "D", "?"},
    {"D49_aC2_ext", TermType::N, "?", "?", "D49_aC2_ext.003"},
};

}  /* tests */

}  /* elfin */