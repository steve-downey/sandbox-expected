// beman/expected/expected.hpp                                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef BEMAN_EXPECTED_EXPECTED_HPP
#define BEMAN_EXPECTED_EXPECTED_HPP

#include <beman/expected/unexpected.hpp>
#include <beman/expected/bad_expected_access.hpp>

#ifndef BEMAN_EXPECTED_INCLUDED_FROM_INTERFACE_UNIT
    #include <cstdlib>
    #include <functional>
    #include <initializer_list>
    #include <memory>
    #include <type_traits>
    #include <utility>
#endif

#if defined(_MSC_VER)
    #define BEMAN_EXPECTED_TRAP() __debugbreak()
#elif defined(__has_builtin) && __has_builtin(__builtin_trap)
    #define BEMAN_EXPECTED_TRAP() __builtin_trap()
#else
    #define BEMAN_EXPECTED_TRAP() std::abort()
#endif

/***
22.8.2 Header <expected> synopsis[expected.syn]

// mostly freestanding
namespace std {
  // [expected.unexpected], class template unexpected
  template<class E> class unexpected;

  // [expected.bad], class template bad_expected_access
  template<class E> class bad_expected_access;

  // [expected.bad.void], specialization for void
  template<> class bad_expected_access<void>;

  // in-place construction of unexpected values
  struct unexpect_t {
    explicit unexpect_t() = default;
  };
  inline constexpr unexpect_t unexpect{};

  // [expected.expected], class template expected
  template<class T, class E> class expected;                                // partially freestanding

  // [expected.void], partial specialization of expected for void types
  template<class T, class E> requires is_void_v<T> class expected<T, E>;    // partially freestanding
}
 */

namespace beman {
namespace expected {

namespace detail {

template <class T>
struct is_expected_specialization : std::false_type {};

// forward-declared in primary template below; specializations added after class definition

// [expected.object.assign] reinit_expected helper
template <class NewVal, class CurVal, class... Args>
constexpr void reinit_expected(NewVal& newval, CurVal& oldval, Args&&... args) {
    if constexpr (std::is_nothrow_constructible_v<NewVal, Args...>) {
        std::destroy_at(std::addressof(oldval));
        std::construct_at(std::addressof(newval), std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<NewVal>) {
        NewVal tmp(std::forward<Args>(args)...);
        std::destroy_at(std::addressof(oldval));
        std::construct_at(std::addressof(newval), std::move(tmp));
    } else {
        CurVal tmp(std::move(oldval));
        std::destroy_at(std::addressof(oldval));
        try {
            std::construct_at(std::addressof(newval), std::forward<Args>(args)...);
        } catch (...) {
            std::construct_at(std::addressof(oldval), std::move(tmp));
            throw;
        }
    }
}

// reference_constructs_from_temporary / reference_converts_from_temporary
#ifdef __cpp_lib_reference_from_temporary
using std::reference_constructs_from_temporary_v;
using std::reference_converts_from_temporary_v;
#else
template <class To, class From>
concept reference_converts_from_temporary_v =
    std::is_reference_v<To> &&
    ((!std::is_reference_v<From> && std::is_convertible_v<std::remove_cvref_t<From>*, std::remove_cvref_t<To>*>) ||
     (std::is_lvalue_reference_v<To> && std::is_const_v<std::remove_reference_t<To>> &&
      std::is_convertible_v<From, const std::remove_cvref_t<To>&&> &&
      !std::is_convertible_v<From, std::remove_cvref_t<To>&>));

template <class To, class From>
concept reference_constructs_from_temporary_v = reference_converts_from_temporary_v<To, From>;
#endif

} // namespace detail

template <class T, class E>
class expected;

namespace detail {
template <class T, class E>
struct is_expected_specialization<expected<T, E>> : std::true_type {};

template <class T, class W>
constexpr bool converts_from_any_cvref = std::disjunction_v<std::is_constructible<T, W&>,
                                                            std::is_convertible<W&, T>,
                                                            std::is_constructible<T, W>,
                                                            std::is_convertible<W, T>,
                                                            std::is_constructible<T, const W&>,
                                                            std::is_convertible<const W&, T>,
                                                            std::is_constructible<T, const W>,
                                                            std::is_convertible<const W, T>>;
} // namespace detail

// [expected.expected], class template expected
template <class T, class E>
class expected {
    static_assert(!std::is_reference_v<T>, "T must not be a reference (use expected<T&,E> specialization)");
    static_assert(!std::is_reference_v<E>, "E must not be a reference (use expected<T,E&> specialization)");
    static_assert(!std::is_void_v<E>, "E must not be void");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>, "T must not be in_place_t");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, unexpect_t>, "T must not be unexpect_t");
    static_assert(!std::is_array_v<T>, "T must not be an array type");
    static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<T>>::value,
                  "T must not be a specialization of unexpected");
    static_assert(!std::is_array_v<E>, "E must not be an array type");

  public:
    using value_type      = T;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // -------------------------------------------------------------------------
    // [expected.object.cons] Constructors
    // -------------------------------------------------------------------------

    // Default constructor: value-initializes T
    constexpr expected() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>;

    // Copy constructor (trivial path)
    constexpr expected(const expected&)
        requires(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<E>)
    = default;

    // Copy constructor (non-trivial path)
    constexpr expected(const expected& rhs)
        requires(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E> &&
                 !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<E>));

    // Move constructor (trivial path)
    constexpr expected(expected&&) noexcept
        requires(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_constructible_v<E>)
    = default;

    // Move constructor (non-trivial path)
    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<T> && std::is_move_constructible_v<E> &&
                 !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_constructible_v<E>));

    // Converting copy constructor from expected<U, G>
    template <class U, class G>
        requires(std::is_constructible_v<T, const U&> && std::is_constructible_v<E, const G&> &&
                 (std::is_same_v<bool, std::remove_cv_t<T>> || !detail::converts_from_any_cvref<T, expected<U, G>>) &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<const U&, T> || !std::is_convertible_v<const G&, E>)
        expected(const expected<U, G>& rhs);

    // Converting move constructor from expected<U, G>
    template <class U, class G>
        requires(std::is_constructible_v<T, U> && std::is_constructible_v<E, G> &&
                 (std::is_same_v<bool, std::remove_cv_t<T>> || !detail::converts_from_any_cvref<T, expected<U, G>>) &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<U, T> || !std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs);

    // Constructor from value U&&
    template <class U = std::remove_cv_t<T>>
        requires(!std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, unexpect_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, expected> && std::is_constructible_v<T, U> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 (!std::is_same_v<bool, std::remove_cv_t<T>> ||
                  !detail::is_expected_specialization<std::remove_cvref_t<U>>::value))
    constexpr explicit(!std::is_convertible_v<U, T>) expected(U&& v) : has_val_(true) {
        std::construct_at(std::addressof(val_), std::forward<U>(v));
    }

    // Constructor from unexpected<G> const&
    template <class G>
        requires std::is_constructible_v<E, const G&>
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e);

    // Constructor from unexpected<G>&&
    template <class G>
        requires std::is_constructible_v<E, G>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e);

    // In-place constructor for value
    template <class... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr explicit expected(std::in_place_t, Args&&... args);

    // In-place constructor for value with initializer_list
    template <class U, class... Args>
        requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr explicit expected(std::in_place_t, std::initializer_list<U> il, Args&&... args);

    // In-place constructor for error
    template <class... Args>
        requires std::is_constructible_v<E, Args...>
    constexpr explicit expected(unexpect_t, Args&&... args);

    // In-place constructor for error with initializer_list
    template <class U, class... Args>
        requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args);

    // -------------------------------------------------------------------------
    // [expected.object.dtor] Destructor
    // -------------------------------------------------------------------------

    constexpr ~expected()
        requires(std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>)
    = default;

    constexpr ~expected()
        requires(!(std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>));

    // -------------------------------------------------------------------------
    // [expected.object.assign] Assignment
    // -------------------------------------------------------------------------

    // Copy assignment (trivial path)
    constexpr expected& operator=(const expected&)
        requires(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
                 std::is_trivially_destructible_v<T> && std::is_trivially_copy_constructible_v<E> &&
                 std::is_trivially_copy_assignable_v<E> && std::is_trivially_destructible_v<E>)
    = default;

    // Copy assignment (non-trivial path)
    constexpr expected& operator=(const expected& rhs)
        requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> && std::is_copy_constructible_v<E> &&
                 std::is_copy_assignable_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>) &&
                 !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
                   std::is_trivially_destructible_v<T> && std::is_trivially_copy_constructible_v<E> &&
                   std::is_trivially_copy_assignable_v<E> && std::is_trivially_destructible_v<E>));

    // Move assignment (trivial path)
    constexpr expected& operator=(expected&&) noexcept
        requires(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
                 std::is_trivially_destructible_v<T> && std::is_trivially_move_constructible_v<E> &&
                 std::is_trivially_move_assignable_v<E> && std::is_trivially_destructible_v<E>)
    = default;

    // Move assignment (non-trivial path)
    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                           std::is_nothrow_move_assignable_v<T> &&
                                                           std::is_nothrow_move_constructible_v<E> &&
                                                           std::is_nothrow_move_assignable_v<E>)
        requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T> && std::is_move_constructible_v<E> &&
                 std::is_move_assignable_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>) &&
                 !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
                   std::is_trivially_destructible_v<T> && std::is_trivially_move_constructible_v<E> &&
                   std::is_trivially_move_assignable_v<E> && std::is_trivially_destructible_v<E>));

    // Assignment from value U&&
    template <class U = std::remove_cv_t<T>>
        requires(!std::is_same_v<expected, std::remove_cvref_t<U>> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T, U> && std::is_assignable_v<T&, U> &&
                 (std::is_nothrow_constructible_v<T, U> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(U&& v) {
        if (has_val_) {
            val_ = std::forward<U>(v);
        } else {
            detail::reinit_expected(val_, unex_, std::forward<U>(v));
            has_val_ = true;
        }
        return *this;
    }

    // Assignment from unexpected<G> const&
    template <class G>
        requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&> &&
                 (std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(const unexpected<G>& e);

    // Assignment from unexpected<G>&&
    template <class G>
        requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G> &&
                 (std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_move_constructible_v<E>))
    constexpr expected& operator=(unexpected<G>&& e);

    // Emplace: destroy current value/error, construct value in-place
    template <class... Args>
        requires std::is_nothrow_constructible_v<T, Args...>
    constexpr T& emplace(Args&&... args) noexcept;

    template <class U, class... Args>
        requires std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr T& emplace(std::initializer_list<U> il, Args&&... args) noexcept;

    // -------------------------------------------------------------------------
    // [expected.object.swap] Swap
    // -------------------------------------------------------------------------

    constexpr void
    swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T> &&
                                 std::is_nothrow_move_constructible_v<E> && std::is_nothrow_swappable_v<E>)
        requires(std::is_swappable_v<T> && std::is_swappable_v<E> && std::is_move_constructible_v<T> &&
                 std::is_move_constructible_v<E> &&
                 (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>));

    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }

    // -------------------------------------------------------------------------
    // [expected.object.obs] Observers
    // -------------------------------------------------------------------------

    constexpr const T* operator->() const noexcept;
    constexpr T*       operator->() noexcept;

    constexpr const T&  operator*() const& noexcept;
    constexpr T&        operator*() & noexcept;
    constexpr const T&& operator*() const&& noexcept;
    constexpr T&&       operator*() && noexcept;

    constexpr explicit operator bool() const noexcept;
    constexpr bool     has_value() const noexcept;

    constexpr const T&  value() const&;
    constexpr T&        value() &;
    constexpr const T&& value() const&&;
    constexpr T&&       value() &&;

    constexpr const E&  error() const& noexcept;
    constexpr E&        error() & noexcept;
    constexpr const E&& error() const&& noexcept;
    constexpr E&&       error() && noexcept;

    template <class U = std::remove_cv_t<T>>
    constexpr T value_or(U&& def) const&;

    template <class U = std::remove_cv_t<T>>
    constexpr T value_or(U&& def) &&;

    template <class G = E>
    constexpr E error_or(G&& def) const&;

    template <class G = E>
    constexpr E error_or(G&& def) &&;

    // -------------------------------------------------------------------------
    // [expected.object.monadic] Monadic operations
    // -------------------------------------------------------------------------

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto and_then(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto and_then(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto and_then(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto and_then(F&& f) const&&;

    template <class F>
        requires std::is_constructible_v<T, T&>
    constexpr auto or_else(F&& f) &;
    template <class F>
        requires std::is_constructible_v<T, T&&>
    constexpr auto or_else(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<T, const T&>
    constexpr auto or_else(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<T, const T&&>
    constexpr auto or_else(F&& f) const&&;

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto transform(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto transform(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto transform(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto transform(F&& f) const&&;

    template <class F>
        requires std::is_constructible_v<T, T&>
    constexpr auto transform_error(F&& f) &;
    template <class F>
        requires std::is_constructible_v<T, T&&>
    constexpr auto transform_error(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<T, const T&>
    constexpr auto transform_error(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<T, const T&&>
    constexpr auto transform_error(F&& f) const&&;

    // -------------------------------------------------------------------------
    // [expected.object.eq] Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    template <class T2, class E2>
        requires(!std::is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return *x == *y;
        return x.error() == y.error();
    }

    template <class T2>
        requires(!detail::is_expected_specialization<T2>::value)
    friend constexpr bool operator==(const expected& x, const T2& val) {
        return x.has_value() && static_cast<bool>(*x == val);
    }

    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    bool has_val_;
    union {
        T val_;
        E unex_;
    };
};

// =============================================================================
// [expected.object.cons] Out-of-line constructor definitions
// =============================================================================

template <class T, class E>
constexpr expected<T, E>::expected() noexcept(std::is_nothrow_default_constructible_v<T>)
    requires std::is_default_constructible_v<T>
    : has_val_(true) {
    std::construct_at(std::addressof(val_));
}

template <class T, class E>
constexpr expected<T, E>::expected(const expected& rhs)
    requires(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E> &&
             !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<E>))
    : has_val_(rhs.has_val_) {
    if (has_val_)
        std::construct_at(std::addressof(val_), rhs.val_);
    else
        std::construct_at(std::addressof(unex_), rhs.unex_);
}

template <class T, class E>
constexpr expected<T, E>::expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                            std::is_nothrow_move_constructible_v<E>)
    requires(std::is_move_constructible_v<T> && std::is_move_constructible_v<E> &&
             !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_constructible_v<E>))
    : has_val_(rhs.has_val_) {
    if (has_val_)
        std::construct_at(std::addressof(val_), std::move(rhs.val_));
    else
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
}

template <class T, class E>
template <class U, class G>
    requires(std::is_constructible_v<T, const U&> && std::is_constructible_v<E, const G&> &&
             (std::is_same_v<bool, std::remove_cv_t<T>> || !detail::converts_from_any_cvref<T, expected<U, G>>) &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(const expected<U, G>& rhs) : has_val_(rhs.has_value()) {
    if (has_val_)
        std::construct_at(std::addressof(val_), *rhs);
    else
        std::construct_at(std::addressof(unex_), rhs.error());
}

template <class T, class E>
template <class U, class G>
    requires(std::is_constructible_v<T, U> && std::is_constructible_v<E, G> &&
             (std::is_same_v<bool, std::remove_cv_t<T>> || !detail::converts_from_any_cvref<T, expected<U, G>>) &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(expected<U, G>&& rhs) : has_val_(rhs.has_value()) {
    if (has_val_)
        std::construct_at(std::addressof(val_), std::move(*rhs));
    else
        std::construct_at(std::addressof(unex_), std::move(rhs.error()));
}

template <class T, class E>
template <class G>
    requires std::is_constructible_v<E, const G&>
constexpr expected<T, E>::expected(const unexpected<G>& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

template <class T, class E>
template <class G>
    requires std::is_constructible_v<E, G>
constexpr expected<T, E>::expected(unexpected<G>&& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::move(e.error()));
}

template <class T, class E>
template <class... Args>
    requires std::is_constructible_v<T, Args...>
constexpr expected<T, E>::expected(std::in_place_t, Args&&... args) : has_val_(true) {
    std::construct_at(std::addressof(val_), std::forward<Args>(args)...);
}

template <class T, class E>
template <class U, class... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
constexpr expected<T, E>::expected(std::in_place_t, std::initializer_list<U> il, Args&&... args) : has_val_(true) {
    std::construct_at(std::addressof(val_), il, std::forward<Args>(args)...);
}

template <class T, class E>
template <class... Args>
    requires std::is_constructible_v<E, Args...>
constexpr expected<T, E>::expected(unexpect_t, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::forward<Args>(args)...);
}

template <class T, class E>
template <class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
constexpr expected<T, E>::expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), il, std::forward<Args>(args)...);
}

// =============================================================================
// [expected.object.dtor] Out-of-line destructor
// =============================================================================

template <class T, class E>
constexpr expected<T, E>::~expected()
    requires(!(std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>))
{
    if (has_val_)
        std::destroy_at(std::addressof(val_));
    else
        std::destroy_at(std::addressof(unex_));
}

// =============================================================================
// [expected.object.assign] Out-of-line assignment definitions
// =============================================================================

template <class T, class E>
constexpr expected<T, E>& expected<T, E>::operator=(const expected& rhs)
    requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> && std::is_copy_constructible_v<E> &&
             std::is_copy_assignable_v<E> &&
             (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>) &&
             !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
               std::is_trivially_destructible_v<T> && std::is_trivially_copy_constructible_v<E> &&
               std::is_trivially_copy_assignable_v<E> && std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        val_ = rhs.val_;
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = rhs.unex_;
    } else if (has_val_) {
        // was value, now error
        detail::reinit_expected(unex_, val_, rhs.unex_);
        has_val_ = false;
    } else {
        // was error, now value
        detail::reinit_expected(val_, unex_, rhs.val_);
        has_val_ = true;
    }
    return *this;
}

template <class T, class E>
constexpr expected<T, E>& expected<T, E>::operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                                             std::is_nothrow_move_assignable_v<T> &&
                                                                             std::is_nothrow_move_constructible_v<E> &&
                                                                             std::is_nothrow_move_assignable_v<E>)
    requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T> && std::is_move_constructible_v<E> &&
             std::is_move_assignable_v<E> &&
             (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>) &&
             !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
               std::is_trivially_destructible_v<T> && std::is_trivially_move_constructible_v<E> &&
               std::is_trivially_move_assignable_v<E> && std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        val_ = std::move(rhs.val_);
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = std::move(rhs.unex_);
    } else if (has_val_) {
        detail::reinit_expected(unex_, val_, std::move(rhs.unex_));
        has_val_ = false;
    } else {
        detail::reinit_expected(val_, unex_, std::move(rhs.val_));
        has_val_ = true;
    }
    return *this;
}

