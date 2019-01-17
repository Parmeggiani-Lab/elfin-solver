#ifndef SPEC_H_
#define SPEC_H_

#include "map_utils.h"
#include "work_package.h"

namespace elfin {

/* Fwd Decl */
struct Options;
typedef SPMap<WorkPackage> WorkPackages;

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
    WorkPackages const& work_packages() const;

    /* modifiers */
    Spec& operator=(Spec const& other) = delete;
    Spec& operator=(Spec&& other);
    void solve_all();
};

}  /* elfin */

#endif  /* end of include guard: SPEC_H_ */