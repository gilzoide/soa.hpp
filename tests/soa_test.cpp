#include <array>
#include <initializer_list>

#include <catch2/catch_test_macros.hpp>
#include <soa.hpp>

struct Foo {
	int i = 0;
	std::string s = "";

	bool operator==(const Foo& other) const {
		return i == other.i
			&& s == other.s;
	}
};
using FooSoA = soa::soa<Foo>;

template<typename Iterable1, typename Iterable2>
void REQUIRE_IT_EQUALS(const Iterable1& iterable1, const Iterable2& iterable2) {
	auto it1 = iterable1.begin();
	auto it2 = iterable2.begin();
	for ( ; it1 != iterable1.end() && it2 != iterable2.end(); ++it1, ++it2) {
		REQUIRE(*it1 == *it2);
	}
	REQUIRE(it1 == iterable1.end());
	REQUIRE(it2 == iterable2.end());
}

TEST_CASE("soa<Foo>") {
	Foo foo1(1, "hello 1");
	Foo foo2(2, "hello 2");
	Foo foo3(3, "hello 3");
	std::initializer_list<Foo> foo_ilist = { foo1, foo2, foo3 };
	std::array<Foo, 3> foo_array({ foo1, foo2, foo3 });

	SECTION("soa()", "[constructor]") {
		FooSoA soa;
		REQUIRE(soa.size() == 0);
		REQUIRE(soa.empty());
	}
	SECTION("soa(size_t)", "[constructor]") {
		FooSoA soa(3);
		REQUIRE(soa.size() == 3);
		REQUIRE(!soa.empty());

		REQUIRE((*soa[0]).i == 0);
		REQUIRE((*soa[1]).i == 0);
		REQUIRE((*soa[2]).i == 0);
	}
	SECTION("soa(size_t, T)", "[constructor]") {
		FooSoA soa(3, foo1);
		REQUIRE(soa.size() == 3);
		REQUIRE(!soa.empty());

		REQUIRE((*soa[0]).i == foo1.i);
		REQUIRE((*soa[1]).i == foo1.i);
		REQUIRE((*soa[2]).i == foo1.i);
	}
	SECTION("soa(InputIt, InputIt)", "[constructor]") {
		FooSoA soa(foo_array.begin(), foo_array.end());
		REQUIRE(soa.size() == 3);
		REQUIRE(!soa.empty());

		REQUIRE((*soa[0]).i == foo_array[0].i);
		REQUIRE((*soa[1]).i == foo_array[1].i);
		REQUIRE((*soa[2]).i == foo_array[2].i);

		REQUIRE_IT_EQUALS(soa, foo_array);
	}
	SECTION("soa(initializer_list)", "[constructor]") {
		FooSoA soa(foo_ilist);
		REQUIRE(soa.size() == 3);
		REQUIRE(!soa.empty());

		REQUIRE((*soa[0]).i == foo1.i);
		REQUIRE((*soa[1]).i == foo2.i);
		REQUIRE((*soa[2]).i == foo3.i);

		REQUIRE_IT_EQUALS(soa, foo_ilist);
	}
	SECTION("soa(const soa&)", "[constructor]") {
		FooSoA soa(foo_ilist);
		FooSoA soa_copy(soa);
		REQUIRE_IT_EQUALS(soa, soa_copy);
	}
	SECTION("soa(soa&&)", "[constructor]") {
		FooSoA moved_soa(foo_ilist);
		FooSoA new_soa(std::move(moved_soa));
		REQUIRE_IT_EQUALS(new_soa, foo_ilist);
		REQUIRE_IT_EQUALS(moved_soa, std::initializer_list<Foo>{});
	}

	SECTION("push_back") {
		FooSoA soa;
		soa.push_back(foo1);
		REQUIRE(soa.size() == 1);
		REQUIRE(!soa.empty());

		soa.push_back(foo2);
		REQUIRE(soa.size() == 2);
		REQUIRE(!soa.empty());
	}

	SECTION("get<>") {
		FooSoA soa(foo_ilist);
		REQUIRE(soa.get<"i">().size() == 3);
		REQUIRE(soa.get<"i">()[0] == foo_array[0].i);
		REQUIRE(soa.get<"i">()[1] == foo_array[1].i);
		REQUIRE(soa.get<"i">()[2] == foo_array[2].i);

		REQUIRE(soa.get<"s">().size() == 3);
		REQUIRE(soa.get<"s">()[0] == foo_array[0].s);
		REQUIRE(soa.get<"s">()[1] == foo_array[1].s);
		REQUIRE(soa.get<"s">()[2] == foo_array[2].s);
	}

	SECTION("iteration") {
		FooSoA soa(foo_ilist);
		REQUIRE_IT_EQUALS(soa, foo_ilist);
	}

	SECTION("wrapper comparison") {
		FooSoA soa1(foo_ilist);
		REQUIRE(soa1[0] == soa1[0]);
		REQUIRE(soa1[1] == soa1[1]);
		REQUIRE(soa1[2] == soa1[2]);

		FooSoA soa2(foo_ilist);
		REQUIRE(soa1[0] == soa2[0]);
		REQUIRE(soa1[1] == soa2[1]);
		REQUIRE(soa1[2] == soa2[2]);

		FooSoA soa_same({ foo1, foo1 });
		REQUIRE(soa_same[0] == soa_same[1]);
	}

	SECTION("wrapper assignment") {
		FooSoA soa(foo_ilist);

		// copy assignment
		Foo foo4(4, "hello 4");
		soa[1] = foo4;
		REQUIRE((*soa[1]) == foo4);

		// move assignment
		soa[2] = std::move(foo4);
		REQUIRE((*soa[2]) == Foo(4, "hello 4"));
	}
}