template <class T, class E>
template <class G>
    requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&> &&
             (std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>))
constexpr expected<T, E>& expected<T, E>::operator=(const unexpected<G>& e) {
    if (!has_val_) {
        unex_ = e.error();
    } else {
        detail::reinit_expected(unex_, val_, e.error());
        has_val_ = false;
    }
    return *this;
}

template <class T, class E>
template <class G>
    requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G> &&
             (std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T> ||
              std::is_nothrow_move_constructible_v<E>))
constexpr expected<T, E>& expected<T, E>::operator=(unexpected<G>&& e) {
    if (!has_val_) {
        unex_ = std::move(e.error());
    } else {
        detail::reinit_expected(unex_, val_, std::move(e.error()));
        has_val_ = false;
    }
    return *this;
}

// =============================================================================
// [expected.object.assign] Out-of-line emplace definitions
// =============================================================================

template <class T, class E>
template <class... Args>
    requires std::is_nothrow_constructible_v<T, Args...>
constexpr T& expected<T, E>::emplace(Args&&... args) noexcept {
    if (has_val_)
        std::destroy_at(std::addressof(val_));
    else
        std::destroy_at(std::addressof(unex_));
    std::construct_at(std::addressof(val_), std::forward<Args>(args)...);
    has_val_ = true;
    return val_;
}

template <class T, class E>
template <class U, class... Args>
    requires std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>
constexpr T& expected<T, E>::emplace(std::initializer_list<U> il, Args&&... args) noexcept {
    if (has_val_)
        std::destroy_at(std::addressof(val_));
    else
        std::destroy_at(std::addressof(unex_));
    std::construct_at(std::addressof(val_), il, std::forward<Args>(args)...);
    has_val_ = true;
    return val_;
}

// =============================================================================
// [expected.object.swap] Out-of-line swap definition
// =============================================================================

template <class T, class E>
constexpr void expected<T, E>::swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                            std::is_nothrow_swappable_v<T> &&
                                                            std::is_nothrow_move_constructible_v<E> &&
                                                            std::is_nothrow_swappable_v<E>)
    requires(std::is_swappable_v<T> && std::is_swappable_v<E> && std::is_move_constructible_v<T> &&
             std::is_move_constructible_v<E> &&
             (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        using std::swap;
        swap(val_, rhs.val_);
    } else if (!has_val_ && !rhs.has_val_) {
        using std::swap;
        swap(unex_, rhs.unex_);
    } else if (has_val_) {
        // this has value, rhs has error
        if constexpr (std::is_nothrow_move_constructible_v<E>) {
            E tmp(std::move(rhs.unex_));
            std::destroy_at(std::addressof(rhs.unex_));
            if constexpr (std::is_nothrow_move_constructible_v<T>) {
                std::construct_at(std::addressof(rhs.val_), std::move(val_));
                std::destroy_at(std::addressof(val_));
                std::construct_at(std::addressof(unex_), std::move(tmp));
            } else {
                try {
                    std::construct_at(std::addressof(rhs.val_), std::move(val_));
                    std::destroy_at(std::addressof(val_));
                    std::construct_at(std::addressof(unex_), std::move(tmp));
                } catch (...) {
                    std::construct_at(std::addressof(rhs.unex_), std::move(tmp));
                    throw;
                }
            }
        } else {
            T tmp(std::move(val_));
            std::destroy_at(std::addressof(val_));
            try {
                std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
                std::destroy_at(std::addressof(rhs.unex_));
                std::construct_at(std::addressof(rhs.val_), std::move(tmp));
            } catch (...) {
                std::construct_at(std::addressof(val_), std::move(tmp));
                throw;
            }
        }
        has_val_     = false;
        rhs.has_val_ = true;
    } else {
        // this has error, rhs has value
        rhs.swap(*this);
    }
}

// =============================================================================
// [expected.object.obs] Out-of-line observer definitions
// =============================================================================

template <class T, class E>
constexpr const T* expected<T, E>::operator->() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::addressof(val_);
}

template <class T, class E>
constexpr T* expected<T, E>::operator->() noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::addressof(val_);
}

template <class T, class E>
constexpr const T& expected<T, E>::operator*() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return val_;
}

template <class T, class E>
constexpr T& expected<T, E>::operator*() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return val_;
}

template <class T, class E>
constexpr const T&& expected<T, E>::operator*() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(val_);
}

template <class T, class E>
constexpr T&& expected<T, E>::operator*() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(val_);
}

template <class T, class E>
constexpr expected<T, E>::operator bool() const noexcept {
    return has_val_;
}

template <class T, class E>
constexpr bool expected<T, E>::has_value() const noexcept {
    return has_val_;
}

template <class T, class E>
constexpr const T& expected<T, E>::value() const& {
    static_assert(std::is_copy_constructible_v<E>, "value() requires is_copy_constructible_v<E>");
    if (!has_val_)
        throw bad_expected_access<E>(unex_);
    return val_;
}

template <class T, class E>
constexpr T& expected<T, E>::value() & {
    static_assert(std::is_copy_constructible_v<E>, "value() requires is_copy_constructible_v<E>");
    if (!has_val_)
        throw bad_expected_access<E>(unex_);
    return val_;
}

template <class T, class E>
constexpr const T&& expected<T, E>::value() const&& {
    static_assert(std::is_copy_constructible_v<E> && std::is_constructible_v<E, decltype(std::move(error()))>,
                  "value() && requires E be copy-constructible and constructible from move(error())");
    if (!has_val_)
        throw bad_expected_access<E>(std::move(unex_));
    return std::move(val_);
}

