# Changes Since Last Version

- **Changes since R10**

  - Wording changes from LWG review.

- **Changes since R9**

  - Fix cast in wording removing base/derived UB

# Comparison table

## Using a raw pointer result for an element search function

This is the convention the C++ core guidelines suggest, to use a raw pointer for representing optional non-owning references. However, there is a user-required check against , no type safety meaning no safety against mis-interpreting such a raw pointer, for example by using pointer arithmetic on it.

## returning result of an element search function via a (smart) pointer

The disadvantage here is that is both non-standard and not well named, therefore this example uses that would have the advantage of avoiding dangling through potential lifetime extension. However, on the downside is still the explicit checks against the on the client side, failing so risks undefined behavior.

## returning result of an element search function via an iterator

This might be the obvious choice, for example, for associative containers, especially since their iterator stability guarantees. However, returning such an iterator will leak the underlying container type as well necessarily requires one to know the sentinel of the container to check for the not-found case.

## Using an optional\<T\*\> as a substitute for optional\<T&\>

This approach adds another level of indirection and requires two checks to take a definite action.

# Motivation

Other than the standard library’s implementation of optional, optionals holding references are common. The desire for such a feature is well understood, and many optional types in commonly used libraries provide it, with the semantics proposed here. One standard library implementation already provides an implementation of but disables its use, because the standard forbids it.

The research in JeanHeyd Meneide’s \_References for Standard Library Vocabulary Types - an optional case study.\_ shows conclusively that rebind semantics are the only safe semantic as assign through on engaged is too bug-prone. Implementations that attempt assign-through are abandoned. The standard library should follow existing practice and supply an that rebinds on assignment.

Additional background reading on can be found in JeanHeyd Meneide’s article \_To Bind and Loose a Reference\_ .

In freestanding environments or for safety-critical libraries, an optional type over references is important to implement containers, that otherwise as the standard library either would cause undefined behavior when accessing an non-available element, throw an exception, or silently create the element. Returning a plain pointer for such an optional reference, as the core guidelines suggest, is a non-type-safe solution and doesn’t protect in any way from accessing an non-existing element by a de-reference. In addition, the monadic APIs of makes is especially attractive by streamlining client code receiving such an optional reference, in contrast to a pointer that requires an explicit nullptr check and de-reference.

There is a principled reason not to provide a partial specialization over as the semantics are in some ways subtly different than the primary template. Assignment may have side-effects not present in the primary, which has pure value semantics. However, I argue this is misleading, as reference semantics often has side-effects. The proposed semantic is similar to what an provides, with much greater usability.

There are well motivated suggestions that perhaps instead of an there should be an that is an independent primary template. This proposal rejects that, because we need a policy over all sum types as to how reference semantics should work, as optional is a variant over T and monostate. That the library sum type can not express the same range of types as the product type, tuple, is an increasing problem as we add more types logically equivalent to a variant. The template types and should behave as extensions of and , or we lose the ability to reason about generic types.

That we can’t guarantee from (product type) that (sum type) is valid, is a problem, and one that reflection can’t solve. A language sum type could, but we need agreement on the semantics.

The semantics of a variant with a reference are as if it holds the address of the referent when referring to that referent. All other semantics are worse. Not being able to express a variant\<T&\> is inconsistent, hostile, and strictly worse than disallowing it.

Thus, we expect future papers to propose and with the ability to hold references. The latter can be used as an iteration type over elements.

# Design

The design is straightforward. The holds a pointer to the underlying object of type , or if the optional is disengaged. The implementation is simple, especially with C++20 and up techniques, using concept constraints. As the held pointer is a primitive regular type with reference semantics, many operations can be defaulted and are by nature. See and . The implementation is less than 200 lines of code, much of it the monadic functions with identical textual implementations with different signatures and different overloads being called.

In place construction is not supported as it would just be a way of providing immediate life-time issues.

## Relational Operations

The definitions of the relational operators are the same as for the base template. Interoperable comparisons between T and optional\<T&\> work as expected. This is not true for the boost optional\<T&\>.

## make_optional

With further research, the existing uses of make_optional\<X&\> seem to be primarily test cases, and deliberate use seems to be exceedingly rare in the wild. Reflector review was much more positive about removing the misleading ability to create an via . In addition, the multiple argument forms can be used to attempt to construct a optional that contains a reference, but this becomes ill formed because of existing mandates at the type level. In order to preserve existing behavior, where make_optional is not well formed if it constructs a reference, changes to should be made.

Adding a non-type template parameter as the first template parameter to the single argument and mandating that the multi-argument version not request a reference type as the parameter, will diagnose mistaken use of and preserve the existing behavior.

Since construction of an object in order to make a reference to it to construct an optional containing a reference would always dangle, there do not seem to be any use cases for the multi-argument or initializer list forms of make_optional for reference types, and the constructor form seems to satisfy all cases for single argument construction of a optional containing a reference, there does not seem to be a need for a factory function for optional over reference.

There was also discussion of using to indicate reference use, in analogy with std::tuple. Unfortunately there are existing uses of optional over reference_wrapper as a workaround for lack of reference specialization, and it would be a breaking change for such code.

## Trivial construction

Construction of should be trivial, because it is straightforward to implement, and is trivial. Boost is not.

## Value Category Affects value()

For several implementations there are distinct overloads for functions depending on value category, with the same implementation. However, this makes it very easy to accidentally steal from the underlying referred to object. Value category should be shallow. Thanks to many people for pointing this out. If “Deducing ” had been used, the problem would have been much more subtle in code review.

## Shallow vs Deep const

There is some implementation divergence in optionals about deep const for . That is, can the referred to be modified through a . Does return an or a , and does return an or a . I believe it is overall more defensible if the is shallow as it would be for a where the constness of the struct ref does not affect if the p pointer can be written through. This is consistent with the rebinding behavior being proposed.

Where deeper constness is desired, would prevent non const access to the underlying object.

## Conditional Explicit

As in the base template, is made conditional on the type used to construct the optional. . This is not present in boost::optional, leading to differences in construction between braced initialization and = that can be surprising.

## value_or

After extensive discussion, it seems there is no particularly wonderful solution for that does not involve a time machine. Implementations of optionals that support reference semantics diverge over the return type, and the current one is arguably wrong, and should use something based on , which of course did not exist when was standardized.

The weak consensus is to return a from as this is least likely to cause issues. There was at least one strong objection to this choice, but all other choices had more objections. The author intends to propose free functions , , , and over all types modeling optional-like, , in the next revision of . This would cover , , and pointer types.

Having return by value also allows the common case of using a literal as the alternative to be expressed concisely.

## in_place_t construction

The reference specialization allows a limited form of in_place construction where the argument can be converted to the appropriate reference without creation of a temporary. As the reference specialization is non-owning, there is no “place” for a temporary to be constructed that will not dangle. For cases where the lifetime of the constructed object would match the lifetime of the optional, the temporary can be constructed explicitly, instead.

## Converting assignment

A similarly limited converting assignment operator is provided for cases where an optional\<U\> has a value or refers to a value which can be converted to a T& without construction of a temporary. In particular, converting an optional\<T&\> to an optional\<T const&\> is supported.

## Compiler Explorer Playground

See <https://compiler-explorer.com/z/zKqE3sn87> for an updated playground with relevant Google Test functions and various optional implementations made available for cross reference including a flattened in-place version of the reference implementation.

# Principles for Reification of Design

Optional must never construct a temporary, or knowingly take the address of an temporary or part of an temporary.

It is always presumed safe to copy the pointer value from an optional, since by induction, it is not dangling.

Optional has no storage, so should never construct a T, it may convert a U to a T, so long as that conversion does not create a temporary.

Constructors that would convert from temporary are marked deleted. They should be sufficiently constrained that it was the correct choice and there is no more general, less constrained constructor that would not have created a dangling pointer.

Failure to compile either by ambiguity or no eligible constructors in the overload set is preferable to optional being responsible for use after free or dangling.

Assignment is always from an optional, which may have been an implicit construction. The assignment cannot throw, the construction/conversion may. The assignment may therefore need annotation converting the rhs if that constructor was explicit. This must not be necessary in the default case of creating an optional reference to an lvalue of the same type.

The model for the constraints and mandates for is taken from over reference types. The type takes the most care of types in the standard library in dealing with creation of temporaries.

As is designed to be converting, to create instances from arguments that can be used to create the underlying type, constructors should be explicit only where the operations used to create the pointer or the notional reference would be or are explicit.

## Construction from temporary

We disallow construction of from any type U in which:

- the constructor body will create a temporary and bind it to a reference.

- a const lvalue reference would be bound to rvalue.

An example of the first case would be construction from . These cases always dangle.

An example of the second case would be a construction from temporary .

Prohibiting the second case does prevent some safe uses of the optional as the function parameter.

Given:

This will make a invocation ill-formed, despite the arg being safe to use from within the function body.

This deviates from the design of the “view” parameters type, like or . However, we believe that this is the right choice due to the following:

- Only a subset of cases would be working. As an illustration the very similar invocation is ill-formed, due to always being dangling.

- Such design leads to the detection of reference to temporaries or local variables when is used as the return type.

  |     |                                                                                                                                                                         |
  |:----|------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|
  |     |   \> getValue() std::string localString; return localString; // Ill-formed. std::optional\<std::string\> localOptionalString; return localOptionalString; // ill-formed |

  One of the main motivational examples of is return from a lookup function, and eliminating dangling in such cases outweighs parameter cases.

  We are very grateful to Arthur O’Dwyer for his work on P2266R3 Simpler implicit move accepted in C++23, which makes it possible to implement this correctly.

- We provide behavior consistent with , that disallows binding to xvalues. We believe that is closer in spirit to than any view type. It certainly shares some of the features.

## Deleting dangling overloads

To achieve the dangling safety expressed before, the constructor is marked deleted if it would lead to binding of the reference to temporary or the xvalue. However, deleted constructors are still considered to be candidates during overload resolution, leading to ambiguity in the following examples:

During the reflector discussion, an option of an alternate design was presented, where the dangling overload would be constrained, and eliminated from the overload set.

We strongly oppose changing this behavior, as:

- We think that it is impossible to detect temporary binding to xvalue in such a design.

- The behavior we propose is consistent with the behavior for optional for object types

  [TABLE]

As language in general treats functions accepting by value and by const reference in the same manner during overload resolution, we believe achieving this consistency is a feature.

The design that was introduced by , and , for references, is followed, where the detection of dangling does not affect the results of overload resolution and instead makes a call that would dangle be ill-formed and diagnosed.

## Assignment of optional\<T&\>

In the case of , any assignment operation is equivalent to assigning a pointer, and there is no observable difference between: using converting assignment from or constructing temporary , and then assigning it to it.

This observation allows us to provide only copy-assignment for , instead of a set of converting assignments, that would need to replicate the signatures of constructors and their constraints. Assignment from any other value is handled by first implicitly constructing and then using copy-assignment. Move-assignment is the same as copy-assignment, since only pointer copy is involved.

## Copy and Assignment of optional\<U&\>&& to optional\<T\>

Care must be take to prevent the assignment of a movable optional to disallow the copy or assignment of the underlying referred to value to be stolen. The assignment or copy constructor should be used instead, which also needs to check slightly different constraints for and for testing . We thank Jan Kokemüller for uncovering this bug. The bug seems to be present in many optional implementations that support references.

# Proposal

Add an lvalue reference specialization for the std::optional template.

# Wording

The wording here cross references and adopts the wording in . The proposed changes are relative to the current working draft .

# Impact on the standard

A pure library extension, affecting no other parts of the library or language.

# Acknowledgments

Many thanks to all of the reviewers and authors of beman.optional, , in particular A. Jiang, Darius Neațu, David Sankel, Eddie Nolan, Jan Kokemüller, Jeff Garland, and River (Xueqing) Wu. Tomasz Kamiński provided extensive support for the library wording of optional\<T&\>.

# Document history

- **Changes since R8**

  - Fix move/assign optional\<U&\> allowing stealing of referenced U

- **Changes since R7**

  - Wording mandates/constraint fixes

  - Hash on T& pulled out

  - Notes on wording rendering

  - “Fix” make_optional\<T&\>

- **Changes since R6**

  - strike refref specialization

  - add converting assignment operator

  - add converting in place constructor

- **Changes since R5**

  - refref specialization

  - fix monadic constraints on base template

- **Changes since R4**

  - feature test macro

  - value_or updates from P3091

- **Changes since R3**

  - make_optional discussion - always value

  - value_or discussion - always value

- **Changes since R1**

  - Design points called out

- **Changes since R0**

  - Wording Updates

# Implementation

``` c++
// ----------------------
// BASE AND DETAILS ELIDED
// ----------------------

/****************/
/* optional<T&> */
/****************/

template <class T>
class optional<T&> {
  public:
    using value_type = T;
    using iterator =
        std::contiguous_iterator<T,
                                 optional>; // see [optionalref.iterators]
  public:
    // \ref{optionalref.ctor}, constructors

    constexpr optional() noexcept = default;
    constexpr optional(nullopt_t) noexcept : optional() {}
    constexpr optional(const optional& rhs) noexcept = default;

    template <class Arg>
        requires(std::is_constructible_v<T&, Arg> &&
                 !std::reference_constructs_from_temporary_v<T&, Arg>)
    constexpr explicit optional(in_place_t, Arg&& arg);

    template <class U>
        requires(std::is_constructible_v<T&, U> &&
                 !(std::is_same_v<std::remove_cvref_t<U>, in_place_t>) &&
                 !(std::is_same_v<std::remove_cvref_t<U>, optional>) &&
                 !std::reference_constructs_from_temporary_v<T&, U>)
    constexpr explicit(!std::is_convertible_v<U, T&>)
        optional(U&& u) noexcept(std::is_nothrow_constructible_v<T&, U>) {
        convert_ref_init_val(u);
    }

    template <class U>
        requires(std::is_constructible_v<T&, U> &&
                 !(std::is_same_v<std::remove_cvref_t<U>, in_place_t>) &&
                 !(std::is_same_v<std::remove_cvref_t<U>, optional>) &&
                 std::reference_constructs_from_temporary_v<T&, U>)
    constexpr optional(U&& u) = delete;

    // The full set of 4 overloads on optional<U> by value category, doubled to
    // 8 by deleting if reference_constructs_from_temporary_v is true. This
    // allows correct constraints by propagating the value category from the
    // optional to the value within the rhs.
    template <class U>
        requires(std::is_constructible_v<T&, U&> &&
                 !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
                 !std::is_same_v<T&, U> &&
                 !std::reference_constructs_from_temporary_v<T&, U&>)
    constexpr explicit(!std::is_convertible_v<U&, T&>) optional(
        optional<U>& rhs) noexcept(std::is_nothrow_constructible_v<T&, U&>);

    template <class U>
        requires(std::is_constructible_v<T&, const U&> &&
                 !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
                 !std::is_same_v<T&, U> &&
                 !std::reference_constructs_from_temporary_v<T&, const U&>)
    constexpr explicit(!std::is_convertible_v<const U&, T&>)
        optional(const optional<U>& rhs) noexcept(
            std::is_nothrow_constructible_v<T&, const U&>);

    template <class U>
        requires(std::is_constructible_v<T&, U> &&
                 !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
                 !std::is_same_v<T&, U> &&
                 !std::reference_constructs_from_temporary_v<T&, U>)
    constexpr explicit(!std::is_convertible_v<U, T&>)
        optional(optional<U>&& rhs) noexcept(
            noexcept(std::is_nothrow_constructible_v<T&, U>));

    template <class U>
        requires(std::is_constructible_v<T&, const U> &&
                 !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
                 !std::is_same_v<T&, U> &&
                 !std::reference_constructs_from_temporary_v<T&, const U>)
    constexpr explicit(!std::is_convertible_v<const U, T&>)
        optional(const optional<U>&& rhs) noexcept(
            noexcept(std::is_nothrow_constructible_v<T&, const U>));

    template <class U>
        requires(std::is_constructible_v<T&, U&> &&
                 !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
                 !std::is_same_v<T&, U> &&
                 std::reference_constructs_from_temporary_v<T&, U&>)
    constexpr optional(optional<U>& rhs) = delete;

    template <class U>
        requires(std::is_constructible_v<T&, const U&> &&
                 !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
                 !std::is_same_v<T&, U> &&
                 std::reference_constructs_from_temporary_v<T&, const U&>)
    constexpr optional(const optional<U>& rhs) = delete;

    template <class U>
        requires(std::is_constructible_v<T&, U> &&
                 !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
                 !std::is_same_v<T&, U> &&
                 std::reference_constructs_from_temporary_v<T&, U>)
    constexpr optional(optional<U>&& rhs) = delete;

    template <class U>
        requires(std::is_constructible_v<T&, const U> &&
                 !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
                 !std::is_same_v<T&, U> &&
                 std::reference_constructs_from_temporary_v<T&, const U>)
    constexpr optional(const optional<U>&& rhs) = delete;

    // \ref{optionalref.dtor}, destructor
    constexpr ~optional() = default;

    // \ref{optionalref.assign}, assignment
    constexpr optional& operator=(nullopt_t) noexcept;

    constexpr optional& operator=(const optional& rhs) noexcept = default;

    template <class U>
        requires(std::is_constructible_v<T&, U> &&
                 !std::reference_constructs_from_temporary_v<T&, U>)
    constexpr T&
    emplace(U&& u) noexcept(std::is_nothrow_constructible_v<T&, U>);

    // \ref{optionalref.swap}, swap
    constexpr void swap(optional& rhs) noexcept;

    // \ref{optional.iterators}, iterator support
    constexpr iterator begin() const noexcept;
    constexpr iterator end() const noexcept;

    // \ref{optionalref.observe}, observers
    constexpr T*       operator->() const noexcept;
    constexpr T&       operator*() const noexcept;
    constexpr explicit operator bool() const noexcept;
    constexpr bool     has_value() const noexcept;
    constexpr T&       value() const;
    template <class U = std::remove_cv_t<T>>
    constexpr std::remove_cv_t<T> value_or(U&& u) const;

    // \ref{optionalref.monadic}, monadic operations
    template <class F>
    constexpr auto and_then(F&& f) const;
    template <class F>
    constexpr optional<std::invoke_result_t<F, T&>> transform(F&& f) const;
    template <class F>
    constexpr optional or_else(F&& f) const;

    // \ref{optional.mod}, modifiers
    constexpr void reset() noexcept;

  private:
    T* value_ = nullptr; // exposition only

    // \ref{optionalref.expos}, exposition only helper functions
    template <class U>
    constexpr void convert_ref_init_val(U&& u) {
        // Creates a variable, \tcode{r},
        // as if by \tcode{T\& r(std::forward<U>(u));}
        // and then initializes \exposid{val} with \tcode{addressof(r)}
        T& r(std::forward<U>(u));
        value_ = std::addressof(r);
    }
};

//  \rSec3[optionalref.ctor]{Constructors}
template <class T>
template <class Arg>
    requires(std::is_constructible_v<T&, Arg> &&
             !std::reference_constructs_from_temporary_v<T&, Arg>)
constexpr optional<T&>::optional(in_place_t, Arg&& arg) {
    convert_ref_init_val(std::forward<Arg>(arg));
}

// Clang is unhappy with the out-of-line definition
//
// template <class T>
// template <class U>
//     requires(std::is_constructible_v<T&, U> &&
//     !(is_same_v<remove_cvref_t<U>, in_place_t>) &&
//              !(is_same_v<remove_cvref_t<U>, optional<T&>>) &&
//              !std::reference_constructs_from_temporary_v<T&, U>)
// constexpr optional<T&>::optional(U&& u)
// noexcept(is_nothrow_constructible_v<T&, U>)
//     : value_(std::addressof(static_cast<T&>(std::forward<U>(u)))) {}

template <class T>
template <class U>
    requires(std::is_constructible_v<T&, U&> &&
             !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
             !std::is_same_v<T&, U> &&
             !std::reference_constructs_from_temporary_v<T&, U&>)
constexpr optional<T&>::optional(optional<U>& rhs) noexcept(
    std::is_nothrow_constructible_v<T&, U&>) {
    if (rhs.has_value()) {
        convert_ref_init_val(*rhs);
    }
}

template <class T>
template <class U>
    requires(std::is_constructible_v<T&, const U&> &&
             !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
             !std::is_same_v<T&, U> &&
             !std::reference_constructs_from_temporary_v<T&, const U&>)
constexpr optional<T&>::optional(const optional<U>& rhs) noexcept(
    std::is_nothrow_constructible_v<T&, const U&>) {
    if (rhs.has_value()) {
        convert_ref_init_val(*rhs);
    }
}

template <class T>
template <class U>
    requires(std::is_constructible_v<T&, U> &&
             !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
             !std::is_same_v<T&, U> &&
             !std::reference_constructs_from_temporary_v<T&, U>)
constexpr optional<T&>::optional(optional<U>&& rhs) noexcept(
    noexcept(std::is_nothrow_constructible_v<T&, U>)) {
    if (rhs.has_value()) {
        convert_ref_init_val(*std::move(rhs));
    }
}

template <class T>
template <class U>
    requires(std::is_constructible_v<T&, const U> &&
             !std::is_same_v<std::remove_cv_t<T>, optional<U>> &&
             !std::is_same_v<T&, U> &&
             !std::reference_constructs_from_temporary_v<T&, const U>)
constexpr optional<T&>::optional(const optional<U>&& rhs) noexcept(
    noexcept(std::is_nothrow_constructible_v<T&, const U>)) {
    if (rhs.has_value()) {
        convert_ref_init_val(*std::move(rhs));
    }
}

// \rSec3[optionalref.assign]{Assignment}
template <class T>
constexpr optional<T&>& optional<T&>::operator=(nullopt_t) noexcept {
    value_ = nullptr;
    return *this;
}

template <class T>
template <class U>
    requires(std::is_constructible_v<T&, U> &&
             !std::reference_constructs_from_temporary_v<T&, U>)
constexpr T&
optional<T&>::emplace(U&& u) noexcept(std::is_nothrow_constructible_v<T&, U>) {
    convert_ref_init_val(std::forward<U>(u));
    return *value_;
}

//   \rSec3[optionalref.swap]{Swap}

template <class T>
constexpr void optional<T&>::swap(optional<T&>& rhs) noexcept {
    std::swap(value_, rhs.value_);
}

// \rSec3[optionalref.iterators]{Iterator Support}

template <class T>
constexpr optional<T&>::iterator optional<T&>::begin() const noexcept {
    return iterator(has_value() ? value_ : nullptr);
};

template <class T>
constexpr optional<T&>::iterator optional<T&>::end() const noexcept {
    return begin() + has_value();
}

// \rSec3[optionalref.observe]{Observers}
template <class T>
constexpr T* optional<T&>::operator->() const noexcept {
    return value_;
}

template <class T>
constexpr T& optional<T&>::operator*() const noexcept {
    return *value_;
}

template <class T>
constexpr optional<T&>::operator bool() const noexcept {
    return value_ != nullptr;
}
template <class T>
constexpr bool optional<T&>::has_value() const noexcept {
    return value_ != nullptr;
}

template <class T>
constexpr T& optional<T&>::value() const {
    return has_value() ? *value_ : throw bad_optional_access();
}

template <class T>
template <class U>
constexpr std::remove_cv_t<T> optional<T&>::value_or(U&& u) const {
    static_assert(std::is_constructible_v<std::remove_cv_t<T>, T&>,
                  "T must be constructible from a T&");
    static_assert(std::is_convertible_v<U, std::remove_cv_t<T>>,
                  "Must be able to convert u to T");
    return has_value() ? *value_
                       : static_cast<std::remove_cv_t<T>>(std::forward<U>(u));
}

//   \rSec3[optionalref.monadic]{Monadic operations}
template <class T>
template <class F>
constexpr auto optional<T&>::and_then(F&& f) const {
    using U = std::invoke_result_t<F, T&>;
    static_assert(detail::is_optional<U>, "F must return an optional");
    if (has_value()) {
        return std::invoke(std::forward<F>(f), *value_);
    } else {
        return std::remove_cvref_t<U>();
    }
}

template <class T>
template <class F>
constexpr optional<std::invoke_result_t<F, T&>>
optional<T&>::transform(F&& f) const {
    using U = std::invoke_result_t<F, T&>;
    static_assert(!std::is_same_v<std::remove_cvref_t<U>, in_place_t>,
                  "Result must not be in_place_t");
    static_assert(!std::is_same_v<std::remove_cvref_t<U>, nullopt_t>,
                  "Result must not be nullopt_t");
    static_assert((std::is_object_v<U> && !std::is_array_v<U>) ||
                      std::is_lvalue_reference_v<U>,
                  "Result must be an non-array object or an lvalue reference");
    if (has_value()) {
        return optional<U>{std::invoke(std::forward<F>(f), *value_)};
    } else {
        return optional<U>{};
    }
}

template <class T>
template <class F>
constexpr optional<T&> optional<T&>::or_else(F&& f) const {
    using U = std::invoke_result_t<F>;
    static_assert(std::is_same_v<std::remove_cvref_t<U>, optional>,
                  "Result must be an optional");
    if (has_value()) {
        return *value_;
    } else {
        return std::forward<F>(f)();
    }
}

// \rSec3[optional.mod]{modifiers}
template <class T>
constexpr void optional<T&>::reset() noexcept {
    value_ = nullptr;
}
} // namespace beman::optional

namespace std {
template <typename T>
    requires requires(T a) {
        {
            std::hash<remove_const_t<T>>{}(a)
        } -> std::convertible_to<std::size_t>;
    }
struct hash<beman::optional::optional<T>> {
    static_assert(!is_reference_v<T>,
                  "hash is not enabled for reference types");
    size_t operator()(const beman::optional::optional<T>& o) const
        noexcept(noexcept(hash<remove_const_t<T>>{}(*o))) {
        if (o) {
            return std::hash<std::remove_const_t<T>>{}(*o);
        } else {
            return 0;
        }
    }
};
```
