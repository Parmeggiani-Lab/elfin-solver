#ifndef OUTPUT_MANAGER_H_
#define OUTPUT_MANAGER_H_

#include <string>
#include <memory>

#include "move_heap.h"
#include "options.h"

namespace elfin {

/* Fwd Decl */
class Spec;

class OutputManager {
private:
    /* types */
    struct PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;
public:
    /* ctors */
    OutputManager(Spec const& spec);

    /* dtors */
    virtual ~OutputManager();

    /* accessors */
    void write_to_file(Options const& options,
                       size_t const indent_size = 4) const;
};

}  /* elfin */

#endif  /* end of include guard: OUTPUT_MANAGER_H_ */