template <class T, class E>
constexpr T&& expected<T, E>::value() && {
    static_assert(std::is_copy_constructible_v<E> && std::is_constructible_v<E, decltype(std::move(error()))>,
                  "value() && requires E be copy-constructible and constructible from move(error())");
    if (!has_val_)
        throw bad_expected_access<E>(std::move(unex_));
    return std::move(val_);
}

template <class T, class E>
constexpr const E& expected<T, E>::error() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return unex_;
}

template <class T, class E>
constexpr E& expected<T, E>::error() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return unex_;
}

template <class T, class E>
constexpr const E&& expected<T, E>::error() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(unex_);
}

template <class T, class E>
constexpr E&& expected<T, E>::error() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_)
        BEMAN_EXPECTED_TRAP();
#endif
    return std::move(unex_);
}

template <class T, class E>
template <class U>
constexpr T expected<T, E>::value_or(U&& def) const& {
    static_assert(std::is_copy_constructible_v<T>, "value_or requires is_copy_constructible_v<T>");
    static_assert(std::is_convertible_v<U, T>, "value_or requires is_convertible_v<U, T>");
    if (has_val_)
        return val_;
    return static_cast<T>(std::forward<U>(def));
}

template <class T, class E>
template <class U>
constexpr T expected<T, E>::value_or(U&& def) && {
    static_assert(std::is_move_constructible_v<T>, "value_or requires is_move_constructible_v<T>");
    static_assert(std::is_convertible_v<U, T>, "value_or requires is_convertible_v<U, T>");
    if (has_val_)
        return std::move(val_);
    return static_cast<T>(std::forward<U>(def));
}

template <class T, class E>
template <class G>
constexpr E expected<T, E>::error_or(G&& def) const& {
    static_assert(std::is_copy_constructible_v<E>, "error_or requires is_copy_constructible_v<E>");
    static_assert(std::is_convertible_v<G, E>, "error_or requires is_convertible_v<G, E>");
    if (!has_val_)
        return unex_;
    return static_cast<E>(std::forward<G>(def));
}

template <class T, class E>
template <class G>
constexpr E expected<T, E>::error_or(G&& def) && {
    static_assert(std::is_move_constructible_v<E>, "error_or requires is_move_constructible_v<E>");
    static_assert(std::is_convertible_v<G, E>, "error_or requires is_convertible_v<G, E>");
    if (!has_val_)
        return std::move(unex_);
    return static_cast<E>(std::forward<G>(def));
}

// =============================================================================
// [expected.object.monadic] Out-of-line monadic operation definitions
// =============================================================================

template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T, E>::and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), val_);
    return U(unexpect, unex_);
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<T, E>::and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), std::move(val_));
    return U(unexpect, std::move(unex_));
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<T, E>::and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), val_);
    return U(unexpect, unex_);
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<T, E>::and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, const T&&>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f), std::move(val_));
    return U(unexpect, std::move(unex_));
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, T&>
constexpr auto expected<T, E>::or_else(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, val_);
    return std::invoke(std::forward<F>(f), unex_);
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, T&&>
constexpr auto expected<T, E>::or_else(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, std::move(val_));
    return std::invoke(std::forward<F>(f), std::move(unex_));
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, const T&>
constexpr auto expected<T, E>::or_else(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, val_);
    return std::invoke(std::forward<F>(f), unex_);
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, const T&&>
constexpr auto expected<T, E>::or_else(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G(std::in_place, std::move(val_));
    return std::invoke(std::forward<F>(f), std::move(unex_));
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T, E>::transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_);
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), val_));
        return expected<U, E>(unexpect, unex_);
    }
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<T, E>::transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F, T&&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), std::move(val_));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_));
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), std::move(val_)));
        return expected<U, E>(unexpect, std::move(unex_));
    }
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<T, E>::transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), val_);
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_);
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), val_));
        return expected<U, E>(unexpect, unex_);
    }
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<T, E>::transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F, const T&&>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f), std::move(val_));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_));
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f), std::move(val_)));
        return expected<U, E>(unexpect, std::move(unex_));
    }
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, T&>
constexpr auto expected<T, E>::transform_error(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>(std::in_place, val_);
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), unex_));
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, T&&>
constexpr auto expected<T, E>::transform_error(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>(std::in_place, std::move(val_));
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_)));
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, const T&>
constexpr auto expected<T, E>::transform_error(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>(std::in_place, val_);
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), unex_));
}

template <class T, class E>
template <class F>
    requires std::is_constructible_v<T, const T&&>
constexpr auto expected<T, E>::transform_error(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>(std::in_place, std::move(val_));
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_)));
}

// =============================================================================
// [expected.void] Partial specialization for void value type
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
class expected<T, E> {
    static_assert(!std::is_reference_v<E>, "E must not be a reference");
    static_assert(!std::is_void_v<E>, "E must not be void");
    static_assert(!std::is_array_v<E>, "E must not be an array type");
    static_assert(std::is_same_v<std::remove_cv_t<E>, E>, "E must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<E>::value, "E must not be an unexpected<X> specialization");

  public:
    using value_type      = T;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // -------------------------------------------------------------------------
    // [expected.void.cons] Constructors
    // -------------------------------------------------------------------------

    constexpr expected() noexcept : has_val_(true) {}

    constexpr expected(const expected&)
        requires std::is_trivially_copy_constructible_v<E>
    = default;

    constexpr expected(const expected& rhs)
        requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>);

    constexpr expected(expected&&) noexcept
        requires std::is_trivially_move_constructible_v<E>
    = default;

    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>);

    // Converting constructor from expected<U, G> where is_void_v<U>
    template <class U, class G>
        requires(std::is_void_v<U> && std::is_constructible_v<E, const G&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const expected<U, G>& rhs);

    template <class U, class G>
        requires(std::is_void_v<U> && std::is_constructible_v<E, G> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
                 !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
    constexpr explicit(!std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs);

    // Constructor from unexpected<G> const&
    template <class G>
        requires std::is_constructible_v<E, const G&>
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e);

    // Constructor from unexpected<G>&&
    template <class G>
        requires std::is_constructible_v<E, G>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e);

    // In-place constructor for value (no args, just marks has-value)
    constexpr explicit expected(std::in_place_t) noexcept : has_val_(true) {}

    // In-place constructor for error
    template <class... Args>
        requires std::is_constructible_v<E, Args...>
    constexpr explicit expected(unexpect_t, Args&&... args);

    // In-place constructor for error with initializer_list
    template <class U, class... Args>
        requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args);

    // -------------------------------------------------------------------------
    // [expected.void.dtor] Destructor
    // -------------------------------------------------------------------------

    constexpr ~expected()
        requires std::is_trivially_destructible_v<E>
    = default;

    constexpr ~expected()
        requires(!std::is_trivially_destructible_v<E>);

    // -------------------------------------------------------------------------
    // [expected.void.assign] Assignment
    // -------------------------------------------------------------------------

    // Copy assignment (trivial path)
    constexpr expected& operator=(const expected&)
        requires(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    // Copy assignment (non-trivial path)
    constexpr expected& operator=(const expected& rhs)
        requires(std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E> &&
                 !(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
                   std::is_trivially_destructible_v<E>));

    // Move assignment (trivial path)
    constexpr expected& operator=(expected&&) noexcept
        requires(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    // Move assignment (non-trivial path)
    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                           std::is_nothrow_move_assignable_v<E>)
        requires(std::is_move_constructible_v<E> && std::is_move_assignable_v<E> &&
                 !(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
                   std::is_trivially_destructible_v<E>));

    template <class G>
        requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
    constexpr expected& operator=(const unexpected<G>& e);

    template <class G>
        requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
    constexpr expected& operator=(unexpected<G>&& e);

    constexpr void emplace() noexcept;

    // -------------------------------------------------------------------------
    // [expected.void.swap] Swap
    // -------------------------------------------------------------------------

    constexpr void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                std::is_nothrow_swappable_v<E>)
        requires(std::is_swappable_v<E> && std::is_move_constructible_v<E>);

    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }

    // -------------------------------------------------------------------------
    // [expected.void.obs] Observers
    // -------------------------------------------------------------------------

    constexpr explicit operator bool() const noexcept { return has_val_; }
    constexpr bool     has_value() const noexcept { return has_val_; }

    constexpr void operator*() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
    }

    constexpr void value() const&;
    constexpr void value() &&;

    constexpr const E& error() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return unex_;
    }
    constexpr E& error() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return unex_;
    }
    constexpr const E&& error() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return std::move(unex_);
    }
    constexpr E&& error() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return std::move(unex_);
    }

    template <class G = E>
    constexpr E error_or(G&& def) const&;

    template <class G = E>
    constexpr E error_or(G&& def) &&;

    // -------------------------------------------------------------------------
    // [expected.void.monadic] Monadic operations
    // -------------------------------------------------------------------------

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto and_then(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto and_then(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto and_then(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto and_then(F&& f) const&&;

    template <class F>
    constexpr auto or_else(F&& f) &;
    template <class F>
    constexpr auto or_else(F&& f) &&;
    template <class F>
    constexpr auto or_else(F&& f) const&;
    template <class F>
    constexpr auto or_else(F&& f) const&&;

    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto transform(F&& f) &;
    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto transform(F&& f) &&;
    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto transform(F&& f) const&;
    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto transform(F&& f) const&&;

    template <class F>
    constexpr auto transform_error(F&& f) &;
    template <class F>
    constexpr auto transform_error(F&& f) &&;
    template <class F>
    constexpr auto transform_error(F&& f) const&;
    template <class F>
    constexpr auto transform_error(F&& f) const&&;

    // -------------------------------------------------------------------------
    // [expected.void.eq] Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    template <class T2, class E2>
        requires std::is_void_v<T2>
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return true;
        return x.error() == y.error();
    }

    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    bool has_val_;
    union {
        E unex_;
    };
};

// =============================================================================
// [expected.void.cons] Out-of-line constructor definitions
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>::expected(const expected& rhs)
    requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), rhs.unex_);
}

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>::expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
    requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>)
    : has_val_(rhs.has_val_) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class U, class G>
    requires(std::is_void_v<U> && std::is_constructible_v<E, const G&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(const expected<U, G>& rhs) : has_val_(rhs.has_value()) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), rhs.error());
}

template <class T, class E>
    requires std::is_void_v<T>
template <class U, class G>
    requires(std::is_void_v<U> && std::is_constructible_v<E, G> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, expected<U, G> &&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G>&> &&
             !std::is_constructible_v<unexpected<E>, const expected<U, G> &&>)
constexpr expected<T, E>::expected(expected<U, G>&& rhs) : has_val_(rhs.has_value()) {
    if (!has_val_)
        std::construct_at(std::addressof(unex_), std::move(rhs.error()));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
    requires std::is_constructible_v<E, const G&>
constexpr expected<T, E>::expected(const unexpected<G>& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), e.error());
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
    requires std::is_constructible_v<E, G>
constexpr expected<T, E>::expected(unexpected<G>&& e) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::move(e.error()));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class... Args>
    requires std::is_constructible_v<E, Args...>
constexpr expected<T, E>::expected(unexpect_t, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), std::forward<Args>(args)...);
}

template <class T, class E>
    requires std::is_void_v<T>
template <class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
constexpr expected<T, E>::expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : has_val_(false) {
    std::construct_at(std::addressof(unex_), il, std::forward<Args>(args)...);
}

// =============================================================================
// [expected.void.dtor] Out-of-line destructor
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>::~expected()
    requires(!std::is_trivially_destructible_v<E>)
{
    if (!has_val_)
        std::destroy_at(std::addressof(unex_));
}

