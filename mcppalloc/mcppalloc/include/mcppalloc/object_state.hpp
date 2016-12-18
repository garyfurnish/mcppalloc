#pragma once
#include "allocator_policy.hpp"
#include "declarations.hpp"
#include "default_allocator_policy.hpp"
#include <gsl/gsl>
#include <mcpputil/mcpputil/alignment.hpp>
#include <memory>
#include <type_traits>
namespace mcppalloc
{
  namespace details
  {
    template <typename Allocator_Policy>
    class object_state_t;
    class object_state_base_t;
    class user_data_base_t;
    /**
     * \brief All object_state_t must be at least c_align_pow2 aligned, so test that.
     **/
    template <typename Allocator_Policy>
    bool is_aligned_properly(const object_state_base_t *os) noexcept;

    class alignas(16) object_state_base_t
    {
    public:
      using uintptr_type = uintptr_t;
      using size_type = size_t;
      using ptrdiff_type = ptrdiff_t;

      static const constexpr size_type cs_alignment = 16;
      /**
       * \brief Return the total size needed for an allocation of object state of sz with header_sz.
       **/
      static size_type needed_size(size_type header_sz, size_type sz, size_type alignment = cs_alignment)
      {
        ::gsl::narrow<ptrdiff_t>(header_sz);
	if(::gsl::narrow_cast<ptrdiff_t>(sz)<0)
	  return ::std::numeric_limits<size_type>::max();
        ::gsl::narrow<ptrdiff_t>(sz);
        return mcpputil::align(header_sz, alignment) + mcpputil::align(sz, alignment);
      }
      /**
       * \brief Return the address of the object_state from the allocated object memory.
       **/
      static object_state_base_t *from_object_start(void *v, size_type alignment = cs_alignment) noexcept;
      /**
       * \brief Return the address of the object_state from the allocated object memory.
       **/
      template <typename Object_State_Type>
      static Object_State_Type *from_object_start(void *v, size_type alignment = cs_alignment) noexcept;
      /**
       * \brief Set next, in_use, and next_valid at once.
       **/
      void set_all(object_state_base_t *next, bool in_use, bool next_valid, bool quasi_freed = false) noexcept;
      /**
       * \brief Set if the memory associated with this state is in use.
       **/
      void set_in_use(bool v) noexcept;
      /**
       * \brief Return false if the memory associated with this state is available for use, true otherwise.
       **/
      bool not_available() const noexcept;
      /**
       * \brief Return true if the memory associated with this state is in use, false otherwise.
       **/
      bool in_use() const noexcept;
      /**
       * \brief Return true if the memory associated with this state has been freed by another thread, but not processed yet.
       **/
      bool quasi_freed() const noexcept;
      /**
       * \brief Set that the memory associated with this state has been freed by another thread but not processed yet.
       **/
      void set_quasi_freed() noexcept;
      /**
       * \brief Set that the memory associated with this state has been freed by another thread but not processed yet.
       **/
      void set_quasi_freed(bool val) noexcept;
      /**
       * \brief Set if next state is valid.
       **/
      void set_next_valid(bool v) noexcept;
      /**
       * \brief Returns true if next state is valid, false otherwise.
       **/
      bool next_valid() const noexcept;
      /**
       * \brief Returns next state.
       **/
      object_state_base_t *next() const noexcept;
      /**
       * \brief Returns next state.
       **/
      template <typename Object_State_Type>
      Object_State_Type *next();
      /**
       * \brief Sets next state.
       *
       * Sets next state.
       * Clears next_valid state.
       **/
      void set_next(object_state_base_t *state) noexcept;
      /**
       * \brief Returns start of object.
       **/
      uint8_t *object_start(size_type alignment = cs_alignment) const noexcept;
      /**
       * \brief Returns size of memory associated with this state.
       **/
      size_type object_size(size_type alignment = cs_alignment) const noexcept;
      /**
       * \brief Returns end of object.
       **/
      uint8_t *object_end(size_type alignment = cs_alignment) const noexcept;
      /**
       * \brief Returns user data pointer.
       **/
      user_data_base_t *user_data() const noexcept;
      /**
       * \brief Sets user data pointer.
       *
       * Sets user data pointer.
       * User data must be 8 byte aligned.
       **/
      void set_user_data(void *user_data) noexcept;
      /**
       * \brief Return 3 user flags bitwise.
       **/
      size_type user_flags() const noexcept;
      /**
       * \brief Set user flags.
       **/
      void set_user_flags(size_type flags) noexcept;
      void verify_magic() noexcept;
      size_type m_pre_magic;
      /**
       * \brief Using pointer hiding, store next pointer, next_valid, in_use.
       *
       * Description is in little endian.
       * 0th bit is in_use
       * 1st bit is next_valid
       * 2nd bit is quasi-freed (allocator intends for free to happen during collect).
       **/
      uintptr_type m_next;
      /**
       * \brief Using pointer hiding, store user data pointer and 3 user flags.
       *
       * Description is in little endian.  Bottom 3 bits are user flags.
       **/
      uintptr_type m_user_data;
      size_type m_post_magic;
      static constexpr const size_type cs_pre_magic = 0x2ab78593;
      static constexpr const size_type cs_post_magic = 0x45a8cda0;
    };

    /**
     * \brief Object state for an object in allocator block.
     **/
    template <typename Allocator_Policy>
    class alignas(16) object_state_t : public object_state_base_t
    {
    public:
      using allocator_policy_type = Allocator_Policy;
      static_assert(::std::is_base_of<allocator_policy_tag_t, allocator_policy_type>::value,
                    "Allocator policy must have allocator_policy_tag_t");
    };
    static_assert(::std::is_pod<object_state_t<default_allocator_policy_t<::std::allocator<void>>>>::value,
                  "object_state_t is not POD");
    struct os_size_compare {
      inline auto operator()(const details::object_state_base_t *a, const details::object_state_base_t *b) const noexcept -> bool
      {
        if (a->object_size() < b->object_size()) {
          return true;
        }
        if (a->object_size() == b->object_size()) {
          return a < b;
        }
        return false;
      }
    };
  }
}
#include "object_state_impl.hpp"
