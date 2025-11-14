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

	operator bool() const {
		return i != 0 || !s.empty();
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

	SECTION("pop_back") {
		FooSoA soa(foo_ilist);
		REQUIRE(soa.size() == 3);
		soa.pop_back();
		REQUIRE(soa.size() == 2);
	}

	SECTION("erase") {
		FooSoA soa(foo_ilist);
		REQUIRE(soa.size() == 3);
		soa.erase(soa.begin());
		REQUIRE(soa.size() == 2);
		REQUIRE(soa[0] == foo2);
		REQUIRE(soa[1] == foo3);
	}

	SECTION("resize") {
		FooSoA soa(foo_ilist);
		REQUIRE(soa.size() == 3);
		soa.resize(1);
		REQUIRE(soa.size() == 1);
	}

	SECTION("swap") {
		FooSoA soa(foo_ilist);
		FooSoA soa2;
		REQUIRE(soa.size() == 3);
		REQUIRE(soa2.size() == 0);
		soa.swap(soa2);
		REQUIRE(soa.size() == 0);
		REQUIRE(soa2.size() == 3);
	}

	SECTION("get<>") {
		FooSoA soa(foo_ilist);
		REQUIRE(soa.field<"i">().size() == 3);
		REQUIRE(soa.field<"i">()[0] == foo_array[0].i);
		REQUIRE(soa.field<"i">()[1] == foo_array[1].i);
		REQUIRE(soa.field<"i">()[2] == foo_array[2].i);

		REQUIRE(soa.field<"s">().size() == 3);
		REQUIRE(soa.field<"s">()[0] == foo_array[0].s);
		REQUIRE(soa.field<"s">()[1] == foo_array[1].s);
		REQUIRE(soa.field<"s">()[2] == foo_array[2].s);

		REQUIRE_IT_EQUALS(soa.field<"i">(), soa.field<0>());
		REQUIRE_IT_EQUALS(soa.field<"i">(), soa.field<int>());
	}

	SECTION("iteration") {
		FooSoA soa(foo_ilist);
		REQUIRE_IT_EQUALS(soa, foo_ilist);
	}

	SECTION("wrapper") {
		SECTION("comparison") {
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

		SECTION("assignment(T)") {
			FooSoA soa(foo_ilist);

			// copy assignment
			Foo foo4(4, "hello 4");
			soa[1] = foo4;
			REQUIRE(soa[1] == foo4);

			// move assignment
			soa[2] = std::move(foo4);
			REQUIRE(soa[2] == Foo(4, "hello 4"));
		}

		SECTION("assignment(wrapper)") {
			FooSoA soa(foo_ilist);
			soa[1] = soa[2];
			REQUIRE(soa[1] == soa[2]);
		}

		SECTION("operator bool") {
			FooSoA soa;
			soa.push_back(Foo());
			REQUIRE(!soa[0].value());
			soa.push_back(Foo(1, "hello"));
			REQUIRE(soa[1].value());
		}

		SECTION("field") {
			FooSoA soa(foo_ilist);
			REQUIRE(soa[0].field<"i">() == foo_array[0].i);
			REQUIRE(soa[1].field<"i">() == foo_array[1].i);
			REQUIRE(soa[2].field<"i">() == foo_array[2].i);

			REQUIRE(soa[0].field<"s">() == foo_array[0].s);
			REQUIRE(soa[1].field<"s">() == foo_array[1].s);
			REQUIRE(soa[2].field<"s">() == foo_array[2].s);
		}
	}

	SECTION("swap") {
		FooSoA soa(foo_ilist);
		using std::swap;
		swap(soa[0], soa[1]);
		REQUIRE(soa[0] == foo2);
		REQUIRE(soa[1] == foo1);
	}
}
