#pragma once

#include <vector> //todo clean up includes. lots of duplicate cstdint, cassert.
#include <cstdint>
#include <cassert>

#include <limits>

namespace tc {
    using Mult = u_int16_t;
    constexpr Mult FREE = 0;

    /**
     * @brief Mapping from "global" generator names or objects to indexes used for value lookup. 
     * @tparam Gen_ 
     */
    template<typename Gen_=void>
    struct Index;
    
    /**
     * @brief Complete representation of a quotient group. Describes the action of each generator on each coset.
     * @tparam Gen_ 
     */
    template<typename Gen_=void>
    struct Cosets;

    /** 
     * @brief Manage the presentation of a Coxeter group and enforce constraints
     * on the multiplicities of its relations.
     * <ul>
     *   <li>
     *     <code>m_ij = 1</code> iff <code>i != j</code>
     *   </li>
     *   <li>
     *     <code>m_ij = m_ji</code>
     *   </li>
     *   <li>
     *     If <code>m_ij == inf</code> (<code>tc::FREE</code>) then no relation is imposed.
     *   </li>
     * </ul>
     * @see
     * <a href="https://en.wikipedia.org/wiki/Coxeter_group#Definition">Coxeter Group (Wikipedia)</a>
     */
    template<typename Gen_=void>
    struct Group;

    template<>
    struct Index<> {
        size_t operator()(size_t const &idx) const {
            return idx;
        }
    };

    template<typename Gen_>
    struct Index {
        using Gen = Gen_;

        std::vector<Gen> _gens{};

        explicit Index(std::vector<Gen> gens) : _gens(gens) {}

        size_t operator()(Gen const &gen) const {
            auto it = std::find(_gens.begin(), _gens.end(), gen);
            assert(it != _gens.end());
            return it - _gens.begin();
        }
    };

    template<>
    struct Cosets<> {
        static constexpr size_t UNSET = std::numeric_limits<size_t>::max();

    private:
        size_t _rank;
        size_t _order;
        bool _complete;
        std::vector<size_t> _data;

    public:
        Cosets(Cosets const &) = default;

        Cosets(Cosets &&) noexcept = default;

        ~Cosets() = default;

        void set(size_t coset, size_t gen, size_t target);

        [[nodiscard]] size_t get(size_t coset, size_t gen) const;

        [[nodiscard]] bool isset(size_t coset, size_t gen) const;

        [[nodiscard]] size_t rank() const;

        [[nodiscard]] size_t order() const;

        [[nodiscard]] bool complete() const;

        [[nodiscard]] size_t size() const;

        friend Group<>;  // only constructible via Group<>::solve

    private:
        explicit Cosets(size_t rank);

        void add_row();

        void set(size_t idx, size_t target);

        [[nodiscard]] size_t get(size_t idx) const;

        [[nodiscard]] bool isset(size_t idx) const;
    };

    template<>
    struct Group<> {
        using Rel = std::tuple<size_t, size_t, Mult>;

    private:
        size_t _rank;
        std::vector<size_t> _mults;

    public:
        Group(Group const &) = default;

        Group(Group &&) noexcept = default;

        ~Group() = default;

        explicit Group(size_t rank);

        void set(size_t, size_t, Mult);

        [[nodiscard]] Mult get(size_t, size_t) const;

        [[nodiscard]] size_t rank() const;

        [[nodiscard]] Group sub(std::vector<size_t> const &idxs) const;

        [[nodiscard]] Cosets<> solve(std::vector<size_t> const &idxs, size_t bound = SIZE_MAX) const;
    };

    template<typename Gen_>
    struct Cosets : public Cosets<> {
        using Gen = Gen_;

    private:
        Index<Gen> _index;

    public:
        Cosets(Cosets<> g, std::vector<Gen> gens)
            : Cosets<>(g), _index(gens) {}

        void set(size_t coset, Gen const &gen, size_t target) {
            Cosets<>::set(coset, _index(gen), target);
        }

        [[nodiscard]] size_t get(size_t coset, Gen const &gen) const {
            return Cosets<>::get(coset, _index(gen));
        }

        [[nodiscard]] bool isset(size_t coset, Gen const &gen) const {
            return Cosets<>::isset(coset, _index(gen));
        }
        
        [[nodiscard]] std::vector<Gen> gens() const {
            return _index._gens;
        }

    private:
        Cosets(size_t rank, std::vector<Gen> gens)
            : Cosets<>(rank), _index(gens) {}
    };

    template<typename Gen_>
    struct Group : public Group<> {
        using Gen = Gen_;
        using Rel = std::tuple<Gen, Gen, Mult>;

    private:
        Index<Gen> _index;

    public:
        Group(Group const &) = default;

        Group(Group &&) noexcept = default;

        Group(Group<> g, std::vector<Gen> gens)
            : Group<>(g), _index(gens) {}

        Group(size_t rank, std::vector<Gen> gens)
            : Group<>(rank), _index(gens) {}

        ~Group() = default;

        void set(Gen const &u, Gen const &v, Mult m) {
            Group<>::set(_index(u), _index(v), m);
        }

        [[nodiscard]] Mult get(Gen const &u, Gen const &v) const {
            return Group<>::get(_index(u), _index(v));
        }

        [[nodiscard]] std::vector<Gen> gens() const {
            return _index._gens;
        }
        
        [[nodiscard]] Group sub(std::vector<Gen> const &gens) const {
            std::vector<size_t> idxs(gens.size());
            std::transform(gens.begin(), gens.end(), idxs.begin(), _index);
            return Group(Group<>::sub(idxs), gens);
        }

        [[nodiscard]] Cosets<Gen> solve(std::vector<Gen> const &gens, size_t bound = SIZE_MAX) const {
            std::vector<size_t> idxs(gens.size());
            std::transform(gens.begin(), gens.end(), idxs.begin(), _index);

            return Cosets<Gen>(Group<>::solve(idxs, bound), gens);
        }
    };
}
