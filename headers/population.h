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
    Buffer const* back_buffer_ = nullptr;
    WorkArea const* work_area_ = nullptr;

    /* modifiers */
    void release_resources();

    /* static */
    static void copy_buffer(
        Buffer const* src,
        Buffer * dst);
public:
    /* ctors */
    Population(WorkArea const* work_area);
    Population(Population const& other);
    Population(Population && other);

    /* dtors */
    virtual ~Population();

    /* accessors */
    Buffer const* front_buffer() const { return front_buffer_; }
    Buffer const* back_buffer() const { return back_buffer_; }

    /* modifiers */
    Population & operator=(Population const& other);
    Population & operator=(Population && other);
    void evolve();
    void score();
    void rank();
    void select();
    void swap_buffer();
};

}  /* elfin */

#endif  /* end of include guard: POPULATION_H_ */