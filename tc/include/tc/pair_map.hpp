#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <vector>

namespace tc {
    template<typename T>
    struct pair_map {
        struct iterator;
        struct const_iterator;
        struct view;
        struct const_view;

    private:
        size_t _size;
        std::vector<T> _data{};

        static size_t idx(size_t i, size_t j);

    public:
        explicit pair_map(size_t size);

        explicit pair_map(size_t size, const T &value);

        [[nodiscard]] size_t size() const;

        T &operator()(size_t i, size_t j);

        T operator()(size_t i, size_t j) const;

        view of(size_t f);

        const_view of(size_t f) const;

        const_view cof(size_t f);

        iterator begin();

        iterator end();

        const_iterator begin() const;

        const_iterator end() const;

        const_iterator cbegin();

        const_iterator cend();
    };

    template<typename T>
    struct pair_map<T>::iterator {
        using reference = std::tuple<size_t, size_t, T &>;

    private:
        pair_map<T> &_map;
        size_t _i, _j;

    public:
        iterator(pair_map<T> &map, size_t i, size_t j);

        iterator operator++();

        iterator operator++(int) &;

        reference operator*();

        bool operator!=(const iterator &other);
    };

    template<typename T>
    struct pair_map<T>::const_iterator {
        using value_type = std::tuple<size_t, size_t, T>;

    private:
        const pair_map<T> &_map;
        size_t _i, _j;

    public:
        const_iterator(const pair_map<T> &map, size_t i, size_t j);

        const_iterator operator++();

        const_iterator operator++(int) &;

        value_type operator*();

        bool operator!=(const const_iterator &other);
    };

    template<typename T>
    struct pair_map<T>::view {
        struct iterator;
        struct const_iterator;

    private:
        pair_map<T> &_map;
        size_t _f;

    public:
        view(pair_map<T> &map, size_t f);

        iterator begin();

        iterator end();

        const_iterator begin() const;

        const_iterator end() const;

        const_iterator cbegin();

        const_iterator cend();
    };

    template<typename T>
    struct pair_map<T>::view::iterator {
        using reference = std::tuple<size_t, size_t, T &>;

    private:
        pair_map<T> &_map;
        size_t _f, _v;

    public:
        iterator(pair_map<T> &map, size_t f, size_t v);

        iterator operator++();

        iterator operator++(int);

        reference operator*();

        bool operator!=(const iterator &other);
    };

    template<typename T>
    struct pair_map<T>::view::const_iterator {
        using value_type = std::tuple<size_t, size_t, T>;

    private:
        const pair_map<T> &_map;
        size_t _f, _v;

    public:
        const_iterator(const pair_map<T> &map, size_t f, size_t v);

        const_iterator operator++();

        const_iterator operator++(int);

        value_type operator*();

        bool operator!=(const const_iterator &other);
    };

    template<typename T>
    struct pair_map<T>::const_view {
        using const_iterator = typename pair_map<T>::view::const_iterator;

    private:
        const pair_map<T> &_map;
        size_t _f;

    public:
        const_view(const pair_map<T> &map, size_t f);

        const_iterator begin() const;

        const_iterator end() const;

        const_iterator cbegin();

        const_iterator cend();
    };

    // region pair_map

    template<typename T>
    size_t pair_map<T>::idx(size_t i, size_t j) {
        if (i < j) std::swap(i, j);
        return j * (j + 1) / 2 + i;
    }

    template<typename T>
    pair_map<T>::pair_map(size_t size)
        : _size(size), _data(size * (size + 1) / 2) {}

    template<typename T>
    pair_map<T>::pair_map(size_t size, const T &value)
        : _size(size), _data(size * (size + 1) / 2, value) {}

    template<typename T>
    size_t pair_map<T>::size() const {
        return _size;
    }

    template<typename T>
    T &pair_map<T>::operator()(size_t i, size_t j) {
        return _data[idx(i, j)];
    }

    template<typename T>
    T pair_map<T>::operator()(size_t i, size_t j) const {
        return _data[idx(i, j)];
    }

    template<typename T>
    typename pair_map<T>::view pair_map<T>::of(size_t f) {
        return view(*this, f);
    }

    template<typename T>
    typename pair_map<T>::const_view pair_map<T>::of(size_t f) const {
        return const_view(*this, f);
    }

    template<typename T>
    typename pair_map<T>::const_view pair_map<T>::cof(size_t f) {
        return const_view(*this, f);
    }

    template<typename T>
    typename pair_map<T>::iterator pair_map<T>::begin() {
        return iterator(*this, 0, 0);
    }

    template<typename T>
    typename pair_map<T>::iterator pair_map<T>::end() {
        return iterator(*this, 0, _size);
    }

    template<typename T>
    typename pair_map<T>::const_iterator pair_map<T>::begin() const {
        return const_iterator(*this, 0, 0);
    }

    template<typename T>
    typename pair_map<T>::const_iterator pair_map<T>::end() const {
        return const_iterator(*this, 0, _size);
    }

    template<typename T>
    typename pair_map<T>::const_iterator pair_map<T>::cbegin() {
        return const_iterator(*this, 0, 0);
    }

    template<typename T>
    typename pair_map<T>::const_iterator pair_map<T>::cend() {
        return const_iterator(*this, 0, _size);
    }

    // endregion

    // region pair_map::iterator

    template<typename T>
    pair_map<T>::iterator::iterator(pair_map<T> &map, size_t i, size_t j)
        :_map(map), _i(i), _j(j) {}

    template<typename T>
    typename pair_map<T>::iterator pair_map<T>::iterator::operator++() {
        if (++_i > _j) {
            _i = 0;
            ++_j;
        }
        return *this;
    }

