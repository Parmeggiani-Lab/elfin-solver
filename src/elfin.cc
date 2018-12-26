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

/* public */
/* ctors */
Elfin::Elfin(int const argc, char const** argv) {
    instances_.insert(this);

    std::signal(SIGINT, interrupt_handler);

    set_log_level(LOG_WARN);

    // Parse arguments and configuration
    InputManager::parse_options(argc, argv);
}

/* dtors */
Elfin::~Elfin() {
    instances_.erase(this);
}

/* modifiers */
int Elfin::run() {
    if (OPTIONS.run_tests) {
        tests::run_all();
    }
    else {
        init();
        solver_.run();
        OutputManager::write_output(solver_);
    }

    return 0;
}

/* private */
/* accessors */
void Elfin::crash_dump() const {
    wrn("Crash-dumping results...\n");
    OutputManager::write_output(solver_, "crash_dump");
}

void Elfin::init() const {
    InputManager::setup();
    parallel::init();
    random::init();
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


}  // namespace elfin
