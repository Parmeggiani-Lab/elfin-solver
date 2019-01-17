#include "work_package.h"

#include "priv_impl.h"

namespace elfin {

/* private */
struct WorkPackage::PImpl : public PImplBase<WorkPackage> {
    using PImplBase::PImplBase;
};

WorkPackage::WorkPackage() :
    pimpl_(new_pimpl<PImpl>(*this))
{}

WorkPackage::~WorkPackage() {}

void WorkPackage::solve() {
    UNIMP();
}

}  /* elfin */