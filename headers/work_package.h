#ifndef WORK_PACKAGE_H_
#define WORK_PACKAGE_H_

#include "work_area.h"

namespace elfin {

class WorkPackage {
private:
    /* type */
    struct PImpl;

    /* data */
    std::unique_ptr<PImpl> pimpl_;

public:
    /* ctors */
    WorkPackage();
    WorkPackage(WorkPackage const& other) = delete;
    WorkPackage(WorkPackage&& other) = delete;

    /* dtors */
    virtual ~WorkPackage();

    /* modifiers */
    WorkPackage& operator=(WorkPackage const& other) = delete;
    WorkPackage& operator=(WorkPackage&& other) = delete;
    void solve();
};

}  /* elfin */

#endif  /* end of include guard: WORK_PACKAGE_H_ */