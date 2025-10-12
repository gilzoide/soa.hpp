#ifndef __SOA_HPP__
#define __SOA_HPP__

#include <cstddef>
#include <span>
#include <vector>

#include "reflect/reflect"

namespace soa {

namespace detail {
	/// Templated metafunction to transform each of a tuple's type
	template<template<typename> typename Transformer, typename... Ts>
	auto transform_tuple_types(std::tuple<Ts...>) -> std::tuple<Transformer<Ts>...>;

	/// Split aggregate type T into a tuple containing all its fields
	template<typename T>
	using to_tuple = decltype(reflect::to<std::tuple>(std::declval<T>()));

	/// Tuple of vectors, one for each field of type T
	template<typename T>
	using fields_vector_tuple = decltype(transform_tuple_types<std::vector>(std::declval<to_tuple<T>>()));
}

template<typename T>
class soa {
	template<typename _soa>
	struct _wrapper {
		_wrapper(_soa *parent, size_t index) : parent(parent), index(index) {}
		_wrapper(const _wrapper&) = default;
		_wrapper(_wrapper&&) = default;

		bool operator==(const _wrapper& other) const {
			if (parent == other.parent && index == other.index) {
				// Fast path: same reference to the same index in the same parent SoA
				return true;
			}
			else {
				// Slow path: compare by value
				return operator==(*other);
			}
		}
		bool operator==(const T& other) const {
			return operator*() == other;
		}
		bool operator!=(const _wrapper& other) const {
			return !operator==(other);
		}
		bool operator!=(const T& other) const {
			return !operator==(other);
		}

		_wrapper& operator=(const T& value) {
			parent->for_each_vector([&](auto&& vec, auto&& field) {
				vec[index] = field;
			}, value);
			return *this;
		}
		_wrapper& operator=(T&& value) {
			parent->for_each_vector([&](auto&& vec, auto&& field) {
				vec[index] = field;
			}, std::move(value));
			return *this;
		}

		T operator*() const {
			T value;
			parent->for_each_vector([&](auto&& vec, auto&& field) {
				field = vec[index];
			}, value);
			return value;
		}

		operator T() const {
			return operator*();
		}

		template<size_t I>
		auto& get() {
			return parent->template get<I>()[index];
		}

		template<reflect::fixed_string FieldName>
		auto& get() {
			return parent->template get<FieldName>()[index];
		}

	private:
		_soa *parent;
		size_t index;
	};

	template<typename _soa>
	struct _iterator {
		using _const_soa = const std::remove_const_t<_soa>;

		_iterator(_soa *parent, size_t index) : parent(parent), index(index) {}
		_iterator(const _iterator&) = default;
		_iterator(_iterator&&) = default;

		_iterator& operator++() {
			++index;
			return *this;
		}
		_iterator operator++(int) {
			_iterator previous = *this;
			++index;
			return previous;
		}

		_iterator& operator--() {
			--index;
			return *this;
		}
		_iterator operator--(int) {
			_iterator previous = *this;
			--index;
			return previous;
		}

		_iterator operator+(size_t n) const {
			return _iterator(parent, index + n);
		}

		_iterator operator-(size_t n) const {
			return _iterator(parent, index - n);
		}

		bool operator==(const _iterator& other) const {
			return parent == other.parent
				&& index == other.index;
		}
		bool operator!=(const _iterator& other) const {
			return !operator==(other);
		}

		_wrapper<_soa> operator*() {
			return _wrapper<_soa>(parent, index);
		}
		_wrapper<_const_soa> operator*() const {
			return _wrapper<_const_soa>(parent, index);
		}

		template<size_t I>
		auto& get() {
			return parent->template get<I>()[index];
		}

		template<reflect::fixed_string FieldName>
		auto& get() {
			return parent->template get<FieldName>()[index];
		}

		operator _iterator<_const_soa>() const {
			return _iterator<_const_soa>(parent, index);
		}

	private:
		friend class soa;
		_soa *parent;
		size_t index;
	};

public:
	using wrapper = _wrapper<soa>;
	using const_wrapper = _wrapper<const soa>;
	using iterator = _iterator<soa>;
	using const_iterator = _iterator<const soa>;

	soa() = default;
	explicit soa(size_t count) {
		for_each_vector([&](auto&& vec, auto&& field) {
			vec.assign(count, field);
		}, T{});
	}
	soa(size_t count, const T& value) {
		for_each_vector([&](auto&& vec, auto&& field) {
			vec.assign(count, field);
		}, value);
	}
	template<typename InputIt>
	soa(InputIt first, InputIt last) {
		insert(end(), first, last);
	}
	soa(const soa& other) = default;
	soa(soa&& other) = default;
	soa(std::initializer_list<T> ilist) {
		insert(end(), ilist);
	}

	soa& operator=(const soa& other) = default;
	soa& operator=(soa&& other) = default;
	soa& operator=(std::initializer_list<T> ilist) {
		assign(ilist);
		return *this;
	}

	void assign(size_t count, const T& value) {
		clear();
		for_each_vector([&](auto&& vec, auto&& field) {
			vec.assign(count, field);
		}, value);
	}
	template<typename InputIt>
	void assign(InputIt first, InputIt last) {
		clear();
		insert(end(), first, last);
	}
	void assign(std::initializer_list<T> ilist) {
		clear();
		insert(end(), ilist);
	}

