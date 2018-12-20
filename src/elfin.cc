/* Copyright 2018 Joy Yeh <joyyeh@gmail.com> */

#include "elfin.h"

#include <csignal>

#include "input_manager.h"
#include "output_manager.h"
#include "parallel_utils.h"
#include "tests.h"

namespace elfin {

Elfin::InstanceMap Elfin::instances_;
bool interrupt_caught = false;

/* private */
/* accessors */
void Elfin::crash_dump() const {
    wrn("Crash-dumping results...\n");
    OutputManager::write_output(solver_, "crash_dump");
}

/* handlers */
void Elfin::interrupt_handler(int const signal) {
    if (interrupt_caught) {
        raw("\n\n");
        die("Caught interrupt signal (second). Aborting NOW.\n");
    }
    else {
        interrupt_caught = true;

        raw("\n\n");
        wrn("Caught interrupt signal (first); trying to save data...\n");

        // Save latest results
        for (auto inst : instances_) {
            inst->crash_dump();
        }

        exit(signal);
    }
}

/* public */
/* ctors */
Elfin::Elfin(int const argc, char const** argv) {
    instances_.insert(this);

    std::signal(SIGINT, interrupt_handler);

    set_log_level(LOG_WARN);

    // Parse arguments and configuration
    InputManager::setup(argc, argv);

    // Set up parallel utils after parsing number of threads into OPTIONS
    parallel::init();

    // Give per-thread Mersenne Twisters
    random::init();
}

/* dtors */
Elfin::~Elfin() {
    instances_.erase(this);
}

/* modifiers */
int Elfin::run() {

    if (OPTIONS.run_unit_tests) {
        tests::run_all();
    } else {
        solver_.run();
        OutputManager::write_output(solver_);
    }

    return 0;
}

}  // namespace elfin
