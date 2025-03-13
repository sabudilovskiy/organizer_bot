#pragma once

#include <boost/json.hpp>
#include <tgbm/utils/traits.hpp>
#include <variant>

#include "utils.hpp"

namespace bot {
template <typename Meta>
struct meta_names {
  static constexpr auto value = []() {
    auto ret = variant_names_v<Meta>;
    for (auto& name : ret) {
      name.remove_suffix(sizeof("_meta_t") - 1);
    }
    return ret;
  }();
};

template <typename Meta>
constexpr auto meta_names_v = meta_names<Meta>::value;

/*
 * `aggregate_with_meta` is a concept that defines a structured type
 * (an aggregate) with an embedded `meta` field, which acts as a variant storage
 * for additional data. This allows storing different types of metadata while
 * keeping the main structure uniform.
 *
 * The key idea is that `meta` represents an extension point for the object,
 * encapsulating variant data while maintaining type safety.
 *
 * Requirements:
 *  * The type must be an aggregate (`tgbm::aggregate<T>`).
 *  * It must contain a field named `meta`, whose type is defined as `T::meta_t`.
 *  * `T::meta_t` must be a `std::variant` containing only aggregate types.
 *  * Each type inside `meta_t` must follow the naming convention:
 *    `enum_value_meta_t` (e.g., `string_meta_t`, `integer_meta_t`).
 *  * The type must define `T::meta_type`, which is typically an alias for an external
 * enum.
 *  * The external enum (`my_object_type`, for example) must:
 *      - Map one-to-one to the index of each type in `meta_t`.
 *      - Be returned from a `type()` method in `T`, which provides the active type.
 *
 * This concept describes a structured type where fixed fields
 * are supplemented by a `meta` field, which behaves as a structured variant
 * with well-defined alternatives.
 *
 * Example:
 *
 * struct string_meta_t { std::string value; };
 * struct integer_meta_t { int value; };
 *
 * using my_object_meta_t = std::variant<string_meta_t, integer_meta_t>;
 * enum class my_object_type { string, integer };
 *
 * struct my_object {
 *     int id;
 *     my_object_meta_t meta;
 *     using meta_t = my_object_meta_t;
 *     using meta_type = my_object_type;
 *
 *     meta_type type() const { return static_cast<meta_type>(meta.index()); }
 * };
 *
 * static_assert(aggregate_with_meta<my_object>);
 */

template <typename T>
concept aggregate_with_meta =
    tgbm::aggregate<T> && requires { requires pfr_extension::has_element_v<"meta", T>; };

template <typename Info>
constexpr bool is_meta_field_v = Info::name == "meta";

template <typename T>
using meta_type_t = typename T::meta_type;

template <typename T>
using meta_value_t = decltype(T::meta);

template <std::size_t I, typename T>
using meta_alternative_t = std::variant_alternative_t<I, meta_value_t<T>>;

template <typename T>
constexpr std::size_t meta_size_v = std::variant_size_v<meta_value_t<T>>;

struct unknown {};

/*
 * `visit_object_with_meta` is an extended version of `visit_object` designed
 * for aggregates that contain a `meta` field, which is a `std::variant`.
 *
 * The function behaves differently depending on whether the object is mutable or
 * constant:
 *
 * 1. If the object is mutable (`T&`):
 *    * The active alternative from `meta` is extracted and passed to the functor
 *      as if `meta` were a regular field.
 *    * The functor does not receive `meta` as a `std::variant`, but instead
 *      as the actual stored value.
 *
 * 2. If the object is constant (`const T&`):
 *    * `meta` is passed as a `std::variant`.
 *    * An additional field, `meta_type`, is passed to the functor,
 *      which is an `enum` representing the active alternative of `meta`.
 *    * The `enum` must be explicitly defined within the type and obtained
 *      via `t.type()`.
 *
 * In both cases, all other fields are processed normally using `visit_object`.
 *
 * Example usage:
 *
 * struct MyObject {
 *     int id;
 *     std::variant<std::string, int> meta;
 *
 *     enum class MetaType { String, Int };
 *     MetaType type() const { return static_cast<MetaType>(meta.index()); }
 * };
 *
 * MyObject obj{42, "example"};
 * visit_object_with_meta(obj, [&]<typename Info>(auto&& field) {
 *     // `meta` is passed as `std::string`, not `std::variant<std::string, int>`.
 * });
 *
 * const MyObject const_obj{42, 100};
 * visit_object_with_meta(const_obj, [&]<typename Info>(auto&& field) {
 *     // `meta` is passed as `std::variant<std::string, int>`.
 *     // `meta_type` (MyObject::MetaType) is also passed separately.
 * });
 */
template <typename T>
void visit_object_with_meta(T& t, auto&& functor) {
  pfr_extension::visit_object(t, [&]<typename Info>(auto& field) {
    if constexpr (is_meta_field_v<Info>) {
      std::visit([&](auto& meta_field) { functor.template operator()<Info>(meta_field); },
                 field);
    } else {
      functor.template operator()<Info>(field);
    }
  });
}

/*
 * `visit_object_with_meta` overload for `const T&`.
 *
 * This version of the function behaves similarly to the mutable overload but preserves
 * `meta` as a `std::variant` instead of extracting its active alternative.
 * Additionally, it provides `meta_type` to the functor.
 *
 * For a detailed explanation, see the main `visit_object_with_meta` overload.
 */
template <typename T>
void visit_object_with_meta(const T& t, auto&& functor) {
  pfr_extension::visit_object(t, [&]<typename Info>(const auto& field) {
    if constexpr (is_meta_field_v<Info>) {
      std::visit(
          [&](const auto& meta_field) { functor.template operator()<Info>(meta_field); },
          field);
    } else {
      functor.template operator()<Info>(field);
    }
  });
  using Info = pfr_extension::field_info<pfr_extension::tuple_size_v<T>, "meta_type">;
  const meta_type_t<T> meta_type = t.type();
  functor.template operator()<Info>(meta_type);
}

template <typename T>
void visit_meta(std::optional<meta_type_t<T>> e, auto&& functor) {
  static_assert(magic_enum::enum_names<meta_type_t<T>>() ==
                meta_names_v<meta_value_t<T>>);
  if (!e) {
    functor.template operator()<missing>();
  }
  auto proxy = [&]<std::size_t I>() {
    functor.template operator()<meta_alternative_t<I, T>>();
  };
  tgbm::visit_index<meta_size_v<T> - 1>() > (proxy, (std::size_t)e);
}

template <typename T>
void visit_meta(meta_type_t<T> e, auto&& functor) {
  static_assert(magic_enum::enum_names<meta_type_t<T>>() ==
                meta_names_v<meta_value_t<T>>);
  auto proxy = [&]<std::size_t I>() {
    functor.template operator()<meta_alternative_t<I, T>>();
  };
  std::size_t index = (std::size_t)e;
  tgbm::visit_index<meta_size_v<T> - 1>(proxy, index);
}

template <typename T>
void emplace_meta(meta_type_t<T> e, T& t) {
  visit_meta<T>(e, [&]<typename F>() { t.meta = F{}; });
}

/*
 * `visit_struct_with_meta` provides a `visit_struct`-based alternative to
 * `visit_object_with_meta`.
 *
 * This function allows visiting the structure of `T` at the type level without an
 * instance, while also including `meta_type` as an additional field.
 *
 * The behavior is analogous to the `const T&` overload of `visit_object_with_meta`,
 * where `meta` is treated as a `std::variant`, and `meta_type` is explicitly provided.
 *
 * For a detailed explanation, see `visit_object_with_meta`.
 */
template <typename T>
void visit_struct_with_meta(auto&& functor) {
  pfr_extension::visit_struct<T>(functor);
  using Info = pfr_extension::field_info<pfr_extension::tuple_size_v<T>, "meta_type">;
  functor.template operator()<Info, meta_type_t<T>>();
}

}  // namespace bot