// =============================================================================
// [expected.void.assign] Out-of-line assignment definitions
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>& expected<T, E>::operator=(const expected& rhs)
    requires(std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E> &&
             !(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
               std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = rhs.unex_;
    } else if (has_val_) {
        // was value, now error
        std::construct_at(std::addressof(unex_), rhs.unex_);
        has_val_ = false;
    } else {
        // was error, now value
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
    return *this;
}

template <class T, class E>
    requires std::is_void_v<T>
constexpr expected<T, E>& expected<T, E>::operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                                             std::is_nothrow_move_assignable_v<E>)
    requires(std::is_move_constructible_v<E> && std::is_move_assignable_v<E> &&
             !(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
               std::is_trivially_destructible_v<E>))
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        unex_ = std::move(rhs.unex_);
    } else if (has_val_) {
        // was value, now error
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
        has_val_ = false;
    } else {
        // was error, now value
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
    return *this;
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
    requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
constexpr expected<T, E>& expected<T, E>::operator=(const unexpected<G>& e) {
    if (!has_val_) {
        unex_ = e.error();
    } else {
        std::construct_at(std::addressof(unex_), e.error());
        has_val_ = false;
    }
    return *this;
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
    requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
constexpr expected<T, E>& expected<T, E>::operator=(unexpected<G>&& e) {
    if (!has_val_) {
        unex_ = std::move(e.error());
    } else {
        std::construct_at(std::addressof(unex_), std::move(e.error()));
        has_val_ = false;
    }
    return *this;
}

template <class T, class E>
    requires std::is_void_v<T>
constexpr void expected<T, E>::emplace() noexcept {
    if (!has_val_) {
        std::destroy_at(std::addressof(unex_));
        has_val_ = true;
    }
}

// =============================================================================
// [expected.void.swap] Out-of-line swap definition
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr void expected<T, E>::swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                            std::is_nothrow_swappable_v<E>)
    requires(std::is_swappable_v<E> && std::is_move_constructible_v<E>)
{
    if (has_val_ && rhs.has_val_) {
        // both value: no-op
    } else if (!has_val_ && !rhs.has_val_) {
        using std::swap;
        swap(unex_, rhs.unex_);
    } else if (has_val_) {
        // this has value, rhs has error
        std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
        std::destroy_at(std::addressof(rhs.unex_));
        has_val_     = false;
        rhs.has_val_ = true;
    } else {
        // this has error, rhs has value
        rhs.swap(*this);
    }
}

// =============================================================================
// [expected.void.obs] Out-of-line observer definitions
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
constexpr void expected<T, E>::value() const& {
    static_assert(std::is_copy_constructible_v<E>, "value() requires is_copy_constructible_v<E>");
    if (!has_val_)
        throw bad_expected_access<E>(unex_);
}

template <class T, class E>
    requires std::is_void_v<T>
constexpr void expected<T, E>::value() && {
    static_assert(std::is_copy_constructible_v<E> && std::is_move_constructible_v<E>,
                  "value() && requires E be copy-constructible and move-constructible");
    if (!has_val_)
        throw bad_expected_access<E>(std::move(unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
constexpr E expected<T, E>::error_or(G&& def) const& {
    static_assert(std::is_copy_constructible_v<E>, "error_or requires is_copy_constructible_v<E>");
    static_assert(std::is_convertible_v<G, E>, "error_or requires is_convertible_v<G, E>");
    if (!has_val_)
        return unex_;
    return static_cast<E>(std::forward<G>(def));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class G>
constexpr E expected<T, E>::error_or(G&& def) && {
    static_assert(std::is_move_constructible_v<E>, "error_or requires is_move_constructible_v<E>");
    static_assert(std::is_convertible_v<G, E>, "error_or requires is_convertible_v<G, E>");
    if (!has_val_)
        return std::move(unex_);
    return static_cast<E>(std::forward<G>(def));
}

// =============================================================================
// [expected.void.monadic] Out-of-line monadic operation definitions
// =============================================================================

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T, E>::and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f));
    return U(unexpect, unex_);
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<T, E>::and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f));
    return U(unexpect, std::move(unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<T, E>::and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f));
    return U(unexpect, unex_);
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<T, E>::and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(detail::is_expected_specialization<U>::value,
                  "and_then: F must return a specialization of expected");
    static_assert(std::is_same_v<typename U::error_type, E>,
                  "and_then: F must return expected with the same error_type");
    if (has_val_)
        return std::invoke(std::forward<F>(f));
    return U(unexpect, std::move(unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
constexpr auto expected<T, E>::or_else(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G();
    return std::invoke(std::forward<F>(f), unex_);
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
constexpr auto expected<T, E>::or_else(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G();
    return std::invoke(std::forward<F>(f), std::move(unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
constexpr auto expected<T, E>::or_else(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G();
    return std::invoke(std::forward<F>(f), unex_);
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
constexpr auto expected<T, E>::or_else(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
    static_assert(detail::is_expected_specialization<G>::value, "or_else: F must return a specialization of expected");
    static_assert(std::is_same_v<typename G::value_type, T>,
                  "or_else: F must return expected with the same value_type");
    if (has_val_)
        return G();
    return std::invoke(std::forward<F>(f), std::move(unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T, E>::transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_);
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f)));
        return expected<U, E>(unexpect, unex_);
    }
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
    requires std::is_constructible_v<E, E&&>
constexpr auto expected<T, E>::transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_));
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f)));
        return expected<U, E>(unexpect, std::move(unex_));
    }
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
    requires std::is_constructible_v<E, const E&>
constexpr auto expected<T, E>::transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, unex_);
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f)));
        return expected<U, E>(unexpect, unex_);
    }
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
    requires std::is_constructible_v<E, const E&&>
constexpr auto expected<T, E>::transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    if constexpr (!std::is_void_v<U>) {
        static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>, "transform: U must not be in_place_t");
        static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
        static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                      "transform: U must not be a specialization of unexpected");
    }
    if constexpr (std::is_void_v<U>) {
        if (has_val_)
            std::invoke(std::forward<F>(f));
        if (has_val_)
            return expected<U, E>();
        return expected<U, E>(unexpect, std::move(unex_));
    } else {
        if (has_val_)
            return expected<U, E>(std::invoke(std::forward<F>(f)));
        return expected<U, E>(unexpect, std::move(unex_));
    }
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
constexpr auto expected<T, E>::transform_error(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>();
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
constexpr auto expected<T, E>::transform_error(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>();
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_)));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
constexpr auto expected<T, E>::transform_error(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>();
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), unex_));
}

template <class T, class E>
    requires std::is_void_v<T>
template <class F>
constexpr auto expected<T, E>::transform_error(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
    static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
    static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
    static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
    static_assert(!detail::is_unexpected_specialization<G>::value,
                  "transform_error: G must not be a specialization of unexpected");
    if (has_val_)
        return expected<T, G>();
    return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_)));
}

// =============================================================================
// Partial specialization: expected<T&, E> — reference value type (P2988)
// =============================================================================

template <class T, class E>
    requires std::is_lvalue_reference_v<T&>
class expected<T&, E> {
    static_assert(!std::is_reference_v<E>, "E must not be a reference");
    static_assert(!std::is_void_v<E>, "E must not be void");
    static_assert(!std::is_array_v<E>, "E must not be an array type");
    static_assert(std::is_object_v<E>, "E must be an object type");
    static_assert(std::is_same_v<std::remove_cv_t<E>, E>, "E must not be cv-qualified");

  public:
    using value_type      = T&;
    using error_type      = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    expected() = delete("expected<T&,E>: no default constructor; T& cannot be null");

    // Copy constructor (trivial path)
    constexpr expected(const expected&)
        requires std::is_trivially_copy_constructible_v<E>
    = default;

    // Copy constructor (non-trivial path)
    constexpr expected(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<E>)
        requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>)
        : has_val_(rhs.has_val_) {
        if (has_val_)
            val_ = rhs.val_;
        else
            std::construct_at(std::addressof(unex_), rhs.unex_);
    }

    // Move constructor (trivial path)
    constexpr expected(expected&&) noexcept
        requires std::is_trivially_move_constructible_v<E>
    = default;

    // Move constructor (non-trivial path)
    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<E> && !std::is_trivially_move_constructible_v<E>)
        : has_val_(rhs.has_val_) {
        if (has_val_)
            val_ = rhs.val_;
        else
            std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
    }

    // Deleted: no in-place value constructor — T& cannot be constructed in-place
    template <class... Args>
    constexpr expected(std::in_place_t, Args&&...) =
        delete("expected<T&,E>: no in_place value constructor; use expected(lvalue_ref) to bind T&");

    // Value constructor — takes U that can bind to T&
    template <class U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, expected> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr explicit(!std::is_convertible_v<U, T&>) expected(U&& u) noexcept : has_val_(true) {
        T& r = std::forward<U>(u);
        val_ = std::addressof(r);
    }

    // Deleted: binding a temporary to T& creates a dangling reference
    template <class U>
        requires(detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr expected(U&&) =
        delete("expected<T&,E>: binding a temporary to T& creates a dangling reference");

    // Converting constructor from expected<U&, G> (copy)
    template <class U, class G>
        requires(std::is_constructible_v<T&, U&> && std::is_constructible_v<E, const G&> &&
                 !detail::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&> || !std::is_convertible_v<const G&, E>)
        expected(const expected<U&, G>& rhs)
        : has_val_(rhs.has_value()) {
        if (has_val_) {
            T& r = *rhs;
            val_ = std::addressof(r);
        } else {
            std::construct_at(std::addressof(unex_), rhs.error());
        }
    }

    // Converting constructor from expected<U&, G> (move)
    template <class U, class G>
        requires(std::is_constructible_v<T&, U&> && std::is_constructible_v<E, G> &&
                 !detail::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&> || !std::is_convertible_v<G, E>) expected(expected<U&, G>&& rhs)
        : has_val_(rhs.has_value()) {
        if (has_val_) {
            T& r = *rhs;
            val_ = std::addressof(r);
        } else {
            std::construct_at(std::addressof(unex_), std::move(rhs.error()));
        }
    }

    // Constructor from unexpected<G> const&
    template <class G>
        requires std::is_constructible_v<E, const G&>
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e) : has_val_(false) {
        std::construct_at(std::addressof(unex_), e.error());
    }

    // Constructor from unexpected<G>&&
    template <class G>
        requires std::is_constructible_v<E, G>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e) : has_val_(false) {
        std::construct_at(std::addressof(unex_), std::move(e.error()));
    }

    // In-place constructor for error
    template <class... Args>
        requires std::is_constructible_v<E, Args...>
    constexpr explicit expected(unexpect_t, Args&&... args) : has_val_(false) {
        std::construct_at(std::addressof(unex_), std::forward<Args>(args)...);
    }

    // In-place constructor for error with initializer_list
    template <class U, class... Args>
        requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args) : has_val_(false) {
        std::construct_at(std::addressof(unex_), il, std::forward<Args>(args)...);
    }

    // -------------------------------------------------------------------------
    // Destructor
    // -------------------------------------------------------------------------

    constexpr ~expected()
        requires std::is_trivially_destructible_v<E>
    = default;

    constexpr ~expected()
        requires(!std::is_trivially_destructible_v<E>)
    {
        if (!has_val_)
            std::destroy_at(std::addressof(unex_));
    }

    // -------------------------------------------------------------------------
    // Assignment (rebind semantics)
    // -------------------------------------------------------------------------

    // Copy assignment (trivial path)
    constexpr expected& operator=(const expected&)
        requires(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    // Copy assignment (non-trivial path)
    constexpr expected& operator=(const expected& rhs)
        requires(std::is_copy_constructible_v<E> && std::is_copy_assignable_v<E> &&
                 !(std::is_trivially_copy_constructible_v<E> && std::is_trivially_copy_assignable_v<E> &&
                   std::is_trivially_destructible_v<E>))
    {
        if (has_val_ && rhs.has_val_) {
            val_ = rhs.val_;
        } else if (!has_val_ && !rhs.has_val_) {
            unex_ = rhs.unex_;
        } else if (has_val_) {
            std::construct_at(std::addressof(unex_), rhs.unex_);
            has_val_ = false;
        } else {
            std::destroy_at(std::addressof(unex_));
            val_     = rhs.val_;
            has_val_ = true;
        }
        return *this;
    }

    // Move assignment (trivial path)
    constexpr expected& operator=(expected&&) noexcept
        requires(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    // Move assignment (non-trivial path)
    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                           std::is_nothrow_move_assignable_v<E>)
        requires(std::is_move_constructible_v<E> && std::is_move_assignable_v<E> &&
                 !(std::is_trivially_move_constructible_v<E> && std::is_trivially_move_assignable_v<E> &&
                   std::is_trivially_destructible_v<E>))
    {
        if (has_val_ && rhs.has_val_) {
            val_ = rhs.val_;
        } else if (!has_val_ && !rhs.has_val_) {
            unex_ = std::move(rhs.unex_);
        } else if (has_val_) {
            std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
            has_val_ = false;
        } else {
            std::destroy_at(std::addressof(unex_));
            val_     = rhs.val_;
            has_val_ = true;
        }
        return *this;
    }

    // Rebind reference from lvalue
    template <class U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, expected> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr expected& operator=(U&& u) {
        if (has_val_) {
            T& r = std::forward<U>(u);
            val_ = std::addressof(r);
        } else {
            std::destroy_at(std::addressof(unex_));
            T& r     = std::forward<U>(u);
            val_     = std::addressof(r);
            has_val_ = true;
        }
        return *this;
    }

    // Assignment from unexpected<G> const&
    template <class G>
        requires(std::is_constructible_v<E, const G&> && std::is_assignable_v<E&, const G&>)
    constexpr expected& operator=(const unexpected<G>& e) {
        if (!has_val_) {
            unex_ = e.error();
        } else {
            std::construct_at(std::addressof(unex_), e.error());
            has_val_ = false;
        }
        return *this;
    }

    // Assignment from unexpected<G>&&
    template <class G>
        requires(std::is_constructible_v<E, G> && std::is_assignable_v<E&, G>)
    constexpr expected& operator=(unexpected<G>&& e) {
        if (!has_val_) {
            unex_ = std::move(e.error());
        } else {
            std::construct_at(std::addressof(unex_), std::move(e.error()));
            has_val_ = false;
        }
        return *this;
    }

    // emplace — rebind the reference
    template <class U = T>
        requires(std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr T& emplace(U&& u) noexcept {
        if (!has_val_) {
            std::destroy_at(std::addressof(unex_));
            has_val_ = true;
        }
        T& r = std::forward<U>(u);
        val_ = std::addressof(r);
        return *val_;
    }

    // -------------------------------------------------------------------------
    // Swap
    // -------------------------------------------------------------------------

    constexpr void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                std::is_nothrow_swappable_v<E>)
        requires(std::is_swappable_v<E> && std::is_move_constructible_v<E>)
    {
        if (has_val_ && rhs.has_val_) {
            std::swap(val_, rhs.val_);
        } else if (!has_val_ && !rhs.has_val_) {
            using std::swap;
            swap(unex_, rhs.unex_);
        } else if (has_val_) {
            // this has value (pointer), rhs has error
            T* tmp = val_;
            std::construct_at(std::addressof(unex_), std::move(rhs.unex_));
            std::destroy_at(std::addressof(rhs.unex_));
            rhs.val_     = tmp;
            has_val_     = false;
            rhs.has_val_ = true;
        } else {
            rhs.swap(*this);
        }
    }

    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }

    // -------------------------------------------------------------------------
    // Observers
    // -------------------------------------------------------------------------

    constexpr T* operator->() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return val_;
    }

    constexpr T& operator*() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return *val_;
    }

    constexpr explicit operator bool() const noexcept { return has_val_; }
    constexpr bool     has_value() const noexcept { return has_val_; }

    constexpr T& value() const& {
        static_assert(std::is_copy_constructible_v<E>, "value() requires is_copy_constructible_v<E>");
        if (!has_val_)
            throw bad_expected_access<E>(unex_);
        return *val_;
    }

    constexpr T& value() && {
        static_assert(std::is_copy_constructible_v<E> && std::is_move_constructible_v<E>,
                      "value() && requires E be copy and move constructible");
        if (!has_val_)
            throw bad_expected_access<E>(std::move(unex_));
        return *val_;
    }

    constexpr const E& error() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return unex_;
    }

    constexpr E& error() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return unex_;
    }

    constexpr const E&& error() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return std::move(unex_);
    }

    constexpr E&& error() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return std::move(unex_);
    }

    template <class U = std::remove_cv_t<T>>
        requires(std::is_object_v<T> && !std::is_array_v<T>)
    constexpr std::remove_cv_t<T> value_or(U&& def) const {
        using X = std::remove_cv_t<T>;
        static_assert(std::is_convertible_v<T&, X>, "value_or requires T& convertible to remove_cv_t<T>");
        static_assert(std::is_convertible_v<U, X>, "value_or requires is_convertible_v<U, remove_cv_t<T>>");
        if (has_val_)
            return *val_;
        return static_cast<X>(std::forward<U>(def));
    }

    template <class G = E>
    constexpr E error_or(G&& def) const& {
        static_assert(std::is_copy_constructible_v<E>, "error_or requires is_copy_constructible_v<E>");
        static_assert(std::is_convertible_v<G, E>, "error_or requires is_convertible_v<G, E>");
        if (!has_val_)
            return unex_;
        return static_cast<E>(std::forward<G>(def));
    }

    template <class G = E>
    constexpr E error_or(G&& def) && {
        static_assert(std::is_move_constructible_v<E>, "error_or requires is_move_constructible_v<E>");
        static_assert(std::is_convertible_v<G, E>, "error_or requires is_convertible_v<G, E>");
        if (!has_val_)
            return std::move(unex_);
        return static_cast<E>(std::forward<G>(def));
    }

    // -------------------------------------------------------------------------
    // Monadic operations
    // -------------------------------------------------------------------------

    // and_then
    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto and_then(F&& f) & {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), *val_);
        return U(unexpect, unex_);
    }

    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto and_then(F&& f) && {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), *val_);
        return U(unexpect, std::move(unex_));
    }

    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto and_then(F&& f) const& {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), *val_);
        return U(unexpect, unex_);
    }

    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto and_then(F&& f) const&& {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), *val_);
        return U(unexpect, std::move(unex_));
    }

    // or_else
    template <class F>
    constexpr auto or_else(F&& f) & {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, value_type>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(*val_);
        return std::invoke(std::forward<F>(f), unex_);
    }

    template <class F>
    constexpr auto or_else(F&& f) && {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, value_type>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(*val_);
        return std::invoke(std::forward<F>(f), std::move(unex_));
    }

    template <class F>
    constexpr auto or_else(F&& f) const& {
        using G = std::remove_cvref_t<std::invoke_result_t<F, const E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, value_type>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(*val_);
        return std::invoke(std::forward<F>(f), unex_);
    }

    template <class F>
    constexpr auto or_else(F&& f) const&& {
        using G = std::remove_cvref_t<std::invoke_result_t<F, const E&&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, value_type>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(*val_);
        return std::invoke(std::forward<F>(f), std::move(unex_));
    }

    // transform
    template <class F>
        requires std::is_constructible_v<E, E&>
    constexpr auto transform(F&& f) & {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), *val_);
            if (has_val_)
                return expected<U, E>();
            return expected<U, E>(unexpect, unex_);
        } else {
            if (has_val_)
                return expected<U, E>(std::invoke(std::forward<F>(f), *val_));
            return expected<U, E>(unexpect, unex_);
        }
    }

    template <class F>
        requires std::is_constructible_v<E, E&&>
    constexpr auto transform(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), *val_);
            if (has_val_)
                return expected<U, E>();
            return expected<U, E>(unexpect, std::move(unex_));
        } else {
            if (has_val_)
                return expected<U, E>(std::invoke(std::forward<F>(f), *val_));
            return expected<U, E>(unexpect, std::move(unex_));
        }
    }

    template <class F>
        requires std::is_constructible_v<E, const E&>
    constexpr auto transform(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), *val_);
            if (has_val_)
                return expected<U, E>();
            return expected<U, E>(unexpect, unex_);
        } else {
            if (has_val_)
                return expected<U, E>(std::invoke(std::forward<F>(f), *val_));
            return expected<U, E>(unexpect, unex_);
        }
    }

    template <class F>
        requires std::is_constructible_v<E, const E&&>
    constexpr auto transform(F&& f) const&& {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), *val_);
            if (has_val_)
                return expected<U, E>();
            return expected<U, E>(unexpect, std::move(unex_));
        } else {
            if (has_val_)
                return expected<U, E>(std::invoke(std::forward<F>(f), *val_));
            return expected<U, E>(unexpect, std::move(unex_));
        }
    }

    // transform_error
    template <class F>
    constexpr auto transform_error(F&& f) & {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T&, G>(*val_);
        return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), unex_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) && {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T&, G>(*val_);
        return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_)));
    }

    template <class F>
    constexpr auto transform_error(F&& f) const& {
        using G = std::remove_cv_t<std::invoke_result_t<F, const E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T&, G>(*val_);
        return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), unex_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) const&& {
        using G = std::remove_cv_t<std::invoke_result_t<F, const E&&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T&, G>(*val_);
        return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), std::move(unex_)));
    }

    // -------------------------------------------------------------------------
    // Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    template <class T2, class E2>
        requires(!std::is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return *x == *y;
        return x.error() == y.error();
    }

    template <class T2>
        requires(!detail::is_expected_specialization<T2>::value)
    friend constexpr bool operator==(const expected& x, const T2& val) {
        return x.has_value() && static_cast<bool>(*x == val);
    }

    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    bool has_val_;
    union {
        T* val_;
        E  unex_;
    };
};

// =============================================================================
// Partial specialization: expected<T, E&> — reference error type (P2988)
// =============================================================================

template <class T, class E>
class expected<T, E&> {
    static_assert(!std::is_void_v<T>, "T must not be void in expected<T, E&>; use expected<void, E&>");
    static_assert(!std::is_reference_v<T>, "T must not be a reference in expected<T, E&>; use expected<T&, E&>");
    static_assert(!std::is_array_v<T>, "T must not be an array type in expected<T, E&>");
    static_assert(std::is_object_v<T>, "T must be an object type in expected<T, E&>");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>, "T must not be in_place_t");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, unexpect_t>, "T must not be unexpect_t");
    static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<T>>::value,
                  "T must not be a specialization of unexpected");
    static_assert(std::is_object_v<E>, "E must be an object type in expected<T, E&>");
    static_assert(!std::is_array_v<E>, "E must not be an array type in expected<T, E&>");

  public:
    using value_type      = T;
    using error_type      = E&;
    using unexpected_type = unexpected<std::remove_cv_t<E>>;

    template <class U>
    using rebind = expected<U, E&>;

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    // Default constructor (value-initializes T)
    constexpr expected() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : has_val_(true) {
        std::construct_at(std::addressof(val_));
    }

    // Copy constructor (trivial path)
    constexpr expected(const expected&)
        requires std::is_trivially_copy_constructible_v<T>
    = default;

    // Copy constructor (non-trivial path)
    constexpr expected(const expected& rhs) noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires(std::is_copy_constructible_v<T> && !std::is_trivially_copy_constructible_v<T>)
        : has_val_(rhs.has_val_) {
        if (has_val_)
            std::construct_at(std::addressof(val_), rhs.val_);
        else
            unex_ptr_ = rhs.unex_ptr_;
    }

    // Move constructor (trivial path)
    constexpr expected(expected&&) noexcept
        requires std::is_trivially_move_constructible_v<T>
    = default;

    // Move constructor (non-trivial path)
    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires(std::is_move_constructible_v<T> && !std::is_trivially_move_constructible_v<T>)
        : has_val_(rhs.has_val_) {
        if (has_val_)
            std::construct_at(std::addressof(val_), std::move(rhs.val_));
        else
            unex_ptr_ = rhs.unex_ptr_;
    }

    // Value constructor
    template <class U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, expected> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value && std::is_constructible_v<T, U>)
    constexpr explicit(!std::is_convertible_v<U, T>) expected(U&& v) noexcept(std::is_nothrow_constructible_v<T, U>)
        : has_val_(true) {
        std::construct_at(std::addressof(val_), std::forward<U>(v));
    }

    // In-place value construction
    template <class... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr explicit expected(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : has_val_(true) {
        std::construct_at(std::addressof(val_), std::forward<Args>(args)...);
    }

    template <class U, class... Args>
        requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr explicit expected(std::in_place_t, std::initializer_list<U> il, Args&&... args) : has_val_(true) {
        std::construct_at(std::addressof(val_), il, std::forward<Args>(args)...);
    }

    // Error constructor — binds E& (no temporary allowed)
    template <class G = E>
        requires(std::is_constructible_v<E&, G &&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr explicit expected(unexpect_t, G&& err) noexcept : has_val_(false) {
        E& r      = std::forward<G>(err);
        unex_ptr_ = std::addressof(r);
    }

    // Deleted: argument cannot bind to E& (covers rvalue, const lvalue, and temp-creating cases)
    template <class G>
        requires(detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr expected(unexpect_t, G&&) =
        delete("expected<T,E&>: argument type cannot bind to non-const E&; provide a mutable lvalue of type E");

    template <class G>
        requires(!std::is_constructible_v<E&, G&&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr expected(unexpect_t, G&&) =
        delete("expected<T,E&>: argument type cannot bind to non-const E&; provide a mutable lvalue of type E");

    // Deleted: no constructor from unexpected<G> (would bind E& to temporary storage in unexpected)
    template <class G>
    constexpr expected(const unexpected<G>&) =
        delete("expected<T,E&>: no constructor from unexpected<G>; use (unexpect, lvalue_ref)");

    template <class G>
    constexpr expected(unexpected<G>&&) =
        delete("expected<T,E&>: no constructor from unexpected<G>; use (unexpect, lvalue_ref)");

    // Converting constructor from expected<U, G&> (copy) — mirrors expected<T&,E>'s from expected<U&,G>
    template <class U, class G>
        requires(std::is_constructible_v<T, const U&> && std::is_convertible_v<G&, E&>)
    constexpr explicit(!std::is_convertible_v<const U&, T> || !std::is_convertible_v<G&, E&>)
        expected(const expected<U, G&>& rhs)
        : has_val_(rhs.has_value()) {
        if (has_val_)
            std::construct_at(std::addressof(val_), *rhs);
        else {
            E& r      = rhs.error();
            unex_ptr_ = std::addressof(r);
        }
    }

    // Converting constructor from expected<U, G&> (move) — moves owned value, rebinds error pointer
    template <class U, class G>
        requires(std::is_constructible_v<T, U &&> && std::is_convertible_v<G&, E&>)
    constexpr explicit(!std::is_convertible_v<U&&, T> || !std::is_convertible_v<G&, E&>)
        expected(expected<U, G&>&& rhs)
        : has_val_(rhs.has_value()) {
        if (has_val_)
            std::construct_at(std::addressof(val_), std::move(*rhs));
        else {
            E& r      = rhs.error();
            unex_ptr_ = std::addressof(r);
        }
    }

    // -------------------------------------------------------------------------
    // Destructor
    // -------------------------------------------------------------------------

    constexpr ~expected()
        requires std::is_trivially_destructible_v<T>
    = default;

    constexpr ~expected()
        requires(!std::is_trivially_destructible_v<T>)
    {
        if (has_val_)
            std::destroy_at(std::addressof(val_));
    }

    // -------------------------------------------------------------------------
    // Assignment
    // -------------------------------------------------------------------------

    // Copy assignment (trivial path)
    constexpr expected& operator=(const expected&)
        requires(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
                 std::is_trivially_destructible_v<T>)
    = default;

    // Copy assignment (non-trivial path)
    constexpr expected& operator=(const expected& rhs)
        requires(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> &&
                 !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
                   std::is_trivially_destructible_v<T>))
    {
        if (has_val_ && rhs.has_val_) {
            val_ = rhs.val_;
        } else if (!has_val_ && !rhs.has_val_) {
            unex_ptr_ = rhs.unex_ptr_;
        } else if (has_val_) {
            std::destroy_at(std::addressof(val_));
            unex_ptr_ = rhs.unex_ptr_;
            has_val_  = false;
        } else {
            std::construct_at(std::addressof(val_), rhs.val_);
            has_val_ = true;
        }
        return *this;
    }

    // Move assignment (trivial path)
    constexpr expected& operator=(expected&&) noexcept
        requires(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
                 std::is_trivially_destructible_v<T>)
    = default;

    // Move assignment (non-trivial path)
    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                           std::is_nothrow_move_assignable_v<T>)
        requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T> &&
                 !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
                   std::is_trivially_destructible_v<T>))
    {
        if (has_val_ && rhs.has_val_) {
            val_ = std::move(rhs.val_);
        } else if (!has_val_ && !rhs.has_val_) {
            unex_ptr_ = rhs.unex_ptr_;
        } else if (has_val_) {
            std::destroy_at(std::addressof(val_));
            unex_ptr_ = rhs.unex_ptr_;
            has_val_  = false;
        } else {
            std::construct_at(std::addressof(val_), std::move(rhs.val_));
            has_val_ = true;
        }
        return *this;
    }

    // Value assignment
    template <class U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, expected> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>)
    constexpr expected& operator=(U&& v) {
        if (has_val_) {
            val_ = std::forward<U>(v);
        } else {
            std::construct_at(std::addressof(val_), std::forward<U>(v));
            has_val_ = true;
        }
        return *this;
    }

    // Deleted: no assignment from unexpected<G> (would rebind E& to temporary storage)
    template <class G>
    constexpr expected& operator=(const unexpected<G>&) =
        delete("expected<T,E&>: no assignment from unexpected<G>; copy-assign from another expected<T,E&>");

    template <class G>
    constexpr expected& operator=(unexpected<G>&&) =
        delete("expected<T,E&>: no assignment from unexpected<G>; copy-assign from another expected<T,E&>");

    // emplace — construct T in-place (nothrow required for exception safety when T is destroyed)
    template <class... Args>
        requires std::is_nothrow_constructible_v<T, Args...>
    constexpr T& emplace(Args&&... args) noexcept {
        if (has_val_)
            std::destroy_at(std::addressof(val_));
        has_val_ = false;
        std::construct_at(std::addressof(val_), std::forward<Args>(args)...);
        has_val_ = true;
        return val_;
    }

    template <class U, class... Args>
        requires std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr T& emplace(std::initializer_list<U> il, Args&&... args) noexcept {
        if (has_val_)
            std::destroy_at(std::addressof(val_));
        has_val_ = false;
        std::construct_at(std::addressof(val_), il, std::forward<Args>(args)...);
        has_val_ = true;
        return val_;
    }

    // -------------------------------------------------------------------------
    // Swap
    // -------------------------------------------------------------------------

    constexpr void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                std::is_nothrow_swappable_v<T>)
        requires(std::is_swappable_v<T> && std::is_move_constructible_v<T>)
    {
        if (has_val_ && rhs.has_val_) {
            using std::swap;
            swap(val_, rhs.val_);
        } else if (!has_val_ && !rhs.has_val_) {
            std::swap(unex_ptr_, rhs.unex_ptr_);
        } else if (has_val_) {
            // this has value, rhs has error — exchange
            E* tmp = rhs.unex_ptr_;
            std::construct_at(std::addressof(rhs.val_), std::move(val_));
            std::destroy_at(std::addressof(val_));
            unex_ptr_    = tmp;
            has_val_     = false;
            rhs.has_val_ = true;
        } else {
            rhs.swap(*this);
        }
    }

    friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y))) { x.swap(y); }

    // -------------------------------------------------------------------------
    // Observers
    // -------------------------------------------------------------------------

    constexpr T* operator->() noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return std::addressof(val_);
    }

    constexpr const T* operator->() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return std::addressof(val_);
    }

    constexpr T& operator*() & noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return val_;
    }

    constexpr const T& operator*() const& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return val_;
    }

    constexpr T&& operator*() && noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return std::move(val_);
    }

    constexpr const T&& operator*() const&& noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return std::move(val_);
    }

    constexpr explicit operator bool() const noexcept { return has_val_; }
    constexpr bool     has_value() const noexcept { return has_val_; }

    constexpr T& value() & {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>>,
                      "value() requires E to be copy constructible");
        if (!has_val_)
            throw bad_expected_access<std::remove_cv_t<E>>(*unex_ptr_);
        return val_;
    }

    constexpr const T& value() const& {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>>,
                      "value() requires E to be copy constructible");
        if (!has_val_)
            throw bad_expected_access<std::remove_cv_t<E>>(*unex_ptr_);
        return val_;
    }

    constexpr T&& value() && {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>> &&
                          std::is_move_constructible_v<std::remove_cv_t<E>>,
                      "value() && requires E to be copy and move constructible");
        if (!has_val_)
            throw bad_expected_access<std::remove_cv_t<E>>(std::move(*unex_ptr_));
        return std::move(val_);
    }

    constexpr const T&& value() const&& {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>> &&
                          std::is_move_constructible_v<std::remove_cv_t<E>>,
                      "value() const&& requires E to be copy and move constructible");
        if (!has_val_)
            throw bad_expected_access<std::remove_cv_t<E>>(std::move(*unex_ptr_));
        return std::move(val_);
    }

    // error() returns E& (shallow const — does not propagate const to the referent)
    constexpr E& error() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return *unex_ptr_;
    }

    template <class U>
        requires(std::is_copy_constructible_v<T> && std::is_convertible_v<U, T>)
    constexpr T value_or(U&& def) const& {
        return has_val_ ? val_ : static_cast<T>(std::forward<U>(def));
    }

    template <class U>
        requires(std::is_move_constructible_v<T> && std::is_convertible_v<U, T>)
    constexpr T value_or(U&& def) && {
        return has_val_ ? std::move(val_) : static_cast<T>(std::forward<U>(def));
    }

    template <class G = std::remove_cv_t<E>>
        requires(std::is_copy_constructible_v<std::remove_cv_t<E>> && std::is_convertible_v<G, std::remove_cv_t<E>>)
    constexpr std::remove_cv_t<E> error_or(G&& def) const {
        if (!has_val_)
            return *unex_ptr_;
        return static_cast<std::remove_cv_t<E>>(std::forward<G>(def));
    }

    // -------------------------------------------------------------------------
    // Monadic operations — all overloads pass E& to callables
    // -------------------------------------------------------------------------

    // and_then: f receives T (value); error propagates as E&
    template <class F>
    constexpr auto and_then(F&& f) & {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), val_);
        return U(unexpect, *unex_ptr_);
    }

    template <class F>
    constexpr auto and_then(F&& f) && {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), std::move(val_));
        return U(unexpect, *unex_ptr_);
    }

    template <class F>
    constexpr auto and_then(F&& f) const& {
        using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), val_);
        return U(unexpect, *unex_ptr_);
    }

    template <class F>
    constexpr auto and_then(F&& f) const&& {
        using U = std::remove_cvref_t<std::invoke_result_t<F, const T&&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), std::move(val_));
        return U(unexpect, *unex_ptr_);
    }

    // or_else: f receives E& (the referenced error); value propagates
    template <class F>
    constexpr auto or_else(F&& f) & {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, T>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(val_);
        return std::invoke(std::forward<F>(f), *unex_ptr_);
    }

    template <class F>
    constexpr auto or_else(F&& f) && {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, T>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(std::move(val_));
        return std::invoke(std::forward<F>(f), *unex_ptr_);
    }

    template <class F>
    constexpr auto or_else(F&& f) const& {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, T>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(val_);
        return std::invoke(std::forward<F>(f), *unex_ptr_);
    }

    template <class F>
    constexpr auto or_else(F&& f) const&& {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, T>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(std::move(val_));
        return std::invoke(std::forward<F>(f), *unex_ptr_);
    }

    // transform: f receives T (value); error propagates as E&
    template <class F>
    constexpr auto transform(F&& f) & {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), val_);
            if (has_val_)
                return expected<U, E&>();
            return expected<U, E&>(unexpect, *unex_ptr_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f), val_));
            return expected<U, E&>(unexpect, *unex_ptr_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), std::move(val_));
            if (has_val_)
                return expected<U, E&>();
            return expected<U, E&>(unexpect, *unex_ptr_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f), std::move(val_)));
            return expected<U, E&>(unexpect, *unex_ptr_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), val_);
            if (has_val_)
                return expected<U, E&>();
            return expected<U, E&>(unexpect, *unex_ptr_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f), val_));
            return expected<U, E&>(unexpect, *unex_ptr_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) const&& {
        using U = std::remove_cv_t<std::invoke_result_t<F, const T&&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), std::move(val_));
            if (has_val_)
                return expected<U, E&>();
            return expected<U, E&>(unexpect, *unex_ptr_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f), std::move(val_)));
            return expected<U, E&>(unexpect, *unex_ptr_);
        }
    }

    // transform_error: f receives E& (the referenced error); value propagates
    template <class F>
    constexpr auto transform_error(F&& f) & {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T, G>(val_);
        return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), *unex_ptr_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) && {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T, G>(std::move(val_));
        return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), *unex_ptr_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) const& {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T, G>(val_);
        return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), *unex_ptr_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) const&& {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T, G>(std::move(val_));
        return expected<T, G>(unexpect, std::invoke(std::forward<F>(f), *unex_ptr_));
    }

    // -------------------------------------------------------------------------
    // Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    template <class T2, class E2>
        requires(!std::is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return *x == *y;
        return x.error() == y.error();
    }

    template <class T2>
        requires(!detail::is_expected_specialization<T2>::value)
    friend constexpr bool operator==(const expected& x, const T2& val) {
        return x.has_value() && static_cast<bool>(*x == val);
    }

    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    bool has_val_;
    union {
        T  val_;
        E* unex_ptr_;
    };
};

// =============================================================================
// Partial specialization: expected<T&, E&> — both value and error are references (P2988)
// =============================================================================

template <class T, class E>
class expected<T&, E&> {
    static_assert(!std::is_array_v<T>, "T must not be an array type in expected<T&, E&>");
    static_assert(std::is_object_v<T>, "T must be an object type in expected<T&, E&>");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>, "T must not be in_place_t");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, unexpect_t>, "T must not be unexpect_t");
    static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<T>>::value,
                  "T must not be a specialization of unexpected");
    static_assert(std::is_object_v<E>, "E must be an object type in expected<T&, E&>");
    static_assert(!std::is_array_v<E>, "E must not be an array type in expected<T&, E&>");

  public:
    using value_type      = T&;
    using error_type      = E&;
    using unexpected_type = unexpected<std::remove_cv_t<E>>;

    template <class U>
    using rebind = expected<U, E&>;

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    expected() = delete("expected<T&,E&>: no default constructor; T& cannot be null");

    // Copy/move constructors — trivial (union holds only pointers + bool has_val_)
    constexpr expected(const expected&) = default;
    constexpr expected(expected&&)      = default;

    // Deleted: no in-place value constructor — T& cannot be constructed in-place
    template <class... Args>
    constexpr expected(std::in_place_t, Args&&...) =
        delete("expected<T&,E&>: no in_place value constructor; use expected(lvalue_ref) to bind T&");

    // Value constructor — binds T& from lvalue (dangling prevention via deleted rvalue overload)
    template <class U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, expected> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr explicit(!std::is_convertible_v<U, T&>) expected(U&& u) noexcept : has_val_(true) {
        T& r = std::forward<U>(u);
        val_ = std::addressof(r);
    }

    // Deleted: binding a temporary to T& creates a dangling reference
    template <class U>
        requires(detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr expected(U&&) =
        delete("expected<T&,E&>: binding a temporary to T& creates a dangling reference");

    // Error constructor — binds E& (no temporary allowed)
    template <class G = E>
        requires(std::is_constructible_v<E&, G &&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr explicit expected(unexpect_t, G&& err) noexcept : has_val_(false) {
        E& r  = std::forward<G>(err);
        unex_ = std::addressof(r);
    }

    // Deleted: argument cannot bind to E& (covers rvalue, const lvalue, and temp-creating cases)
    template <class G>
        requires(detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr expected(unexpect_t, G&&) =
        delete("expected<T&,E&>: argument type cannot bind to non-const E&; provide a mutable lvalue of type E");

    template <class G>
        requires(!std::is_constructible_v<E&, G&&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr expected(unexpect_t, G&&) =
        delete("expected<T&,E&>: argument type cannot bind to non-const E&; provide a mutable lvalue of type E");

    // Deleted: no constructor from unexpected<G> (would bind E& to temporary storage in unexpected)
    template <class G>
    constexpr expected(const unexpected<G>&) =
        delete("expected<T&,E&>: no constructor from unexpected<G>; use (unexpect, lvalue_ref)");

    template <class G>
    constexpr expected(unexpected<G>&&) =
        delete("expected<T&,E&>: no constructor from unexpected<G>; use (unexpect, lvalue_ref)");

    // Converting constructor from expected<U&, G&> (copy)
    template <class U, class G>
        requires(std::is_constructible_v<T&, U&> && std::is_convertible_v<G&, E&> &&
                 !detail::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&> || !std::is_convertible_v<G&, E&>)
        expected(const expected<U&, G&>& rhs)
        : has_val_(rhs.has_value()) {
        if (has_val_) {
            T& r = *rhs;
            val_ = std::addressof(r);
        } else {
            E& e  = rhs.error();
            unex_ = std::addressof(e);
        }
    }

    // Converting constructor from expected<U&, G&> (move — pointers, so same as copy)
    template <class U, class G>
        requires(std::is_constructible_v<T&, U&> && std::is_convertible_v<G&, E&> &&
                 !detail::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&> || !std::is_convertible_v<G&, E&>)
        expected(expected<U&, G&>&& rhs)
        : has_val_(rhs.has_value()) {
        if (has_val_) {
            T& r = *rhs;
            val_ = std::addressof(r);
        } else {
            E& e  = rhs.error();
            unex_ = std::addressof(e);
        }
    }

    // -------------------------------------------------------------------------
    // Destructor — trivial (union holds only pointers)
    // -------------------------------------------------------------------------

    constexpr ~expected() = default;

    // -------------------------------------------------------------------------
    // Assignment
    // -------------------------------------------------------------------------

    // Copy/move — trivial (just pointers + bool)
    constexpr expected& operator=(const expected&) = default;
    constexpr expected& operator=(expected&&)      = default;

    // Value rebind — rebinds T* (no destruction needed, pointer is trivial)
    template <class U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, expected> &&
                 !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value &&
                 std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr expected& operator=(U&& u) noexcept {
        T& r     = std::forward<U>(u);
        val_     = std::addressof(r);
        has_val_ = true;
        return *this;
    }

    // Deleted: no assignment from unexpected<G>
    template <class G>
    constexpr expected& operator=(const unexpected<G>&) =
        delete("expected<T&,E&>: no assignment from unexpected<G>; copy-assign from another expected<T&,E&>");

    template <class G>
    constexpr expected& operator=(unexpected<G>&&) =
        delete("expected<T&,E&>: no assignment from unexpected<G>; copy-assign from another expected<T&,E&>");

    // emplace — rebind T& (pointer transition is trivial)
    template <class U = T>
        requires(std::is_constructible_v<T&, U> && !detail::reference_constructs_from_temporary_v<T&, U>)
    constexpr T& emplace(U&& u) noexcept {
        T& r     = std::forward<U>(u);
        val_     = std::addressof(r);
        has_val_ = true;
        return *val_;
    }

    // -------------------------------------------------------------------------
    // Swap
    // -------------------------------------------------------------------------

    constexpr void swap(expected& rhs) noexcept {
        if (has_val_ && rhs.has_val_) {
            std::swap(val_, rhs.val_);
        } else if (!has_val_ && !rhs.has_val_) {
            std::swap(unex_, rhs.unex_);
        } else if (has_val_) {
            T* my_val    = val_;
            E* rhs_err   = rhs.unex_;
            unex_        = rhs_err;
            rhs.val_     = my_val;
            has_val_     = false;
            rhs.has_val_ = true;
        } else {
            rhs.swap(*this);
        }
    }

    friend constexpr void swap(expected& x, expected& y) noexcept { x.swap(y); }

    // -------------------------------------------------------------------------
    // Observers — shallow const on both sides (references don't propagate const)
    // -------------------------------------------------------------------------

    constexpr T* operator->() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return val_;
    }

    constexpr T& operator*() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return *val_;
    }

    constexpr explicit operator bool() const noexcept { return has_val_; }
    constexpr bool     has_value() const noexcept { return has_val_; }

    constexpr T& value() const& {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>>,
                      "value() requires E to be copy constructible");
        if (!has_val_)
            throw bad_expected_access<std::remove_cv_t<E>>(*unex_);
        return *val_;
    }

    constexpr T& value() && {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>>,
                      "value() requires E to be copy constructible");
        if (!has_val_)
            throw bad_expected_access<std::remove_cv_t<E>>(*unex_);
        return *val_;
    }

    // error() — shallow const: always returns E& regardless of const on expected
    constexpr E& error() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return *unex_;
    }

    template <class U = std::remove_cv_t<T>>
        requires(std::is_object_v<T> && !std::is_array_v<T>)
    constexpr std::remove_cv_t<T> value_or(U&& def) const {
        using X = std::remove_cv_t<T>;
        static_assert(std::is_convertible_v<T&, X>, "value_or requires T& convertible to remove_cv_t<T>");
        static_assert(std::is_convertible_v<U, X>, "value_or requires is_convertible_v<U, remove_cv_t<T>>");
        if (has_val_)
            return *val_;
        return static_cast<X>(std::forward<U>(def));
    }

    template <class G = std::remove_cv_t<E>>
    constexpr std::remove_cv_t<E> error_or(G&& def) const {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>>,
                      "error_or requires E to be copy constructible");
        static_assert(std::is_convertible_v<G, std::remove_cv_t<E>>,
                      "error_or requires is_convertible_v<G, remove_cv_t<E>>");
        if (!has_val_)
            return *unex_;
        return static_cast<std::remove_cv_t<E>>(std::forward<G>(def));
    }

    // -------------------------------------------------------------------------
    // Monadic operations — T& value side, E& error side (shallow const on both)
    // -------------------------------------------------------------------------

    // and_then: f receives T& (value); error propagates as E&
    template <class F>
    constexpr auto and_then(F&& f) & {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), *val_);
        return U(unexpect, *unex_);
    }

    template <class F>
    constexpr auto and_then(F&& f) && {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), *val_);
        return U(unexpect, *unex_);
    }

    template <class F>
    constexpr auto and_then(F&& f) const& {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), *val_);
        return U(unexpect, *unex_);
    }

    template <class F>
    constexpr auto and_then(F&& f) const&& {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f), *val_);
        return U(unexpect, *unex_);
    }

    // or_else: f receives E& (the referenced error); value propagates as T&
    template <class F>
    constexpr auto or_else(F&& f) & {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, value_type>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(*val_);
        return std::invoke(std::forward<F>(f), *unex_);
    }

    template <class F>
    constexpr auto or_else(F&& f) && {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, value_type>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(*val_);
        return std::invoke(std::forward<F>(f), *unex_);
    }

    template <class F>
    constexpr auto or_else(F&& f) const& {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, value_type>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(*val_);
        return std::invoke(std::forward<F>(f), *unex_);
    }

    template <class F>
    constexpr auto or_else(F&& f) const&& {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_same_v<typename G::value_type, value_type>,
                      "or_else: F must return expected with the same value_type");
        if (has_val_)
            return G(*val_);
        return std::invoke(std::forward<F>(f), *unex_);
    }

    // transform: f receives T& (value); error propagates as E&; result is expected<U, E&>
    template <class F>
    constexpr auto transform(F&& f) & {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), *val_);
            if (has_val_)
                return expected<U, E&>();
            return expected<U, E&>(unexpect, *unex_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f), *val_));
            return expected<U, E&>(unexpect, *unex_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), *val_);
            if (has_val_)
                return expected<U, E&>();
            return expected<U, E&>(unexpect, *unex_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f), *val_));
            return expected<U, E&>(unexpect, *unex_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), *val_);
            if (has_val_)
                return expected<U, E&>();
            return expected<U, E&>(unexpect, *unex_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f), *val_));
            return expected<U, E&>(unexpect, *unex_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) const&& {
        using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f), *val_);
            if (has_val_)
                return expected<U, E&>();
            return expected<U, E&>(unexpect, *unex_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f), *val_));
            return expected<U, E&>(unexpect, *unex_);
        }
    }

    // transform_error: f receives E& (the referenced error); value propagates as T&; result is expected<T&, G>
    template <class F>
    constexpr auto transform_error(F&& f) & {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T&, G>(*val_);
        return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), *unex_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) && {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T&, G>(*val_);
        return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), *unex_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) const& {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T&, G>(*val_);
        return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), *unex_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) const&& {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<T&, G>(*val_);
        return expected<T&, G>(unexpect, std::invoke(std::forward<F>(f), *unex_));
    }

    // -------------------------------------------------------------------------
    // Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    template <class T2, class E2>
        requires(!std::is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return *x == *y;
        return x.error() == y.error();
    }

    template <class T2>
        requires(!detail::is_expected_specialization<T2>::value)
    friend constexpr bool operator==(const expected& x, const T2& val) {
        return x.has_value() && static_cast<bool>(*x == val);
    }

    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    bool has_val_;
    union {
        T* val_;
        E* unex_;
    };
};

