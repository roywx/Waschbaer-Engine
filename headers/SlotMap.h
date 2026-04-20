/*
Slot Map is a custom data structure based on Allan Deutsch's C++Now 2017 talk
*/ 

#ifndef SLOT_MAP_H
#define SLOT_MAP_H

#include <vector>
#include <stdexcept>

template<typename T>
class SlotMap {
public:
    using iterator = typename std::vector<T>::iterator;

    struct alignas(8) Handle {
        uint32_t index;
        uint32_t generation;
    };

    bool contains(const Handle& h) const;
    Handle insert(const T& value);
    Handle insert(T&& value);
    T& get(const Handle& h);
    void erase(const Handle& h);
    uint32_t size() const;
    void clear();
    void reserve(int size);
    Handle handle_of(const iterator it) const;

    iterator begin() { return _data.begin(); }
    iterator end()   { return _data.end(); }

private:
    std::vector<Handle> _indices{{0, 0}};
    std::vector<uint32_t> _erase; 
    std::vector<T> _data;

    uint32_t _freeHead = 0;
    uint32_t _freeTail = 0;
};

template<typename T>
uint32_t SlotMap<T>::size() const{
    return _data.size();
}

// Doesn't protect against Handle use after clear()
template<typename T>
void SlotMap<T>::clear(){
    _data.clear();
    _erase.clear();
    _indices = {{0, 0}};

    _freeHead = 0;
    _freeTail = 0;
}

template<typename T>
void SlotMap<T>::reserve(int size){
    _data.reserve(size);
    _indices.reserve(size);
    _erase.reserve(size);
}

template<typename T>
typename SlotMap<T>::Handle SlotMap<T>::handle_of(const iterator it) const{
   ptrdiff_t d_idx = it - _data.begin();
    if (d_idx < 0 || static_cast<size_t>(d_idx) >= _data.size())
        throw std::out_of_range("error slotmap iterator out of range ");
    uint32_t slot = _erase[d_idx];
    return { slot, _indices[slot].generation };
}

template<typename T>
bool SlotMap<T>::contains(const Handle& h) const{
    return (h.index < _indices.size() && _indices[h.index].generation == h.generation);
}

template<typename T>
T& SlotMap<T>::get(const Handle& h){
    if(contains(h)){
        return _data[_indices[h.index].index];
    }
    throw std::out_of_range("error accessing slotmap with invalid or expired handle");
}

template<typename T>
typename SlotMap<T>::Handle SlotMap<T>::insert(const T& value){
    if(_freeHead == _freeTail){
        _indices.push_back({0, 0});
        _freeTail = _indices.size() - 1;
        _indices[_freeHead].index = _freeTail;
    }

    _data.push_back(value);
    _erase.push_back(_freeHead);

    uint32_t temp = _indices[_freeHead].index;
    _indices[_freeHead].index = _data.size() - 1;
    Handle key = {_freeHead, _indices[_freeHead].generation};
    _freeHead = temp;

    return key;
}

template<typename T>
typename SlotMap<T>::Handle SlotMap<T>::insert(T&& value){
    if(_freeHead == _freeTail){
        _indices.push_back({0, 0});
        _freeTail = _indices.size() - 1;
        _indices[_freeHead].index = _freeTail;
    }

    _data.push_back(std::move(value));
    _erase.push_back(_freeHead);

    uint32_t temp = _indices[_freeHead].index;
    _indices[_freeHead].index = _data.size() - 1;
    Handle key = {_freeHead, _indices[_freeHead].generation};
    _freeHead = temp;

    return key;
}

template<typename T>
void SlotMap<T>::erase(const Handle& h){
    if(!contains(h)) return;
    _indices[h.index].generation++;
    uint32_t d_idx = _indices[h.index].index;
    std::swap(_data[d_idx],  _data.back());
    _data.pop_back();
    std::swap(_erase[d_idx], _erase.back());
    _erase.pop_back();

    if(d_idx < _erase.size()) { _indices[_erase[d_idx]].index = d_idx; } 
    
    _indices[_freeTail].index = h.index;
    _freeTail = h.index;  
    
}

#endif