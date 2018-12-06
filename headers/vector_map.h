#ifndef VECTOR_MAP_H_
#define VECTOR_MAP_H_

#include <vector>
#include <unordered_map>

#include "random_utils.h"

#define VECTOR_MAP_RESERVE_N 16

namespace elfin {

template<typename ItemType>
class VectorMap {
protected:
    /* types */
    typedef std::vector<ItemType> ItemList;
    typedef std::unordered_map<ItemType, size_t> ItemMap;
    typedef typename ItemMap::iterator ItemMapIterator;

    /* data members*/
    ItemList items_;
    ItemMap item_to_id_map_;

public:
    /* ctors */
    VectorMap() {
        items_.reserve(VECTOR_MAP_RESERVE_N);
    }

    /* dtors */
    virtual ~VectorMap() {}

    /* getters */
    const ItemList & items() const { return items_; }
    bool empty() const { return items_.empty(); }
    size_t size() const { return items_.size(); }
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
        items_.emplace_back(std::forward<Args>(args)...);
        item_to_id_map_[items_.back()] = items_.size() - 1;
    }

    void erase(ItemMapIterator itr) {
        // itr->second is value, which is index to items_
        const ItemType & back_val = items_.back();
        item_to_id_map_[back_val] = itr->second;
        
        items_[itr->second] = back_val;
        items_.pop_back();

        // itr->first is key to map
        item_to_id_map_.erase(itr->first);
    }

    void erase(const ItemType & item) {
        erase(item_to_id_map_.find(item));
    }

    void pop_back() {
        item_to_id_map_.erase(item_to_id_map_.find(items_.back()));
        items_.pop_back();
    }

    void clear() {
        items_.clear();
        item_to_id_map_.clear();
    }
};

}  /* elfin */

#endif  /* end of include guard: VECTOR_MAP_H_ */