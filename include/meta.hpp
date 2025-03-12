#pragma once

#include <boost/json.hpp>

#include "json/reader.hpp"
#include "json/writer.hpp"
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

template <typename Meta, typename MetaType>
struct json_reader_meta {
  static_assert(magic_enum::enum_names<MetaType>() == meta_names_v<Meta>);

  static Meta read(const boost::json::value& value) {
    Meta result;
    auto& obj = value.as_object();
    MetaType type = [&]() {
      auto it = obj.find("type");
      if (it == obj.end()) {
        throw std::runtime_error("not found");
      }
      auto e = magic_enum::enum_cast<MetaType>(it->value().as_string());
      if (!e) {
        throw std::runtime_error("Unknown type");
      }
      return *e;
    }();

    tgbm::visit_index<magic_enum::enum_count<MetaType>() - 1>(
        [&]<std::size_t I>() {
          using RType = std::variant_alternative_t<I, Meta>;
          auto it = obj.find("value");
          if (it == obj.end()) {
            throw std::runtime_error("not found");
          }
          result = json_reader<RType>::read(it->value());
        },
        (std::size_t)type);
    return result;
  }
};

template <typename Meta, typename MetaType>
struct json_writer_meta {
  static_assert(magic_enum::enum_names<MetaType>() == meta_names_v<Meta>);

  static void write(boost::json::value& v, const Meta& meta) {
    boost::json::object res;
    res["type"] = magic_enum::enum_name(MetaType(meta.index()));
    std::visit(
        [&]<typename V>(const V& value) { json_writer<V>::write(res["value"], value); },
        meta);
    v = std::move(res);
  }
};

}  // namespace bot
