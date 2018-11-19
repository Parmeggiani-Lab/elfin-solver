#ifndef ROULETTE_H_
#define ROULETTE_H_

#include <vector>

#include "jutil.h"

namespace elfin {

template<typename T>
struct Roulette
{
    std::vector<float> cmlprobs;
    T rand_item(const std::vector<T> items) const {
        /*
         * Picks a random module based on cumulative probability derived as
         * n_links of module over database link_total_.
         *
         * WARNING: This assumes the cumulative probabilities were assigned in
         * ascending sorted order.
         */
        panic_if(items.size() != cmlprobs.size(),
                 "Roulette::rand_item() size mismatch: items.size() != cmlprobs.size()");

        auto itr = std::upper_bound(
                       cmlprobs.begin(),
                       cmlprobs.end(),
                       get_dice_0to1()
                   );
        return items.at(itr - cmlprobs.begin());
    }
};

}  /* elfin */

#endif  /* end of include guard: ROULETTE_H_ */