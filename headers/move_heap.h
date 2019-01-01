#ifndef MOVE_HEAP_H_
#define MOVE_HEAP_H_

#include <queue>

namespace elfin {

// https://stackoverflow.com/questions/22047964/how-to-move-elements-out-of-stl-priority-queue
template <
    class T,
    class Container = std::vector<T>,
    class Compare = std::less<typename Container::value_type >>
class MoveHeap : public std::priority_queue<T, Container, Compare> {
public:
    T top_and_pop() {
        std::pop_heap(c.begin(), c.end(), comp);
        T value = std::move(c.back());
        c.pop_back();
        return value;
    }

protected:
    using std::priority_queue<T, Container, Compare>::c;
    using std::priority_queue<T, Container, Compare>::comp;
};

}  /* elfin */

#endif  /* end of include guard: MOVE_HEAP_H_ */