	// Element access
	wrapper at(size_t index) {
		if (index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return wrapper(this, index);
	}
	const_wrapper at(size_t index) const {
		if (index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return const_wrapper(this, index);
	}

	wrapper operator[](size_t index) {
		return wrapper(this, index);
	}
	const_wrapper operator[](size_t index) const {
		return const_wrapper(this, index);
	}

	template<size_t I>
	auto get() {
		return std::span(get_vector<I>());
	}

	template<size_t I>
	auto get() const {
		return std::span(get_vector<I>());
	}

	template<reflect::fixed_string FieldName>
	auto get() {
		constexpr size_t I = reflect::index_of<FieldName, T>();
		return get<I>();
	}

	template<reflect::fixed_string FieldName>
	auto get() const {
		constexpr size_t I = reflect::index_of<FieldName, T>();
		return get<I>();
	}

	wrapper front() {
		return wrapper(this, 0);
	}
	const_wrapper front() const {
		return const_wrapper(this, 0);
	}

	wrapper back() {
		return wrapper(this, size() - 1);
	}
	const_wrapper back() const {
		return const_wrapper(this, size() - 1);
	}

	// Iterators
	iterator begin() {
		return iterator(this, 0);
	}
	const_iterator begin() const {
		return const_iterator(this, 0);
	}
	const_iterator cbegin() const {
		return const_iterator(this, 0);
	}

	iterator end() {
		return iterator(this, size());
	}
	const_iterator end() const {
		return const_iterator(this, size());
	}
	const_iterator cend() const {
		return const_iterator(this, size());
	}

	// Capacity
	bool empty() const {
		return get_vector<0>().empty();
	}

	size_t size() const {
		return get_vector<0>().size();
	}

	size_t max_size() const {
		return get_vector<0>().max_size();
	}

	void reserve(size_t new_cap) {
		for_each_vector([&](auto&& vec) {
			vec.reserve(new_cap);
		});
	}

	size_t capacity() const {
		return get_vector<0>().capacity();
	}

	void shrink_to_fit() {
		for_each_vector([&](auto&& vec) {
			vec.shrink_to_fit();
		});
	}

	// Modifiers
	void clear() {
		for_each_vector([&](auto&& vec) {
			vec.clear();
		});
	}

	iterator insert(const_iterator pos, const T& value) {
		size_t index = pos.index;
		for_each_vector([&](auto&& vec, auto&& field) {
			vec.insert(vec.begin() + index, field);
		}, value);
		return iterator(this, index);
	}
	iterator insert(const_iterator pos, size_t count, const T& value) {
		for (size_t i = 0; i < count; ++i) {
			insert(pos + i, value);
		}
		return iterator(this, pos.index);
	}
	template<typename InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last) {
		size_t index = pos.index;
		for (auto it = first; it != last; ++it, ++pos) {
			insert(pos, *it);
		}
		return iterator(this, index);
	}
	iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
		size_t index = pos.index;
		for (auto&& value : ilist) {
			insert(pos++, value);
		}
		return iterator(this, index);
	}

	iterator erase(const_iterator pos) {
		for_each_vector([&](auto&& vec) {
			vec.erase(vec.begin() + pos.index);
		});
		return iterator(this, pos.index);
	}
	iterator erase(const_iterator first, const_iterator last) {
		size_t first_index = first.index;
		size_t last_index = last.index;
		for_each_vector([&](auto&& vec) {
			vec.erase(vec.begin() + first_index, vec.begin() + last_index);
		});
		return iterator(this, first_index);
	}

	void push_back(const T& value) {
		for_each_vector([&](auto&& vec, auto&& field) {
			vec.push_back(field);
		}, value);
	}

	void pop_back() {
		for_each_vector([&](auto&& vec) {
			vec.pop_back();
		});
	}

	void resize(size_t count) {
		for_each_vector([&](auto&& vec) {
			vec.resize(count);
		});
	}
	void resize(size_t count, const T& value) {
		for_each_vector([&](auto&& vec, auto&& field) {
			vec.resize(count, field);
		}, value);
	}

	void swap(soa& other) noexcept {
		vectors.swap(other.vectors);
	}

private:
	detail::fields_vector_tuple<T> vectors;

	template<size_t I>
	auto& get_vector() {
		return std::get<I>(vectors);
	}

	template<size_t I>
	auto& get_vector() const {
		return std::get<I>(vectors);
	}

	template<typename Fn>
	void for_each_vector(Fn&& fn) {
		reflect::for_each<T>([&](auto I) {
			fn(get_vector<I>());
		});
	}

	template<typename Fn>
	void for_each_vector(Fn&& fn) const {
		reflect::for_each<T>([&](auto I) {
			fn(get_vector<I>());
		});
	}

	template<typename Fn, typename U>
	void for_each_vector(Fn&& fn, U&& value) {
		reflect::for_each<T>([&](auto I) {
			auto&& field = reflect::get<I>(value);
			fn(get_vector<I>(), field);
		});
	}

	template<typename Fn, typename U>
	void for_each_vector(Fn&& fn, U&& value) const {
		reflect::for_each<T>([&](auto I) {
			auto&& field = reflect::get<I>(value);
			fn(get_vector<I>(), field);
		});
	}
};

}

#endif // __SOA_HPP__
