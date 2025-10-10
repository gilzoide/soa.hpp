#ifndef __SOA_HPP__
#define __SOA_HPP__

#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include "reflect/reflect"

namespace soa {

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
			reflect::for_each<T>([&](auto I) {
				auto&& field = reflect::get<I>(value);
				auto vec = parent->template get_vector<I>();
				(*vec)[index] = field;
			});
			return *this;
		}

		T operator*() const {
			T value;
			reflect::for_each<T>([&](auto I) {
				auto vec = parent->template get_vector<I>();
				reflect::get<I>(value) = (*vec)[index];
			});
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

	soa() {
		reflect::for_each<T>([&](auto I) {
			vectors[I] = new field_vector<I>;
		});
	}
	explicit soa(size_t count) {
		reflect::for_each<T>([&](auto I) {
			vectors[I] = new field_vector<I>(count);
		});
	}
	soa(size_t count, const T& value) {
		reflect::for_each<T>([&](auto I) {
			auto&& field = reflect::get<I>(value);
			vectors[I] = new field_vector<I>(count, field);
		});
	}
	template<typename InputIt>
	soa(InputIt first, InputIt last) : soa() {
		insert(end(), first, last);
	}
	soa(const soa& other) {
		reflect::for_each<T>([&](auto I) {
			vectors[I] = new field_vector<I>(*other.get_vector<I>());
		});
	}
	soa(soa&& other) {
		reflect::for_each<T>([&](auto I) {
			vectors[I] = new field_vector<I>(std::move(*other.get_vector<I>()));
		});
	}
	soa(std::initializer_list<T> ilist) : soa() {
		insert(end(), ilist);
	}

	~soa() {
		reflect::for_each<T>([&](auto I) {
			delete get_vector<I>();
		});
	}

	soa& operator=(const soa& other) {
		if (this != &other) {
			reflect::for_each<T>([&](auto I) {
				auto vec = get_vector<I>();
				*vec = *other.get_vector<I>();
			});
		}
		return *this;
	}
	soa& operator=(soa&& other) {
		if (this != &other) {
			reflect::for_each<T>([&](auto I) {
				auto vec = get_vector<I>();
				*vec = std::move(*other.get_vector<I>());
			});
		}
		return *this;
	}
	soa& operator=(std::initializer_list<T> ilist) {
		assign(ilist);
		return *this;
	}

	void assign(size_t count, const T& value) {
		reflect::for_each<T>([&](auto I) {
			auto&& field = reflect::get<I>(value);
			auto vec = get_vector<I>();
			vec->assign(count, field);
		});
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
		return std::span(*get_vector<I>());
	}

	template<size_t I>
	auto get() const {
		return std::span(*get_vector<I>());
	}

	template<reflect::fixed_string FieldName>
	auto get() {
		return std::span(*get_vector<reflect::index_of<FieldName, T>()>());
	}

	template<reflect::fixed_string FieldName>
	auto get() const {
		return std::span(*get_vector<reflect::index_of<FieldName, T>()>());
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
		return get_vector<0>()->empty();
	}

	size_t size() const {
		return get_vector<0>()->size();
	}

	size_t max_size() const {
		return get_vector<0>()->max_size();
	}

	void reserve(size_t new_cap) {
		reflect::for_each<T>([&](auto I) {
			auto vec = get_vector<I>();
			vec->reserve(new_cap);
		});
	}

	size_t capacity() const {
		return get_vector<0>()->capacity();
	}

	void shrink_to_fit() {
		reflect::for_each<T>([&](auto I) {
			auto vec = get_vector<I>();
			vec->shrink_to_fit();
		});
	}

	// Modifiers
	void clear() {
		reflect::for_each<T>([&](auto I) {
			auto vec = get_vector<I>();
			vec->clear();
		});
	}

	iterator insert(const_iterator pos, const T& value) {
		size_t index = pos.index;
		reflect::for_each<T>([&](auto I) {
			auto&& field = reflect::get<I>(value);
			auto vec = get_vector<I>();
			vec->insert(vec->begin() + index, field);
		});
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
		for (auto it = first; it != last; ++it) {
			insert(pos++, *it);
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
		size_t index = pos.index;
		reflect::for_each<T>([&](auto I) {
			auto vec = get_vector<I>();
			vec->erase(vec->begin() + index);
		});
		return iterator(this, index);
	}
	iterator erase(const_iterator first, const_iterator last) {
		size_t first_index = first.index;
		size_t last_index = last.index;
		reflect::for_each<T>([&](auto I) {
			auto vec = get_vector<I>();
			vec->erase(vec->begin() + first_index, vec->begin() + last_index);
		});
		return iterator(this, first_index);
	}

	void push_back(const T& value) {
		reflect::for_each<T>([&](auto I) {
			auto&& field = reflect::get<I>(value);
			auto vec = get_vector<I>();
			vec->push_back(field);
		});
	}

	void pop_back() {
		reflect::for_each<T>([&](auto I) {
			auto vec = get_vector<I>();
			vec->pop_back();
		});
	}

	void resize(size_t count) {
		reflect::for_each<T>([&](auto I) {
			auto vec = get_vector<I>();
			vec->resize(count);
		});
	}
	void resize(size_t count, const T& value) {
		reflect::for_each<T>([&](auto I) {
			auto&& field = reflect::get<I>(value);
			auto vec = get_vector<I>();
			vec->resize(count, field);
		});
	}

	void swap(soa& other) noexcept {
		vectors.swap(other.vectors);
	}

private:
	std::array<void *, reflect::size<T>()> vectors;

	template<size_t I>
	using field_vector = std::vector<std::remove_cvref_t<reflect::member_type<I, T>>>;

	template<size_t I>
	auto get_vector() {
		return static_cast<field_vector<I>*>(vectors[I]);
	}

	template<size_t I>
	auto get_vector() const {
		return static_cast<const field_vector<I>*>(vectors[I]);
	}
};

}

#endif // __SOA_HPP__
