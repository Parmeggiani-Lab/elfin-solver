#ifndef TEST_CONSTS_H_
#define TEST_CONSTS_H_

#include "geometry.h"
#include "recipe.h"

namespace elfin {

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

/* Recipes */
static Recipe const quarter_snake_free_recipe {
    {"D79_aC2_04", TerminusType::C, 1},
    {"D79", TerminusType::C, 0},
    {"D79", TerminusType::C, 0},
    {"D79", TerminusType::C, 0},
    {"D79_j1_D54", TerminusType::C, 0},
    {"D54_j1_D79", TerminusType::C, 0},
    {"D79_j2_D14", TerminusType::C, 0},
};

}  /* elfin */

#endif  /* end of include guard: TEST_CONSTS_H_ */