    template<typename T>
    typename pair_map<T>::iterator pair_map<T>::iterator::operator++(int) &{
        iterator it = *this;
        ++this;
        return it;
    }

    template<typename T>
    typename pair_map<T>::iterator::reference pair_map<T>::iterator::operator*() {
        return std::tie(_i, _j, _map(_i, _j));
    }

    template<typename T>
    bool pair_map<T>::iterator::operator!=(const pair_map::iterator &other) {
        return &_map != &other._map || _i != other._i || _j != other._j;
    }

    // endregion

    // region pair_map::const_iterator

    template<typename T>
    pair_map<T>::const_iterator::const_iterator(const pair_map<T> &map, size_t i, size_t j)
        :_map(map), _i(i), _j(j) {}

    template<typename T>
    typename pair_map<T>::const_iterator pair_map<T>::const_iterator::operator++() {
        if (++_i > _j) {
            _i = 0;
            ++_j;
        }
        return *this;
    }

    template<typename T>
    typename pair_map<T>::const_iterator pair_map<T>::const_iterator::operator++(int) &{
        const_iterator it = *this;
        ++this;
        return it;
    }

    template<typename T>
    typename pair_map<T>::const_iterator::value_type pair_map<T>::const_iterator::operator*() {
        return std::tuple(_i, _j, _map(_i, _j));
    }

    template<typename T>
    bool pair_map<T>::const_iterator::operator!=(const pair_map::const_iterator &other) {
        return &_map != &other._map || _i != other._i || _j != other._j;
    }

    // endregion

    // region pair_map::view

    template<typename T>
    pair_map<T>::view::view(pair_map<T> &map, size_t f)
        : _map(map), _f(f) {}

    template<typename T>
    typename pair_map<T>::view::iterator pair_map<T>::view::begin() {
        return iterator(_map, _f, 0);
    }

    template<typename T>
    typename pair_map<T>::view::iterator pair_map<T>::view::end() {
        return iterator(_map, _f, _map._size);
    }

    template<typename T>
    typename pair_map<T>::view::const_iterator pair_map<T>::view::begin() const {
        return const_iterator(_map, _f, 0);
    }

    template<typename T>
    typename pair_map<T>::view::const_iterator pair_map<T>::view::end() const {
        return const_iterator(_map, _f, _map._size);
    }

    template<typename T>
    typename pair_map<T>::view::const_iterator pair_map<T>::view::cbegin() {
        return const_iterator(_map, _f, 0);
    }

    template<typename T>
    typename pair_map<T>::view::const_iterator pair_map<T>::view::cend() {
        return const_iterator(_map, _f, _map._size);
    }

    // endregion

    // region pair_map::view::iterator

    template<typename T>
    pair_map<T>::view::iterator::iterator(pair_map<T> &map, size_t f, size_t v)
        : _map(map), _f(f), _v(v) {}

    template<typename T>
    typename pair_map<T>::view::iterator pair_map<T>::view::iterator::operator++() {
        ++_v;
        return *this;
    }

    template<typename T>
    typename pair_map<T>::view::iterator pair_map<T>::view::iterator::operator++(int) {
        iterator it = *this;
        ++this;
        return it;
    }

    template<typename T>
    typename pair_map<T>::view::iterator::reference pair_map<T>::view::iterator::operator*() {
        auto [i, j] = std::minmax(_f, _v);
        return std::tie(i, j, _map(i, j));
    }

    template<typename T>
    bool pair_map<T>::view::iterator::operator!=(const pair_map::view::iterator &other) {
        return &_map != &other._map || _f != other._f || _v != other._v;
    }

    // endregion

    // region pair_map::view::const_iterator

    template<typename T>
    pair_map<T>::view::const_iterator::const_iterator(const pair_map<T> &map, size_t f, size_t v)
        : _map(map), _f(f), _v(v) {}

    template<typename T>
    typename pair_map<T>::view::const_iterator pair_map<T>::view::const_iterator::operator++() {
        ++_v;
        return *this;
    }

    template<typename T>
    typename pair_map<T>::view::const_iterator pair_map<T>::view::const_iterator::operator++(int) {
        const_iterator it = *this;
        ++this;
        return it;
    }

    template<typename T>
    typename pair_map<T>::view::const_iterator::value_type pair_map<T>::view::const_iterator::operator*() {
        auto [i, j] = std::minmax(_f, _v);
        return std::tuple(i, j, _map(i, j));
    }

    template<typename T>
    bool pair_map<T>::view::const_iterator::operator!=(const pair_map::view::const_iterator &other) {
        return &_map != &other._map || _f != other._f || _v != other._v;
    }

    // endregion

    // region pair_map::const_view

    template<typename T>
    pair_map<T>::const_view::const_view(const pair_map<T> &map, size_t f)
        : _map(map), _f(f) {}

    template<typename T>
    typename pair_map<T>::const_view::const_iterator pair_map<T>::const_view::begin() const {
        return const_iterator(_map, _f, 0);
    }

    template<typename T>
    typename pair_map<T>::const_view::const_iterator pair_map<T>::const_view::end() const {
        return const_iterator(_map, _f, _map._size);
    }

    template<typename T>
    typename pair_map<T>::const_view::const_iterator pair_map<T>::const_view::cbegin() {
        return const_iterator(_map, _f, 0);
    }

    template<typename T>
    typename pair_map<T>::const_view::const_iterator pair_map<T>::const_view::cend() {
        return const_iterator(_map, _f, _map._size);
    }
    
    // endregion
}
