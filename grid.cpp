#include <iostream>
#include <vector>
#include <array>
#include <cassert>
#include <utility>


template <typename T>
class GridView {
    T* ptr;
    unsigned cols;
public:
    GridView(T* p, unsigned c) : ptr(p), cols(c) {}
    T& operator[](unsigned x) { return ptr[x]; }
    const T& operator[](unsigned x) const { return ptr[x]; }
};

template <typename T, unsigned N = 2>
class Grid {
private:
    std::vector<T> data;             
    std::array<unsigned, N> dims;    

    
    Grid() : data(), dims{} {}

    
    template <typename Tuple, std::size_t... Is>
    std::array<unsigned, N> get_dims(Tuple&& t, std::index_sequence<Is...>) {
        return { static_cast<unsigned>(std::get<Is>(t))... };
    }

    
    Grid<T, N - 1> make_subgrid(unsigned i) const {
        Grid<T, N - 1> subgrid;
        unsigned sub_size = 1;
        for (unsigned j = 1; j < N; ++j) {
            subgrid.dims[j - 1] = dims[j];
            sub_size *= dims[j];
        }
        subgrid.data.assign(data.begin() + i * sub_size, data.begin() + (i + 1) * sub_size);
        return subgrid;
    }

    template <typename U, unsigned M> friend class Grid;

public:
    using size_type = unsigned;

    
    Grid(T const& t) : data(1, t) {
        dims.fill(1);
    }

    
    template <typename... Args>
    Grid(Args... args) {
        static_assert(sizeof...(Args) == N || sizeof...(Args) == N + 1, 
                      "Ошибка: неверное количество аргументов!");
        
        auto tup = std::forward_as_tuple(args...);
        dims = get_dims(tup, std::make_index_sequence<N>{}); 
        
        unsigned total_size = 1;
        for (auto d : dims) total_size *= d;

        if constexpr (sizeof...(Args) == N) {
            data.resize(total_size); 
        } else {
            data.assign(total_size, std::get<N>(tup)); 
        }
    }

   
    ~Grid() = default;
    Grid(const Grid&) = default;
    Grid& operator=(const Grid&) = default;
    Grid(Grid&&) noexcept = default;
    Grid& operator=(Grid&&) noexcept = default;

    
    template <typename... Args>
    T& operator()(Args... args) {
        static_assert(sizeof...(Args) == N, "Неверное число индексов");
        unsigned idx_arr[] = { static_cast<unsigned>(args)... };
        unsigned flat_idx = 0;
        for (unsigned i = 0; i < N; ++i) flat_idx = flat_idx * dims[i] + idx_arr[i];
        return data[flat_idx];
    }

    template <typename... Args>
    const T& operator()(Args... args) const {
        static_assert(sizeof...(Args) == N, "Неверное число индексов");
        unsigned idx_arr[] = { static_cast<unsigned>(args)... };
        unsigned flat_idx = 0;
        for (unsigned i = 0; i < N; ++i) flat_idx = flat_idx * dims[i] + idx_arr[i];
        return data[flat_idx];
    }

    
    decltype(auto) operator[](unsigned i) {
        if constexpr (N == 1) {
            return (data[i]);
        } else if constexpr (N == 2) {
            return GridView<T>(data.data() + i * dims[1], dims[1]);
        } else {
            return make_subgrid(i);
        }
    }

    decltype(auto) operator[](unsigned i) const {
        if constexpr (N == 1) {
            return T(data[i]);
        } else if constexpr (N == 2) {
            return GridView<const T>(data.data() + i * dims[1], dims[1]);
        } else {
            return make_subgrid(i);
        }
    }

    size_type get_y_size() const { return dims[0]; }
    size_type get_x_size() const { return dims[1]; }
};


int main() {
    Grid<float> g(3, 2, 0.0f);
    assert(3 == g.get_y_size());
    assert(2 == g.get_x_size());

    using gsize_t = Grid<int>::size_type;
    for (gsize_t y = 0; y != g.get_y_size(); ++y)
        for (gsize_t x = 0; x != g.get_x_size(); ++x)
            assert(0.0f == g[y][x]);

    for (gsize_t y = 0; y != g.get_y_size(); ++y)
        for (gsize_t x = 0; x != g.get_x_size(); ++x)
            g[y][x] = 1.0f;

    for (gsize_t y = 0; y != g.get_y_size(); ++y)
        for (gsize_t x = 0; x != g.get_x_size(); ++x)
            assert(1.0f == g(y, x));

    Grid<float, 3> const g3(2, 3, 4, 1.0f);
    assert(1.0f == g3(1, 1, 1));

    Grid<float, 2> g2(2, 5, 2.0f);
    assert(2.0f == g2(1, 1));

    g2 = g3[1];
    assert(1.0f == g2(1, 1));

    struct NoDefault {
        int v;
        explicit NoDefault(int x) : v(x) {}
    };
    Grid<NoDefault> gnd(2, 3, NoDefault(7));
    assert(7 == gnd(1, 2).v);

    std::cout << "ALL TESTS PASSED\n";
    return 0;
}