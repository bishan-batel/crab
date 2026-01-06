#include <algorithm>
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include "./preamble.hpp"
#include "crab/core.hpp"
#include "crab/option.hpp"
#include "crab/range.hpp"
#include "crab/type_traits.hpp"

namespace crab::choice {

  namespace impl {

    template<auto tag_value, typename T>
    struct Case {
      inline static const Case instance{};

      static constexpr auto tag{tag_value};

      using tag_type = decltype(tag_value);
      using type = T;

      CRAB_PURE_CONSTEXPR static auto is_tag(const tag_type& tag) -> bool {
        return tag == tag_value;
      }

      CRAB_PURE_CONSTEXPR static auto as_ptr(void* raw) -> T* {
        return reinterpret_cast<T*>(raw);
      }

      CRAB_PURE_CONSTEXPR static auto as_ptr(const void* raw) -> const T* {
        return reinterpret_cast<const T*>(raw);
      }

      CRAB_PURE_CONSTEXPR static auto as_ref(void* raw) -> T& {
        debug_assert(raw != nullptr, "Cannot create ref from nullptr");
        return *as_ptr(raw);
      }

      CRAB_PURE_CONSTEXPR static auto as_ref(const void* raw) -> const T& {
        debug_assert(raw != nullptr, "Cannot create ref from nullptr");
        return *as_ptr(raw);
      }

      CRAB_PURE_CONSTEXPR static auto destruct(void* value) {
        debug_assert(value != nullptr, "Cannot destruct nullptr");
        std::destroy_at(as_ptr(value));
      }

      template<std::constructible_from<T>... Args>
      CRAB_PURE_CONSTEXPR auto construct(void* value, Args&&... args) {
        debug_assert(value != nullptr, "Cannot destruct nullptr");
        std::construct_at(as_ptr(value), std::forward<Args>(args)...);
      }
    };

    template<typename T>
    struct case_type : ty::false_type {};

    template<auto tag_value, typename T>
    struct case_type<Case<tag_value, T>> : ty::true_type {
      using type = Case<tag_value, T>::type;
      using tag_type = Case<tag_value, T>::tag_type;
      static constexpr auto tag{tag_value};
    };

    template<typename T>
    using case_tag_type_t = case_type<T>::tag_type;

    template<typename T>
    using case_type_t = case_type<T>::type;

    template<typename T>
    constexpr auto case_tag{case_type<T>::tag};
  }

  template<typename... Cases>
  class Choice {
  public:

    static constexpr usize NUM_CASES{sizeof...(Cases)};

    static_assert(NUM_CASES, "Choice cannot have no variants");
    static_assert((ty::all_same<impl::case_tag_type_t<Cases>...>), "All cases must have the same tag type");

    using Tag = impl::case_tag_type_t<ty::nth_type<0, Cases...>>;

    static_assert(std::equality_comparable<Tag>, "Tag type for Choice<> must be equality comparable with itself");

    /**
     * Static storage for each tag value for each type case
     */
    static constexpr SizedArray<Tag, NUM_CASES> TAGS{impl::case_tag<Cases>...};

    static CRAB_CONSTEVAL auto assert_unique_tags() -> bool {
      for (usize i = 0; i < NUM_CASES; i++) {
        for (usize j = i + 1; j < NUM_CASES; j++) {
          if (TAGS.at(i) == TAGS.at(j)) {
            return false;
          }
        }
      }
      return true;
    }

    static_assert(assert_unique_tags(), "Choice<T...> cannot have cases with overlapping tags");

    static CRAB_CONSTEVAL auto index_of_tag(Tag tag) -> usize {
      const auto location{std::ranges::find(TAGS, tag)};
      return std::distance(TAGS.begin(), location);
    }

    template<Tag tag>
    using CaseType = ty::nth_type<index_of_tag(tag), impl::case_type_t<Cases>...>;

    /**
     * List of all data sizes
     */
    static constexpr std::initializer_list<usize> DATA_SIZES{
      mem::size_of<impl::case_type_t<Cases>>()...,
    };

    /**
     * Minimum byte size needed to contain all data
     */
    static constexpr usize DATA_SIZE{std::ranges::max(DATA_SIZES)};

    CRAB_PURE_CONSTEXPR auto destruct() {
      // clang-format off
      if (invalid) CRAB_UNLIKELY return;
      // clang-format on

      invalid = not(
        [this]<Tag case_tag, typename T>(const impl::Case<case_tag, T>& variant) {
          if (not variant.is_tag(tag())) {
            return false;
          }

          variant.destruct(bytes.data());

          return true;
        }(Cases::instance)
        or ...
      );

      debug_assert(invalid == false, "Did not find a correct destruction method for choice ");
    }

  public:

    // template<Tag tag>
    // explicit Choice(CaseType<tag>&& value) requires(ty::either<impl::Case<tag, T>, Cases...>)
    // {}

    template<Tag requested>
    CRAB_PURE_INLINE_CONSTEXPR auto as() const& {
      using T = CaseType<requested>;

      constexpr impl::Case<requested, T> case_impl{};

      using Option = option::Option<const T&>;

      if (tag() != requested) {
        return Option{};
      }

      return Option{case_impl.as_ref(bytes.data())};
    }

    template<Tag requested>
    CRAB_PURE_INLINE_CONSTEXPR auto as() & {
      using T = CaseType<requested>;
      using Case = impl::Case<requested, T>;
      using Option = option::Option<const T&>;

      if (tag() != requested) {
        return Option{};
      }

      return Option{Case::as_ref(bytes.data())};
    }

    CRAB_PURE_INLINE_CONSTEXPR auto tag() const -> const Tag& {
      return tag_value;
    }

  private:

    SizedArray<std::byte, DATA_SIZE> bytes;
    Tag tag_value;
    bool invalid{false};
  };

  enum class EitherOr {
    First,
    Second
  };

  template<typename T, typename U>
  using Either = Choice<impl::Case<EitherOr::First, T>, impl::Case<EitherOr::Second, U>>;
}

#if CRAB_USE_PRELUDE

using crab::choice::Choice;

#endif
