#pragma once
#include <cstdlib>
#include <mcppalloc/object_state.hpp>
#include <mcpputil/mcpputil/boost/container/flat_set.hpp>
#include <mcpputil/mcpputil/function_iterator.hpp>
#include <mcpputil/mcpputil/unsafe_cast.hpp>
namespace mcppalloc::sparse::details
{
  template <typename Block_Policy>
  class sparse_allocator_block_base_t
  {
  public:
    using block_policy_type = Block_Policy;
    using byte_pointer_type = typename block_policy_type::byte_pointer_type;
    using size_type = typename block_policy_type::size_type;
    using object_state_type = mcppalloc::details::object_state_base_t;

    sparse_allocator_block_base_t() = default;
    sparse_allocator_block_base_t(void *start, size_t length, size_t minimum_alloc_length, size_t object_state_type_size);
    sparse_allocator_block_base_t(sparse_allocator_block_base_t &) = delete;
    sparse_allocator_block_base_t(sparse_allocator_block_base_t &&) noexcept = default;
    sparse_allocator_block_base_t &operator=(const sparse_allocator_block_base_t &) = delete;
    sparse_allocator_block_base_t &operator=(sparse_allocator_block_base_t &&) noexcept = default;

    static constexpr size_t minimum_header_alignment() noexcept
    {
      return 16;
    }
    /**
     * \brief Return true if valid, false otherwise.
     **/
    bool valid() const noexcept;
    /**
     * \brief Return false if items are allocated.  Otherwise may return true or
     *false.
     *
     * Return true if no items allocated.
     * Return false if items are allocated or if no items are allocated and a
     *collection is needed. Note the semantics here!
     **/
    bool empty() const noexcept;
    /**
     * \brief Begin iterator to allow bytewise iteration over memory block.
     **/
    auto begin() const noexcept -> byte_pointer_type;
    /**
     * \brief End iterator to allow bytewise iteration over memory block.
     **/
    auto end() const noexcept -> byte_pointer_type;
    /**
     * \brief Return beginning object state.
     *
     * Produces undefined behavior if no valid object states.
     **/
    auto _object_state_begin() const noexcept -> object_state_type *;
    /**
     * \brief Return size of memory.
     **/
    auto memory_size() const noexcept -> size_type;
    /**
     * \brief End iterator for object_states
     **/
    auto current_end() const noexcept -> mcppalloc::details::object_state_base_t *;
    /**
     * \brief Find the object state associated with the given address.
     *
     * @return Associated object state, nullptr if not found.
     **/
    auto find_address(void *addr) const noexcept -> mcppalloc::details::object_state_base_t *;
    /**
     * \brief Find the object state associated with the given address.
     *
     * @return Associated object state, nullptr if not found.
     **/
    template <typename State_Type>
    auto find_address(void *addr) const noexcept -> State_Type *;

  protected:
    /**
     * \brief Next allocator pointer if whole block has not yet been used.
     **/
    ::mcppalloc::details::object_state_base_t *m_next_alloc_ptr;
    /**
     * \brief End of memory block.
     **/
    uint8_t *m_end;
    /**
     * \brief Minimum object allocation length.
     **/
    size_t m_minimum_alloc_length;
    /**
     * \brief Object state type size.
     **/
    size_t m_object_state_type_size;
    /**
     * \brief start of memory block.
     **/
    uint8_t *m_start;
  };
  template <typename Block_Policy>
  inline sparse_allocator_block_base_t<Block_Policy>::sparse_allocator_block_base_t(void *start,
                                                                                    size_t length,
                                                                                    size_t minimum_alloc_length,
                                                                                    size_t object_state_type_size)
      : m_next_alloc_ptr(reinterpret_cast<mcppalloc::details::object_state_base_t *>(start)),
        m_end(reinterpret_cast<uint8_t *>(start) + length),
        m_minimum_alloc_length(
            mcppalloc::details::object_state_base_t::needed_size(object_state_type_size, minimum_alloc_length)),
        m_object_state_type_size(object_state_type_size), m_start(reinterpret_cast<uint8_t *>(start))

  {
    if (mcpputil_unlikely(reinterpret_cast<size_t>(m_start) % minimum_header_alignment() != 0)) {
      ::std::abort();
    }
    if (mcpputil_unlikely(reinterpret_cast<size_t>(m_end) % minimum_header_alignment() != 0)) {
      ::std::abort();
    }
    // setup first object state
    static_cast<mcppalloc::details::object_state_base_t *>(m_next_alloc_ptr)
        ->set_all(reinterpret_cast<mcppalloc::details::object_state_base_t *>(reinterpret_cast<uint8_t *>(start) + length), false,
                  false, false);
  }

  template <typename Block_Policy>
  inline bool sparse_allocator_block_base_t<Block_Policy>::valid() const noexcept
  {
    return m_start != nullptr;
  }
  template <typename Block_Policy>
  bool sparse_allocator_block_base_t<Block_Policy>::empty() const noexcept
  {
    if (m_next_alloc_ptr == nullptr) {
      return false;
    }
    return reinterpret_cast<uint8_t *>(m_next_alloc_ptr) == begin() && !m_next_alloc_ptr->next_valid();
  }
  template <typename Block_Policy>
  MCPPALLOC_ALWAYS_INLINE auto sparse_allocator_block_base_t<Block_Policy>::begin() const noexcept -> byte_pointer_type
  {
    return m_start;
  }
  template <typename Block_Policy>
  MCPPALLOC_ALWAYS_INLINE auto sparse_allocator_block_base_t<Block_Policy>::end() const noexcept -> byte_pointer_type
  {
    return m_end;
  }
  template <typename Block_Policy>
  MCPPALLOC_ALWAYS_INLINE auto sparse_allocator_block_base_t<Block_Policy>::_object_state_begin() const noexcept
      -> object_state_type *
  {
    return mcpputil::unsafe_cast<object_state_type>(begin());
  }
  template <typename Block_Policy>
  auto sparse_allocator_block_base_t<Block_Policy>::memory_size() const noexcept -> size_type
  {
    return static_cast<size_type>(end() - begin());
  }
  template <typename Block_Policy>
  auto sparse_allocator_block_base_t<Block_Policy>::current_end() const noexcept -> ::mcppalloc::details::object_state_base_t *
  {
    if (m_next_alloc_ptr == nullptr) {
      return reinterpret_cast<::mcppalloc::details::object_state_base_t *>(end());
    }
    return m_next_alloc_ptr;
  }
  template <typename Block_Policy>
  auto sparse_allocator_block_base_t<Block_Policy>::find_address(void *addr) const noexcept
      -> mcppalloc::details::object_state_base_t *
  {
    for (auto it = mcpputil::make_next_iterator(_object_state_begin()); it != mcpputil::make_next_iterator(current_end()); ++it) {
      if (it->object_end() > addr) {
        return it;
      }
    }
    return nullptr;
  }
  template <typename Block_Policy>
  template <typename State_Type>
  auto sparse_allocator_block_base_t<Block_Policy>::find_address(void *addr) const noexcept -> State_Type *
  {
    return static_cast<State_Type *>(find_address(addr));
  }
} // namespace mcppalloc::sparse::details
