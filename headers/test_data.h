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
    {82.54196166992188, 3.187546730041504, -44.660125732421875},
    {54.139976501464844, -23.924468994140625, -35.15853500366211},
    {26.635669708251953, -57.53522872924805, -29.187021255493164},
    {21.75318145751953, -63.43537139892578, -1.899409294128418},
    {12.520357131958008, -50.98127365112305, 13.686529159545898},
    { -4.097459316253662, -37.3050651550293, 18.167621612548828},
    { -40.844879150390625, -42.66680908203125, 7.421332359313965}
};

static V3fList const quarter_snake_free_coordinates_origin = {
    { -71.71833038330078, -120.2606201171875, -20.815467834472656},
    { -65.39551544189453, -82.83096313476562, -6.993098735809326},
    { -65.34649658203125, -42.00255584716797, 8.971489906311035},
    { -46.18830871582031, -37.8956298828125, 29.449790954589844},
    { -24.26881980895996, -39.349205017089844, 28.61638641357422},
    { -6.4165496826171875, -34.18701934814453, 16.872482299804688},
    {1.430511474609375e-05, -1.430511474609375e-05, 4.76837158203125e-06}
};

static Recipe const quarter_snake_free_recipe {
    {"D79_aC2_04", TerminusType::C, "B", "A"},
    {"D79", TerminusType::C, "A",  "A"},
    {"D79", TerminusType::C, "A",  "A"},
    {"D79", TerminusType::C, "A",  "A"},
    {"D79_j1_D54", TerminusType::C, "A",  "A"},
    {"D54_j1_D79", TerminusType::C, "A",  "A"},
    {"D79_j2_D14", TerminusType::C, "A",  "A"},
};

}  /* tests */

}  /* elfin */

#endif  /* end of include guard: TEST_DATA_H_ */