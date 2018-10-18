#ifndef ELFIN_H_
#define ELFIN_H_

#include <vector>
#include <memory>

#include "../core/EvolutionSolver.h"
#include "input/spec.h"

namespace elfin {

class ElfinRunner
{
private:
    Options options_;
    std::shared_ptr<Spec> spec_;
    EvolutionSolver * es_;

    RelaMat rela_mat_;
    NameIdMap name_id_map_;
    IdNameMap id_name_map_;
    RadiiList radii_list_;

    bool es_started_ = false;
    static std::vector<ElfinRunner *> instances_;

    int run_unit_tests();
    int run_meta_tests();
    void crash_dump();
    static void interrupt_handler(const int signal);
public:
    ElfinRunner(const int argc, const char ** argv);
    void run();
};

} /* elfin */

#endif /* end of include guard: ELFIN_H_ */