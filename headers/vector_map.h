#ifndef VECTOR_MAP_H_
#define VECTOR_MAP_H_

#include <vector>
#include <unordered_map>

#include "vector_utils.h"

namespace elfin {

template<typename ItemType>
class VectorMap {
protected:
    /* types */
    typedef Vector<ItemType> ItemList;
    typedef typename ItemList::iterator ItemListItr;
    typedef typename ItemList::const_iterator ItemListCItr;
    typedef std::unordered_map<ItemType, size_t> ItemMap;
    typedef typename ItemMap::iterator ItemMapIterator;

    /* data members*/
    ItemList items_;
    ItemMap item_to_id_map_;
public:
    /* ctors */
    VectorMap() {}

    /* dtors */
    virtual ~VectorMap() {}

    /* accessors */
    ItemList const& items() const { return items_; }
    ItemListCItr find(ItemType const& item) const {
        DEBUG(items_.empty());

        auto map_itr = item_to_id_map_.find(item);
        DEBUG(map_itr == item_to_id_map_.end());

        return items_.begin() + map_itr.second;
    }
    bool empty() const { return items_.empty(); }
    size_t size() const { return items_.size(); }
    bool has(ItemType const& item) const {
        return item_to_id_map_.find(item) != item_to_id_map_.end();
    }
    ItemListCItr begin() const { return items_.begin(); }
    ItemListCItr end() const { return items_.end(); }
    ItemType const& pick_random() const {
        return this->pick_random();
    }
    ItemType const& back() const {
        return items_.back();
    }

    /* modifiers */
    ItemType & back() {
        return items_.back();
    }

    ItemType & pick_random() {
        return items_.pick_random();
    }

    void reserve(size_t const size) {
        items_.reserve(size);
    }

    void push_back(ItemType const& item) {
        item_to_id_map_[item] = items_.size();
        items_.push_back(item);
    }

    void push_back(ItemType && item) {
        item_to_id_map_[item] = items_.size();
        items_.push_back(std::move(item));
    }

    template <class ... Args>
    void emplace_back(Args && ... args) {
        items_.emplace_back(std::forward<Args>(args)...);
        item_to_id_map_[items_.back()] = items_.size() - 1;
    }

    void erase(ItemMapIterator itr) {
        // itr->second is value, which is index to items_
        ItemType const& back_val = items_.back();
        item_to_id_map_[back_val] = itr->second;

        items_[itr->second] = back_val;
        items_.pop_back();

        // itr->first is key to map
        item_to_id_map_.erase(itr->first);
    }

    void erase(ItemType & item) {
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