/* Copyright 2018 Joy Yeh <joyyeh@gmail.com> */

#include "elfin.h"

#include <csignal>

#include "input_manager.h"
#include "output_manager.h"
#include "tests.h"
#include "exceptions.h"

namespace elfin {

/* public */
/* ctors */
Elfin::Elfin(int const argc, char const** argv) {
    // Display all info + warnings by default.
    JUtil.set_log_lvl(LOGLVL_INFO);

    // Parse arguments and configuration.
    InputManager::parse(argc, argv);
}

/* dtors */
Elfin::~Elfin() {}

/* modifiers */
void Elfin::run() {
    if (OPTIONS.run_tests) {
        tests::run_all();
    }
    else {
        InputManager::setup();

        auto const& opt = InputManager::options();

        auto& spec = InputManager::spec();
        spec.solve_all();

        OutputManager(spec).write_to_file(opt);
    }
}

/* handlers */
void Elfin::interrupt_handler(int const signal) {
    static bool interrupt_caught = false;

    if (interrupt_caught) {
        fprintf(stderr, "\n\n");
        fprintf(stderr, "Caught interrupt signal (second). Aborting NOW.\n");
        throw ExitException(1, "Second Interupt Signal");
    }
    else {
        interrupt_caught = true;

        throw ExitException(1, "First Interupt Signal");
    }
}

}  // namespace elfin

int main(int const argc, const char ** argv) {
    try {
        std::signal(SIGINT, elfin::Elfin::interrupt_handler);
        elfin::Elfin(argc, argv).run();
        return 0;
    }
    catch (elfin::ExitException const& e) {
        if (e.code) {
            JUtil.warn("Abort (code=%d). Readon: %s\n", e.code, e.what());
        }
        return e.code;
    }
    catch (elfin::ElfinException const& e)
    {
        JUtil.error("Aborting. Reason: %s\n", e.what());
    }
    catch (std::exception const& e)
    {
        JUtil.error("Aborting due to general exception: %s\n", e.what());
    }
    catch (...)
    {
        JUtil.error("Aborting due to unknown exception.\n");
    }

    return 1;
}