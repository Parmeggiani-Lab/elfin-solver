#ifndef TERMINUS_H_
#define TERMINUS_H_

#include "link.h"
#include "terminus_type.h"
#include "roulette.h"

namespace elfin {

struct Module;

class Terminus {
    friend Module;
private:
    /* data members */
    bool finalized_ = false;
    LinkList links_;
    Roulette<Link *> n_rlt_;
    Roulette<Link *> c_rlt_;
    size_t basic_link_size_;

public:
    /* data */
    const LinkList & links() const { return links_; }

    /* getters */
    const Link & pick_random_link(const TerminusType term) const;

    /* modifiers */
    void finalize();
};

}  /* elfin */

#endif  /* end of include guard: TERMINUS_H_ */