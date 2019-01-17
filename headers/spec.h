#ifndef SPEC_H_
#define SPEC_H_

#include "map_utils.h"
#include "work_package.h"

namespace elfin {

/* Fwd Decl */
struct Options;
class WorkArea;
typedef SPMap<WorkArea> WorkAreaMap;

class Spec {
private:
    /* type */
    struct PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;

public:
    /* ctors */
    Spec(Options const& options);
    Spec(Spec const& other) = delete;
    Spec(Spec&& other);

    /* dtors */
    virtual ~Spec();

    /* accessors */
    WorkAreaMap const& work_areas() const;

    /* modifiers */
    Spec& operator=(Spec const& other) = delete;
    Spec& operator=(Spec&& other);
    void solve_all();
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */