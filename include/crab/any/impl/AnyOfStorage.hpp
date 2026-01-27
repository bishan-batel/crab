#pragma once

#include <array>
#include <cstddef>
#include <memory>

#include "crab/any/impl/Buffer.hpp"
#include "crab/mem/size_of.hpp"
#include "crab/mem/address_of.hpp"
#include "crab/mem/forward.hpp"
#include "crab/mem/move.hpp"
#include "crab/ty/construct.hpp"

namespace crab::any::impl {

  template<typename... Ts>
  constexpr usize ByteSize{std::max({
    []() {
      if constexpr (ty::is_reference<Ts>) {
        return mem::size_of<ty::remove_reference<Ts>*>();
      } else {
        return mem::size_of<Ts>();
      }
    }()...,
  })};

  template<typename... Ts>
  constexpr usize AlignOf{std::max({
    []() {
      if constexpr (ty::is_reference<Ts>) {
        return alignof(ty::remove_reference<Ts>*);
      } else {
        return alignof(Ts);
      }
    }()...,
  })};

  template<typename T>
  struct AnyOfStorage final {
    template<usize Size, usize Align, typename... Args>
    requires std::constructible_from<T, Args...>
    static auto construct(Buffer<Size, Align>& buffer, Args&&... args) -> void {
      std::construct_at<T, Args...>(as_ptr(buffer), std::forward<Args>(args)...);
    }

    template<usize Size, usize Align>
    static auto destroy(Buffer<Size, Align>& buffer) -> void {
      std::destroy_at(as_ptr(buffer));
    }

    template<usize Size, usize Align>
    static auto move(Buffer<Size, Align>& from, Buffer<Size, Align>& to) -> void {
      construct<Size, Align, T&&>(to, mem::move(as_ref(from)));
    }

    template<usize Size, usize Align>
    static auto copy(const Buffer<Size, Align>& from, Buffer<Size, Align>& to) -> void {
      static_assert(ty::copyable<T>, "Cannot use copy on a non-copyable type");

      construct<Size, Align, const T&>(to, as_ref(from));
    }

    template<usize Size, usize Align>
    static auto move_assign(Buffer<Size, Align>& from, Buffer<Size, Align>& to) -> void {
      as_ref(to) = mem::move(as_ref(from));
    }

    template<usize Size, usize Align>
    static auto copy_assign(const Buffer<Size, Align>& from, Buffer<Size, Align>& to) -> void {
      static_assert(ty::movable<T>, "Cannot use copy on a non-copyable type");
      as_ref(to) = implicit_cast<const T&>(as_ref(from));
    }

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ref(Buffer<Size, Align>& buffer) -> T& {
      return *as_ptr(buffer);
    }

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ref(const Buffer<Size, Align>& buffer) -> const T& {
      return *as_ptr(buffer);
    }

  private:

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ptr(Buffer<Size, Align>& buffer) -> T* {
      return buffer.template as_ptr<T>();
    }

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ptr(const Buffer<Size, Align>& buffer) -> const T* {
      return buffer.template as_ptr<T>();
    }
  };

  template<typename T>
  struct AnyOfStorage<T&> final {

    template<usize Size, usize Align>
    static auto construct(Buffer<Size, Align>& buffer, T& ref) -> void {
      T** ptr{buffer.template as_ptr<T*>()};
      *ptr = mem::address_of(ref);
    }

    template<usize Size, usize Align>
    static auto destroy(Buffer<Size, Align>&) -> void {}

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ref(const Buffer<Size, Align>& buffer) -> T& {
      T* const* ptr{buffer.template as_ptr<T*>()};
      return **ptr;
    }

    template<usize Size, usize Align>
    static auto move(Buffer<Size, Align>& from, Buffer<Size, Align>& to) -> void {
      copy(from, to);
    }

    template<usize Size, usize Align>
    static auto copy(const Buffer<Size, Align>& from, Buffer<Size, Align>& to) -> void {
      contruct(to, as_ref(from.buffer));
    }

    template<usize Size, usize Align>
    static auto move_assign(Buffer<Size, Align>& from, Buffer<Size, Align>& to) -> void {
      copy(from, to);
    }

    template<usize Size, usize Align>
    static auto copy_assign(const Buffer<Size, Align>& from, Buffer<Size, Align>& to) -> void {
      contruct(to, as_ref(from.buffer));
    }
  };

  template<typename T>
  struct AnyOfConstructor {
  protected:

    auto impl_construct(T&& value, auto& buffer) -> std::type_identity<T> {
      AnyOfStorage<T>::construct(buffer, mem::forward<T>(value));

      return {};
    }

    auto impl_insert(T&& value, auto& buffer) -> std::type_identity<T> {
      AnyOfStorage<T>::construct(buffer, mem::forward<T>(value));

      return {};
    }

    template<typename... Args>
    requires std::constructible_from<T, Args...>
    auto impl_emplace(std::type_identity<T>, auto& buffer, Args&&... args) -> std::type_identity<T> {
      AnyOfStorage<T>::construct(buffer, mem::forward<Args>(args)...);

      return {};
    }
  };

}
