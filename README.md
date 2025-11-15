# soa.hpp
[Struct of Arrays (SoA)](https://en.wikipedia.org/wiki/AoS_and_SoA#Structure_of_arrays) container template for C++20 that uses compile-time reflection to iterate over existing fields and access fields by name.


## Features
- Just instantiate a `soa<T>` to have a Struct of Arrays for type `T`, no extra steps involved
- Uses [reflect](https://github.com/qlibs/reflect) for accessing individual fields by name and iterating over fields at compile-time
- API similar to `std::vector`, allowing for easy and familiar manipulation of the data
- Header only, easy to integrate with any project
- Requires C++20


## Usage example
```cpp
#include <cassert>
#include <string>

// 1. Include soa.hpp
#include <soa.hpp>

// You can use any struct/class, no extra markup is necessary
struct Foo {
    int a;
    std::string b;
    char c;

    bool operator==(const Foo& other) const = default;
};

int main() {
    // 2. Instantiate your SoA for the desired type.
    // In this case, "foo_soa" will contain one vector for each field of Foo.
    soa::soa<Foo> foo_soa;


    // 3. Insert data.
    // Fields will be separated in the arrays automatically.
    foo_soa.push_back(Foo{});
    foo_soa.push_back(Foo{ 1, "b", 'c' });
    foo_soa.insert(foo_soa.begin(), {
        Foo{ 1 },
        Foo{ 2 },
        Foo{ 3 },
    });
    assert(foo_soa.size() == 5);


    // 4. Access individual field arrays using `field`.
    // Pass either the field name, index, or type.
    // Note: a span is returned to avoid accessing underlying vectors directly.
    std::span<int> a_values = foo_soa.field<"a">();
    std::span<std::string> b_values = foo_soa.field<1>();
    std::span<char> c_values = foo_soa.field<char>();


    // 5. Access individual elements using `at` or `[]`.
    // A proxy object is returned referencing the fields in the SoA.
    auto proxy_0 = foo_soa[0];
    // Access references to the underlying fields using `field`
    int& a = proxy_0.field<"a">();
    std::string& b = proxy_0.field<1>();
    char& c = proxy_0.field<char>();
    // Access a tuple of all fields using `fields`
    auto&& [a2, b2, c2] = proxy_0.fields();
    // Construct a new value from the proxy
    Foo foo_0 = proxy_0;
    // Assigning a new value to the proxy changes the underlying fields correctly
    proxy_0 = Foo{};
    // Comparing proxies work as expected
    assert(proxy_0 == foo_soa.front());
    // Comparing proxies with values also work as expected
    assert(proxy_0 == Foo{});


    // 6. Iterate over the SoA
    for (auto it : foo_soa) {
        // Iterated objects are also proxies.
        it.field<"a">() = 42;
    }

    // 7. And more, read the API! (tip: it looks A LOT like std::vector's)

    return 0;
}
```


## Integrating with CMake
You can integrate soa.hpp with CMake targets by adding a copy of this repository and linking with the `soa.hpp` target:
```cmake
add_subdirectory("path/to/soa.hpp")
target_link_libraries(my_awesome_target soa.hpp)
```
