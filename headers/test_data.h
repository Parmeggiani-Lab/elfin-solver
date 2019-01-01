#ifndef TEST_DATA_H_
#define TEST_DATA_H_

#include "geometry.h"
#include "terminus_type.h"

namespace elfin {

namespace tests {

struct RecipeStep {
    std::string mod_name;
    TerminusType src_term = TerminusType::NONE;
    std::string src_chain;
    std::string dst_chain;
};
typedef std::vector<RecipeStep> Recipe;

/* default test input */
static V3fList const quarter_snake_free_coordinates = {
    { -40.84488391876221, -42.66680717468262, 7.421331405639648},
    { -4.097461700439453, -37.30506181716919, 18.167617321014404},
    {12.520356178283691, -50.98127365112305, 13.686530590057373},
    {21.753182411193848, -63.435373306274414, -1.8994081020355225},
    {26.63567066192627, -57.53522872924805, -29.187021255493164},
    {54.139981269836426, -23.92446756362915, -35.15852928161621},
    {82.54197120666504, 3.187551498413086, -44.66012001037598}
};

static V3fList const quarter_snake_free_coordinates_origin = {
    { -2.384185791015625e-06, -4.76837158203125e-06, 5.960464477539062e-07},
    { -6.416528224945068, -34.18703556060791, 16.872470378875732},
    { -24.268782138824463, -39.349234104156494, 28.61638069152832},
    { -46.18825912475586, -37.89566516876221, 29.449806213378906},
    { -65.34646987915039, -42.002596855163574, 8.971515893936157},
    { -65.39548873901367, -82.83100128173828, -6.993079781532288},
    { -71.71829223632812, -120.26065826416016, -20.81545352935791}
};

static Recipe const quarter_snake_free_recipe {
    {"D79_aC2_04", TerminusType::C, "B", "A"},
    {"D79", TerminusType::C, "A", "A"},
    {"D79", TerminusType::C, "A", "A"},
    {"D79", TerminusType::C, "A", "A"},
    {"D79_j1_D54", TerminusType::C, "A", "A"},
    {"D54_j1_D79", TerminusType::C, "A", "A"},
    {"D79_j2_D14", TerminusType::C, "A", "A"},
};

static Recipe const half_snake_free_recipe {
    {"D54_j1_D79", TerminusType::N, "A", "A"},
    {"D54", TerminusType::N, "A", "A"},
    {"D14_j1_D54", TerminusType::N, "A", "A"},
    {"D14_j2_D14", TerminusType::N, "A", "A"},
    {"D79_j2_D14", TerminusType::N, "A", "A"},
    {"D79_aC2_04", TerminusType::C, "B", "A"},
    {"D79", TerminusType::C, "A", "A"},
    {"D79", TerminusType::C, "A", "A"},
    {"D79", TerminusType::C, "A", "A"},
    {"D79_j1_D54", TerminusType::C, "A", "A"},
    {"D54_j1_D79", TerminusType::C, "A", "A"},
    {"D79_j2_D14", TerminusType::C, "A", "A"},
};

static Recipe const H_1h_recipe {
    {"D49_aC2_ext", TerminusType::N, "D", "A"},
    {"D49", TerminusType::N, "A", "A"},
    {"D49", TerminusType::N, "A", "A"},
    {"D49", TerminusType::N, "A", "A"},
    {"D49", TerminusType::N, "A", "A"}
};

}  /* tests */

}  /* elfin */

#endif  /* end of include guard: TEST_DATA_H_ */