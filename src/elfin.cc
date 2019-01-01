/* Copyright 2018 Joy Yeh <joyyeh@gmail.com> */

#include "elfin.h"

#include <csignal>

#include "input_manager.h"
#include "output_manager.h"
#include "parallel_utils.h"
#include "tests.h"
#include "exit_exception.h"

namespace elfin {

Elfin::InstanceMap Elfin::instances_;

/* public */
/* ctors */
Elfin::Elfin(int const argc, char const** argv) {
    instances_.insert(this);

    std::signal(SIGINT, interrupt_handler);

    // Display all info + warnings by default.
    JUtil.set_log_lvl(LOGLVL_INFO);

    // Parse arguments and configuration.
    InputManager::parse_options(argc, argv);
}

/* dtors */
Elfin::~Elfin() {
    instances_.erase(this);
}

/* modifiers */
void Elfin::run() {
    if (OPTIONS.run_tests) {
        tests::run_all();
    }
    else {
        InputManager::setup();
        solver_.run();
        OutputManager::write_output(solver_);
    }
}

/* private */
/* accessors */
void Elfin::crash_dump() const {
    JUtil.warn("Crash-dumping results...\n");
    OutputManager::write_output(solver_, "crash_dump");
}

/* handlers */
void Elfin::interrupt_handler(int const signal) {
    static bool interrupt_caught = false;

    if (interrupt_caught) {
        fprintf(stderr, "\n\n");
        PANIC("Caught interrupt signal (second). Aborting NOW.\n");
    }
    else {
        interrupt_caught = true;

        fprintf(stderr, "\n\n");
        JUtil.warn("Caught interrupt signal (first); trying to save data...\n");

        // Save latest results
        for (auto inst : instances_) {
            inst->crash_dump();
        }

        throw ExitException{signal};
    }
}


}  // namespace elfin

int main(int const argc, const char ** argv) {
    try {
        elfin::Elfin(argc, argv).run();
        return 0;
    }
    catch (elfin::ExitException e) {
        if (e.code) {
            JUtil.error("Aborting due to none zero code: %d\n", e.code);
        }
        return e.code;
    }
}