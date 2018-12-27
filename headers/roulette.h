#ifndef ROULETTE_H_
#define ROULETTE_H_

#include <vector>
#include <sstream>

#include "random_utils.h"
#include "string_utils.h"
#include "debug_utils.h"

namespace elfin {

//
// A vector wrapper that also stores a probability distribution from which
// random items are drawn.
//
template<typename ItemType>
class Roulette {
protected:
    /* types */
    typedef std::vector<ItemType> ItemList;
    typedef std::vector<float> CummProbDist;
    
    /* data */
    ItemList items_;
    CummProbDist cpd_;
    float total_ = 0;

    /* modifiers */
    void accumulate_prob(float const prob) {
        total_ += prob;
        cpd_.push_back(total_);
    }
public:
    /* ctors */
    Roulette() {}
    Roulette(ItemList const& items, CummProbDist const& cpd) :
        items_(items) {
        for (float prob : cpd) {
            accumulate_prob(prob);
        }
        NICE_PANIC(items.size() != cpd.size());
    }

    /* dtors */
    virtual ~Roulette() {}

    /* accessors */
    ItemList const& items() const { return items_; }
    CummProbDist const& cpd() const { return cpd_; }
    size_t total() const { return total_; }
    ItemType const& draw() const {
        NICE_PANIC(cpd_.empty());
        DEBUG(cpd_.size() != items_.size(),
              string_format("cml_sum size=%zu but container size=%zu\n",
                            cpd_.size(), items_.size()));
        auto itr = std::upper_bound(
                       begin(cpd_),
                       end(cpd_),
                       random::get_dice(total_)
                   );
        return items_.at(itr - begin(cpd_));
    }

    /* modifiers */
    void push_back(float const prob, ItemType const& item) {
        accumulate_prob(prob);
        items_.push_back(item);
    }

    void clear() {
        items_.clear();
        cpd_.clear();
        total_ = 0;
    }

    template <class ... Args>
    void emplace_back(float const prob, Args &&... args) {
        accumulate_prob(prob);
        items_.emplace_back(std::forward<Args>(args)...);
    }

    void pop_back() {
        cpd_.pop_back();
        total_ = cpd_.back();
        items_.pop_back();
    }
};

}  /* elfin */

#endif  /* end of include guard: ROULETTE_H_ */