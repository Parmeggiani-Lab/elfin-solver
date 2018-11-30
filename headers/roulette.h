#ifndef ROULETTE_H_
#define ROULETTE_H_

#include <vector>
#include <type_traits>

#include "jutil.h"
#include "random_utils.h"

namespace elfin {

// struct Roulette {
//     std::vector<float> cmlprobs;

//     /*
//      * Picks a random item of type T based on cumulative probability.
//      *
//      */
//     template<typename T>
//     T & rand_item(const std::vector<T> items) const {
//         panic_when(items.size() != cmlprobs.size());

//         auto itr = std::upper_bound(
//                        cmlprobs.begin(),
//                        cmlprobs.end(),
//                        get_dice_0to1()
//                    );
//         return items.at(itr - cmlprobs.begin());
//     }

//     /*
//      * Divides all cumulative probabilities by sum so the range becomes
//      * (0, 1].
//      */
//     void normalize(const size_t sum) {
//         panic_when(sum <= cmlprobs.back());

//         for (float & cp : cmlprobs) {
//             cp /= sum;
//         }
//     }
// };

template<class Container, typename ItemType>
class Roulette {
public:
    typedef size_t (*CmlSumFunctor)(ItemType &);

private:
    /* data members */
    Container & container_;
    CmlSumFunctor cml_sum_functor_;
    std::vector<size_t> cml_sum_;
    size_t total_ = 0;
public:
    /* ctors & dtors */
    Roulette(Container & container, CmlSumFunctor cml_sum_functor) :
        container_(container), cml_sum_functor_(cml_sum_functor) {}

    /* getters & setters */
    const std::vector<size_t> & cml_sum() const { return cml_sum_; }
    size_t total() const { return total_; }

    ItemType & rand_item() {
        auto itr = std::upper_bound(
                       cml_sum_.begin(),
                       cml_sum_.end(),
                       get_dice(total_)
                   );
        return container_.at(itr - cml_sum_.begin());
    }

    const ItemType & rand_item() const {
        auto itr = std::upper_bound(
                       cml_sum_.begin(),
                       cml_sum_.end(),
                       get_dice(total_)
                   );
        return container_.at(itr - cml_sum_.begin());
    }

    void cumulate(size_t cs) {
        total_ += cs;
        cml_sum_.push_back(total_);
    }
};

}  /* elfin */

#endif  /* end of include guard: ROULETTE_H_ */