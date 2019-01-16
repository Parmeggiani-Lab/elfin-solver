#ifndef ELFIN_H_
#define ELFIN_H_

namespace elfin {

class Elfin {
public:
    /* ctors */
    Elfin(int const argc, const char ** argv);

    /* dtors */
    ~Elfin();

    /* modifiers */
    void run();

    /* signal handlers */
    static void interrupt_handler(int const signal);
};

} /* elfin */

#endif /* end of include guard: ELFIN_H_ */