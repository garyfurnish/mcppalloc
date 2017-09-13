#pragma once
#include <cassert>
#include <mcpputil/mcpputil/alignment.hpp>
#include <mcpputil/mcpputil/bitwise.hpp>
namespace mcppalloc
{
  namespace details
  {
    MCPPALLOC_ALWAYS_INLINE bool is_aligned_properly(const object_state_base_t *os) noexcept
    {
      return os == mcpputil::align(os, object_state_base_t::cs_alignment);
    }
    MCPPALLOC_ALWAYS_INLINE object_state_base_t *object_state_base_t::from_object_start(void *v, size_type alignment) noexcept
    {
      auto nv = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(v) & (~(alignment - 1)));
      return reinterpret_cast<object_state_base_t *>(reinterpret_cast<uint8_t *>(nv) -
                                                     mcpputil::align(sizeof(object_state_base_t), alignment));
    }
    template <typename Object_State_Type>
    MCPPALLOC_ALWAYS_INLINE Object_State_Type *object_state_base_t::from_object_start(void *v, size_type alignment) noexcept
    {
      auto os = from_object_start(v, alignment);
      if (mcpputil_unlikely(reinterpret_cast<uintptr_t>(os) % 16 != 0)) {
        ::std::abort();
      }
      return static_cast<Object_State_Type *>(os);
    }

    MCPPALLOC_ALWAYS_INLINE void
    object_state_base_t::set_all(object_state_base_t *next, bool in_use, bool next_valid, bool quasi_freed) noexcept
    {
      m_pre_magic = cs_pre_magic;
      m_next = reinterpret_cast<size_type>(next) | static_cast<size_type>(in_use) | (static_cast<size_type>(next_valid) << 1) |
               (static_cast<size_type>(quasi_freed) << 2);
      m_post_magic = cs_post_magic;
    }
    MCPPALLOC_ALWAYS_INLINE void object_state_base_t::verify_magic() noexcept
    {
#ifdef _DEBUG
      if (mcpputil_unlikely(m_pre_magic != cs_pre_magic)) {
        ::std::abort();
      } else if (mcpputil_unlikely(m_post_magic != cs_post_magic)) {
        ::std::abort();
      }
#endif
    }

    MCPPALLOC_ALWAYS_INLINE void object_state_base_t::set_in_use(bool v) noexcept
    {
      auto ptr = static_cast<size_type>(m_next);
      auto iv = static_cast<size_type>(v);
      m_next = (ptr & mcpputil::bitwise_negate(1)) | (iv & 1);
    }
    MCPPALLOC_ALWAYS_INLINE bool object_state_base_t::not_available() const noexcept
    {
      return 0 < (m_next & 5);
    }
    MCPPALLOC_ALWAYS_INLINE bool object_state_base_t::in_use() const noexcept
    {
      return (m_next & 1) != 0;
    }
    MCPPALLOC_ALWAYS_INLINE bool object_state_base_t::quasi_freed() const noexcept
    {
      return (m_next & 4) > 0;
    }
    MCPPALLOC_ALWAYS_INLINE void object_state_base_t::set_quasi_freed() noexcept
    {
      set_all(next(), false, next_valid(), true);
    }
    MCPPALLOC_ALWAYS_INLINE void object_state_base_t::set_quasi_freed(bool val) noexcept
    {
      assert(!(val && in_use()));
      set_all(next(), in_use(), next_valid(), val);
    }
    MCPPALLOC_ALWAYS_INLINE void object_state_base_t::set_next_valid(bool v) noexcept
    {
      auto ptr = static_cast<size_type>(m_next);
      size_type iv = static_cast<size_type>(v) << 1;
      m_next = (ptr & mcpputil::bitwise_negate(2)) | (iv & 2);
    }
    MCPPALLOC_ALWAYS_INLINE bool object_state_base_t::next_valid() const noexcept
    {
      return (static_cast<size_type>(m_next) & 2) > 0;
    }
    MCPPALLOC_ALWAYS_INLINE object_state_base_t *object_state_base_t::next() const noexcept
    {
      return reinterpret_cast<object_state_base_t *>(m_next & mcpputil::bitwise_negate(7));
    }
    template <typename Object_State_Type>
    MCPPALLOC_ALWAYS_INLINE Object_State_Type *object_state_base_t::next()
    {
      return static_cast<Object_State_Type *>(next());
    }

    MCPPALLOC_ALWAYS_INLINE void object_state_base_t::set_next(object_state_base_t *state) noexcept
    {
      auto ptr = reinterpret_cast<size_type>(state);
      //      size_type iv = static_cast<size_type>(not_available());
      m_next = (ptr & static_cast<size_type>(-4)) | (m_next & 1); //(iv & 1);
    }
    MCPPALLOC_ALWAYS_INLINE uint8_t *object_state_base_t::object_start(size_type alignment) const noexcept
    {
      return const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(this) +                 // NOLINT
                                   mcpputil::align(sizeof(object_state_base_t), alignment)); // NOLINT
    }
    MCPPALLOC_ALWAYS_INLINE auto object_state_base_t::object_size(size_type alignment) const noexcept -> size_type
    {
      // It is invariant that object_start() > next for all valid objects.
      return static_cast<size_type>(reinterpret_cast<uint8_t *>(next()) - object_start(alignment));
    }
    MCPPALLOC_ALWAYS_INLINE uint8_t *object_state_base_t::object_end(size_type alignment) const noexcept
    {
      return const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(this) +
                                   mcpputil::align(sizeof(object_state_base_t), cs_alignment) + object_size(alignment));
    }
    MCPPALLOC_ALWAYS_INLINE user_data_base_t *object_state_base_t::user_data() const noexcept
    {
      auto tmp = m_user_data & (~static_cast<size_type>(7));
      if ((tmp & 15) != 0) {
        return nullptr;
      }
      return reinterpret_cast<user_data_base_t *>(tmp);
    }
    MCPPALLOC_ALWAYS_INLINE void object_state_base_t::set_user_data(void *user_data) noexcept
    {
      assert(user_data == mcpputil::align_pow2(user_data, 3));
      m_user_data = reinterpret_cast<size_type>(user_data) | user_flags();
    }
    MCPPALLOC_ALWAYS_INLINE auto object_state_base_t::user_flags() const noexcept -> size_type
    {
      return m_user_data & 7;
    }
    MCPPALLOC_ALWAYS_INLINE void object_state_base_t::set_user_flags(size_type flags) noexcept
    {
      assert(flags < 8);
      m_user_data = reinterpret_cast<size_type>(user_data()) | flags;
    }
  } // namespace details
} // namespace mcppalloc
