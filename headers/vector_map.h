#ifndef VECTOR_MAP_H_
#define VECTOR_MAP_H_

#include <vector>
#include <unordered_map>

#include "random_utils.h"

namespace elfin {

template<typename ItemType>
class VectorMap {
private:
    /* types */
    typedef std::vector<ItemType> ItemList;
    typedef std::unordered_map<ItemType, size_t> ItemMap;

    /* data members*/
    ItemList items_;
    ItemMap item_to_id_map_;

public:
    /* ctors */
    VectorMap() {}

    /* dtors */
    virtual ~VectorMap() {}

    /* getters */
    const ItemList & items() const { return items_; }

    ItemList::iterator find(const ItemType & item) {
        return items_.find(item);
    }

    bool has(const ItemType & item) const {
        return item_to_id_map_.find(item) != item_to_id_map_.end();
    }

    /*
     * Returns a random item based on uniform probability.
     */
    ItemType & rand_item() {
        DEBUG(items_.size() == 0);
        const size_t rand_idx = get_dice(items_.size());
        return items_.at(rand_idx);
    }

    const ItemType & rand_item() const {
        return const_cast<VectorMap<ItemType> *>(this)->rand_item();
    }

    /* modifiers */
    void push_back(const ItemType & item) {
        item_to_id_map_[item] = items_.size();
        items_.push_back(item);
    }

    template <class ... Args>
    void emplace_back(Args && ... args) {
        items_.emplace_back(args);
        item_to_id_map_[items_.back()] = items_.size() - 1;
    }

    void erase(ItemList::iterator itr) {
        item_to_id_map_.erase(*itr);
        items_[itr - items_.begin()] = items_.back();
        items_.pop_back();
    }

    void pop_back() {
        item_to_id_map_.erase(item_to_id_map_.find(items_.back()));
        items_.pop_back();
    }
};

}  /* elfin */

#endif  /* end of include guard: VECTOR_MAP_H_ */