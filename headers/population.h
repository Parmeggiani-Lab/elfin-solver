#ifndef POPULATION_H_
#define POPULATION_H_

#include <vector>
#include <type_traits>

#include "candidate.h"
#include "work_area.h"

namespace elfin {

class Population
{
protected:
    /* type */
    struct Buffer : public CandidateList {
        virtual ~Buffer() {
            for (auto cand_ptr : *this) {
                delete cand_ptr;
            }
            this->clear();
        }
    };
    /* data */
    Buffer * front_buffer_ = nullptr;
    const Buffer * back_buffer_ = nullptr;
    const WorkArea * work_area_ = nullptr;

    /* modifiers */
    void release_resources();

    /* static */
    static void copy_buffer(
        const Buffer * src,
        Buffer * dst);
public:
    /* ctors */
    Population(const WorkArea * work_area);
    Population(const Population & other);
    Population(Population && other);

    /* dtors */
    virtual ~Population();

    /* accessors */
    const Buffer * front_buffer() const { return front_buffer_; }
    const Buffer * back_buffer() const { return back_buffer_; }

    /* modifiers */
    Population & operator=(const Population & other);
    Population & operator=(Population && other);
    void evolve();
    void score();
    void rank();
    void select();
    void swap_buffer();
};

}  /* elfin */

#endif  /* end of include guard: POPULATION_H_ */