// =============================================================================
// Partial specialization: expected<void, E&> — void value + reference error type (P2988)
// =============================================================================

template <class E>
class expected<void, E&> {
    static_assert(std::is_object_v<E>, "E must be an object type in expected<void, E&>");
    static_assert(!std::is_array_v<E>, "E must not be an array type in expected<void, E&>");

  public:
    using value_type      = void;
    using error_type      = E&;
    using unexpected_type = unexpected<std::remove_cv_t<E>>;

    template <class U>
    using rebind = expected<U, E&>;

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    // Default constructor — void/success state
    constexpr expected() noexcept : unex_ptr_(nullptr), has_val_(true) {}

    // Copy/move — trivial (just pointer + bool)
    constexpr expected(const expected&)     = default;
    constexpr expected(expected&&) noexcept = default;

    // In-place value constructor
    constexpr explicit expected(std::in_place_t) noexcept : unex_ptr_(nullptr), has_val_(true) {}

    // Error constructor from unexpected<G> const& — E& binds to G's stored value via its lvalue accessor
    template <class G>
        requires(std::is_constructible_v<E&, G&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr explicit(!std::is_convertible_v<G&, E&>) expected(const unexpected<G>& e) noexcept : has_val_(false) {
        E& r      = const_cast<G&>(e.error());
        unex_ptr_ = std::addressof(r);
    }

    // Error constructor from unexpected<G>&& (non-const lvalue accessor gives G&, binds to E&)
    template <class G>
        requires(std::is_constructible_v<E&, G&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr explicit(!std::is_convertible_v<G&, E&>) expected(unexpected<G>&& e) noexcept : has_val_(false) {
        E& r      = e.error();
        unex_ptr_ = std::addressof(r);
    }

    // In-place error constructor — binds E& directly (no temporary allowed)
    template <class G = E>
        requires(std::is_constructible_v<E&, G &&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr explicit expected(unexpect_t, G&& err) noexcept : has_val_(false) {
        E& r      = std::forward<G>(err);
        unex_ptr_ = std::addressof(r);
    }

    // Deleted: argument cannot bind to E& (covers rvalue, const lvalue, and temp-creating cases)
    template <class G>
        requires(detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr expected(unexpect_t, G&&) =
        delete("expected<void,E&>: argument type cannot bind to non-const E&; provide a mutable lvalue of type E");

    template <class G>
        requires(!std::is_constructible_v<E&, G&&> && !detail::reference_constructs_from_temporary_v<E&, G>)
    constexpr expected(unexpect_t, G&&) =
        delete("expected<void,E&>: argument type cannot bind to non-const E&; provide a mutable lvalue of type E");

    // Converting constructor from expected<void, G&>
    template <class G>
        requires std::is_convertible_v<G&, E&>
    constexpr explicit(!std::is_convertible_v<G&, E&>) expected(const expected<void, G&>& rhs)
        : has_val_(rhs.has_value()) {
        if (!has_val_) {
            E& r      = rhs.error();
            unex_ptr_ = std::addressof(r);
        }
    }

    // -------------------------------------------------------------------------
    // Destructor — trivial (pointer + bool only)
    // -------------------------------------------------------------------------

    constexpr ~expected() = default;

    // -------------------------------------------------------------------------
    // Assignment
    // -------------------------------------------------------------------------

    // Copy/move — trivial
    constexpr expected& operator=(const expected&)     = default;
    constexpr expected& operator=(expected&&) noexcept = default;

    // emplace — transition to void success state (always noexcept)
    constexpr void emplace() noexcept { has_val_ = true; }

    // -------------------------------------------------------------------------
    // Swap
    // -------------------------------------------------------------------------

    constexpr void swap(expected& rhs) noexcept {
        if (has_val_ && rhs.has_val_) {
            // both success: nothing to do
        } else if (!has_val_ && !rhs.has_val_) {
            std::swap(unex_ptr_, rhs.unex_ptr_);
        } else if (has_val_) {
            unex_ptr_     = rhs.unex_ptr_;
            rhs.unex_ptr_ = nullptr;
            has_val_      = false;
            rhs.has_val_  = true;
        } else {
            rhs.swap(*this);
        }
    }

    friend constexpr void swap(expected& x, expected& y) noexcept { x.swap(y); }

    // -------------------------------------------------------------------------
    // Observers
    // -------------------------------------------------------------------------

    constexpr explicit operator bool() const noexcept { return has_val_; }
    constexpr bool     has_value() const noexcept { return has_val_; }

    constexpr void operator*() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (!has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
    }

    constexpr void value() const& {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>>,
                      "value() requires E to be copy constructible");
        if (!has_val_)
            throw bad_expected_access<std::remove_cv_t<E>>(*unex_ptr_);
    }

    constexpr void value() && {
        static_assert(std::is_copy_constructible_v<std::remove_cv_t<E>> &&
                          std::is_move_constructible_v<std::remove_cv_t<E>>,
                      "value() && requires E to be copy and move constructible");
        if (!has_val_)
            throw bad_expected_access<std::remove_cv_t<E>>(*unex_ptr_);
    }

    // error() — shallow const: always returns E& regardless of const on expected
    constexpr E& error() const noexcept {
#if defined(BEMAN_EXPECTED_HARDENED)
        if (has_val_)
            BEMAN_EXPECTED_TRAP();
#endif
        return *unex_ptr_;
    }

    template <class G = std::remove_cv_t<E>>
        requires(std::is_copy_constructible_v<std::remove_cv_t<E>> && std::is_convertible_v<G, std::remove_cv_t<E>>)
    constexpr std::remove_cv_t<E> error_or(G&& def) const {
        if (!has_val_)
            return *unex_ptr_;
        return static_cast<std::remove_cv_t<E>>(std::forward<G>(def));
    }

    // -------------------------------------------------------------------------
    // Deleted: value_or is not available for void expected
    template <class U>
    constexpr void value_or(U&&) const =
        delete("expected<void,E&>: no value_or for void specialization");

    // Monadic operations — void value + E& error
    // -------------------------------------------------------------------------

    // and_then: F called with no args (void value); error propagates as E&
    template <class F>
    constexpr auto and_then(F&& f) & {
        using U = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f));
        return U(unexpect, *unex_ptr_);
    }

    template <class F>
    constexpr auto and_then(F&& f) && {
        using U = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f));
        return U(unexpect, *unex_ptr_);
    }

    template <class F>
    constexpr auto and_then(F&& f) const& {
        using U = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f));
        return U(unexpect, *unex_ptr_);
    }

    template <class F>
    constexpr auto and_then(F&& f) const&& {
        using U = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(detail::is_expected_specialization<U>::value,
                      "and_then: F must return a specialization of expected");
        static_assert(std::is_same_v<typename U::error_type, E&>,
                      "and_then: F must return expected with the same error_type");
        if (has_val_)
            return std::invoke(std::forward<F>(f));
        return U(unexpect, *unex_ptr_);
    }

    // or_else: F receives E& (the referenced error); value propagates as void success
    template <class F>
    constexpr auto or_else(F&& f) & {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_void_v<typename G::value_type>, "or_else: F must return expected with void value_type");
        if (has_val_)
            return G();
        return std::invoke(std::forward<F>(f), *unex_ptr_);
    }

    template <class F>
    constexpr auto or_else(F&& f) && {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_void_v<typename G::value_type>, "or_else: F must return expected with void value_type");
        if (has_val_)
            return G();
        return std::invoke(std::forward<F>(f), *unex_ptr_);
    }

    template <class F>
    constexpr auto or_else(F&& f) const& {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_void_v<typename G::value_type>, "or_else: F must return expected with void value_type");
        if (has_val_)
            return G();
        return std::invoke(std::forward<F>(f), *unex_ptr_);
    }

    template <class F>
    constexpr auto or_else(F&& f) const&& {
        using G = std::remove_cvref_t<std::invoke_result_t<F, E&>>;
        static_assert(detail::is_expected_specialization<G>::value,
                      "or_else: F must return a specialization of expected");
        static_assert(std::is_void_v<typename G::value_type>, "or_else: F must return expected with void value_type");
        if (has_val_)
            return G();
        return std::invoke(std::forward<F>(f), *unex_ptr_);
    }

    // transform: F called with no args; error propagates as E&
    template <class F>
    constexpr auto transform(F&& f) & {
        using U = std::remove_cv_t<std::invoke_result_t<F>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f));
            if (has_val_)
                return expected<void, E&>();
            return expected<void, E&>(unexpect, *unex_ptr_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f)));
            return expected<U, E&>(unexpect, *unex_ptr_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f));
            if (has_val_)
                return expected<void, E&>();
            return expected<void, E&>(unexpect, *unex_ptr_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f)));
            return expected<U, E&>(unexpect, *unex_ptr_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) const& {
        using U = std::remove_cv_t<std::invoke_result_t<F>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f));
            if (has_val_)
                return expected<void, E&>();
            return expected<void, E&>(unexpect, *unex_ptr_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f)));
            return expected<U, E&>(unexpect, *unex_ptr_);
        }
    }

    template <class F>
    constexpr auto transform(F&& f) const&& {
        using U = std::remove_cv_t<std::invoke_result_t<F>>;
        if constexpr (!std::is_void_v<U>) {
            static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                          "transform: U must not be in_place_t");
            static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>, "transform: U must not be unexpect_t");
            static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                          "transform: U must not be a specialization of unexpected");
        }
        if constexpr (std::is_void_v<U>) {
            if (has_val_)
                std::invoke(std::forward<F>(f));
            if (has_val_)
                return expected<void, E&>();
            return expected<void, E&>(unexpect, *unex_ptr_);
        } else {
            if (has_val_)
                return expected<U, E&>(std::invoke(std::forward<F>(f)));
            return expected<U, E&>(unexpect, *unex_ptr_);
        }
    }

    // transform_error: F receives E&; value propagates as void success
    template <class F>
    constexpr auto transform_error(F&& f) & {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<void, G>();
        return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), *unex_ptr_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) && {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<void, G>();
        return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), *unex_ptr_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) const& {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<void, G>();
        return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), *unex_ptr_));
    }

    template <class F>
    constexpr auto transform_error(F&& f) const&& {
        using G = std::remove_cv_t<std::invoke_result_t<F, E&>>;
        static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
        static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
        static_assert(std::is_same_v<G, std::remove_cv_t<G>>, "transform_error: G must not be cv-qualified");
        static_assert(!detail::is_unexpected_specialization<G>::value,
                      "transform_error: G must not be a specialization of unexpected");
        if (has_val_)
            return expected<void, G>();
        return expected<void, G>(unexpect, std::invoke(std::forward<F>(f), *unex_ptr_));
    }

    // -------------------------------------------------------------------------
    // Equality operators (hidden friends)
    // -------------------------------------------------------------------------

    template <class T2, class E2>
        requires std::is_void_v<T2>
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y) {
        if (x.has_value() != y.has_value())
            return false;
        if (x.has_value())
            return true;
        return x.error() == y.error();
    }

    template <class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e) {
        return !x.has_value() && static_cast<bool>(x.error() == e.error());
    }

  private:
    E*   unex_ptr_;
    bool has_val_;
};

} // namespace expected
} // namespace beman

#undef BEMAN_EXPECTED_TRAP

#endif
