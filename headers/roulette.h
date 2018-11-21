#ifndef ROULETTE_H_
#define ROULETTE_H_

#include <vector>

#include "jutil.h"
#include "random_utils.h"

namespace elfin {

struct Roulette
{
    std::vector<float> cmlprobs;

    /*
     * Picks a random item of type T based on cumulative probability.
     *
     * WARNING: This assumes the cumulative probabilities were assigned in
     * ascending sorted order.
     */
    template<typename T>
    T rand_item(const std::vector<T> items) const {
        panic_when(items.size() != cmlprobs.size());

        auto itr = std::upper_bound(
                       cmlprobs.begin(),
                       cmlprobs.end(),
                       get_dice_0to1()
                   );
        return items.at(itr - cmlprobs.begin());
    }

    /*
     * Divides all cumulative probabilities by sum so the range becomes
     * (0, 1].
     */
    void normalize(const size_t sum) {
        panic_when(sum <= cmlprobs.back());
        
        for (float & cp : cmlprobs) {
            cp /= sum;
        }
    }
};

}  /* elfin */

#endif  /* end of include guard: ROULETTE_H_ */