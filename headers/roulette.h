#ifndef ROULETTE_H_
#define ROULETTE_H_

#include <vector>
#include <type_traits>

#include "jutil.h"
#include "random_utils.h"
#include "string_utils.h"
#include "debug_utils.h"

namespace elfin {

template<class Container, typename ItemType>
class Roulette {
private:
    /* data members */
    Container & container_;
    std::vector<size_t> cml_sum_;
    size_t total_ = 0;
public:
    /* ctors & dtors */
    Roulette(Container & container) :
        container_(container) {}

    /* getters & setters */
    const std::vector<size_t> & cml_sum() const { return cml_sum_; }
    const Container & container() const { return container_; }
    size_t total() const { return total_; }

    ItemType & rand_item() {
#ifndef NDEBUG
        DEBUG(cml_sum_.size() != container_.size(),
              string_format("cml_sum size=%lu but container size=%lu\n",
                            cml_sum_.size(), container_.size()));
#endif  /* ifndef NDEBUG */
        auto itr = std::upper_bound(
                       cml_sum_.begin(),
                       cml_sum_.end(),
                       get_dice(total_)
                   );
        return container_.at(itr - cml_sum_.begin());
    }

    const ItemType & rand_item() const {
#ifndef NDEBUG
        DEBUG(cml_sum_.size() != container_.size(),
              string_format("cml_sum size=%lu but container size=%lu\n",
                            cml_sum_.size(), container_.size()));
#endif  /* ifndef NDEBUG */
        auto itr = std::upper_bound(
                       cml_sum_.begin(),
                       cml_sum_.end(),
                       get_dice(total_)
                   );
        return container_.at(itr - cml_sum_.begin());
    }

    void cumulate(const size_t cs) {
        total_ += cs;
        cml_sum_.push_back(total_);
    }
};

}  /* elfin */

#endif  /* end of include guard: ROULETTE_H_ */