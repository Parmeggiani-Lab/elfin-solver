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
    /* types */
    typedef Roulette<LinkList, Link> LinkListRoulette;

    /* data members */
    LinkList links_;
    LinkListRoulette n_rlt_ = LinkListRoulette(links_);
    LinkListRoulette c_rlt_ = LinkListRoulette(links_);
    size_t basic_link_size_;

public:
    /* data members */
    const LinkList & links() const { return links_; }

    /* other mehotds */
    void finalize();
    const Link & pick_random_link(const TerminusType term) const;
};

}  /* elfin */

#endif  /* end of include guard: TERMINUS_H_ */