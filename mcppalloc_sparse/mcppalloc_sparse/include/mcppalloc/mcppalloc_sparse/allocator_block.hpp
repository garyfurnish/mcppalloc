#pragma once
#include "sparse_allocator_block_base.hpp"
#include <algorithm>
#include <mcppalloc/block.hpp>
#include <mcppalloc/default_allocator_policy.hpp>
#include <mcpputil/mcpputil/boost/property_tree/ptree_fwd.hpp>
#include <mcpputil/mcpputil/container.hpp>
#include <mcpputil/mcpputil/make_unique.hpp>
#include <string>
#include <vector>
namespace mcppalloc
{
  template <typename Policy, typename OS>
  auto get_allocated_memory(const ::std::tuple<block_t<Policy>, OS *> &allocation)
  {
    return ::std::get<0>(allocation).m_ptr;
  }
  template <typename Policy, typename OS>
  auto get_allocated_size(const ::std::tuple<block_t<Policy>, OS *> &allocation)
  {
    return ::std::get<0>(allocation).m_size;
  }
  template <typename Policy, typename OS>
  bool allocation_valid(const ::std::tuple<block_t<Policy>, OS *> &allocation)
  {
    return nullptr != get_allocated_memory(allocation);
  }
  template <typename Policy, typename OS>
  auto get_allocation_object_state(const ::std::tuple<block_t<Policy>, OS *> &allocation)
  {
    return ::std::get<1>(allocation);
  }
  namespace sparse::details
  {
    struct default_sparse_allocator_block_policy_t {
      using byte_pointer_type = uint8_t *;
      using size_type = size_t;
    };
    /**
     * \brief Allocator block.
     *
     * This is designed to be moved to where it is used for cache linearity.
     * The fields in this should be organized to maximize cache hits.
     * This class is incredibly cache layout sensitive.
     *
     * Internally this implements a linked list of object_state_t followed by data.
     * The last valid object_state_t always points to an object_state_t at end().
     **/
    template <typename Allocator_Policy>
    class allocator_block_t : public sparse_allocator_block_base_t<default_sparse_allocator_block_policy_t>
    {
    public:
      using size_type = size_t;
      using allocator_policy_type = Allocator_Policy;
      static_assert(::std::is_base_of<allocator_policy_tag_t, allocator_policy_type>::value,
                    "Allocator policy must be allocator_policy");
      using allocator = typename allocator_policy_type::internal_allocator_type;
      using user_data_type = typename allocator_policy_type::user_data_type;
      using object_state_type = ::mcppalloc::details::object_state_t<allocator_policy_type>;
      static user_data_type s_default_user_data;
      using block_type = block_t<allocator_policy_type>;
      using allocation_return_type = ::std::tuple<block_type, object_state_type *>;

      allocator_block_t() = default;
      /**
       * \brief Constructor
       * @param start Start of memory block that this allocator uses.
       * @param length Length of memory block that this allocator uses
       * @param minimum_alloc_length Minimum length of object that can be allocated using this allocator.
       * @param maximum_alloc_length Maximum length of object that can be allocated using this allocator.
      **/
      MCPPALLOC_ALWAYS_INLINE
      allocator_block_t(void *start, size_t length, size_t minimum_alloc_length, size_t maximum_alloc_length) noexcept;
      allocator_block_t(const allocator_block_t &) = delete;
      MCPPALLOC_ALWAYS_INLINE allocator_block_t(allocator_block_t &&) noexcept;
      allocator_block_t &operator=(const allocator_block_t &) = delete;
      MCPPALLOC_ALWAYS_INLINE allocator_block_t &operator=(allocator_block_t &&) noexcept;
      ~allocator_block_t();
      /**
       * \brief Allocate size bytes on the block.
       *
       * @return Valid pointer if possible, nullptr otherwise.
      **/
      auto allocate(size_t size) -> allocation_return_type;
      /**
       * \brief Destroy a v that is on the block.
       *
       * The usual cause of failure would be the pointer not being in the block.
       * @return True on success, false on failure.
      **/
      bool destroy(void *v);
      /**
       * \brief Destroy a v that is on the block.
       *
       * The usual cause of failure would be the pointer not being in the block.
       * @param v Pointer to destroy.
       * @param last_collapsed_size Max size of allocation made available by destroying.
       * @param last_max_alloc_available Return the previous last max alloc available.
       * @return True on success, false on failure.
      **/
      bool destroy(void *v, size_t &last_collapsed_size, size_t &last_max_alloc_available);
      /**
       * \brief Collect any adjacent blocks that may have formed into one block.
       * @param num_quasifreed Increment by number of quasifreed found.
      **/
      void collect(size_t &num_quasifreed);
      /**
       * \brief Return the maximum allocation size available.
      **/
      size_t max_alloc_available();
      /**
       * \brief Verify object state os.
       *
       * Code may optimize out in release mode.
       * @param os Object state to verify.
       **/
      void _verify(const object_state_type *os);
      /**
       * \brief Clear all control structures and invalidate.
       **/
      void clear();
      /**
       * \brief Return true if no more allocations can be performed.
       **/
      bool full() const noexcept;
      /**
       * \brief Return minimum object allocation length.
      **/
      size_t minimum_allocation_length() const;
      /**
       * \brief Return maximum object allocation length.
      **/
      size_t maximum_allocation_length() const;
      /**
       * \brief Return updated last max alloc available.
       **/
      size_t last_max_alloc_available() const noexcept;
      /**
       * \brief Return the bytes of secondary memory used.
       **/
      size_t secondary_memory_used() const noexcept;
      /**
       * \brief Shrink secondary data structures to fit.
       **/
      void shrink_secondary_memory_usage_to_fit();
      /**
       * \brief Put information about abs into a property tree.
       * @param level Level of information to give.  Higher is more verbose.
       **/
      void to_ptree(::boost::property_tree::ptree &ptree, int level) const;

    public:
      /**
       * \brief Default user data option.
       *
       * The allocated memory is stored in control allocator.
       **/
      mcpputil::allocator_unique_ptr_t<user_data_type, allocator> m_default_user_data;
      /**
       * \brief Updated last max alloc available.
       *
       * Designed to sync with allocator block sets.
       **/
      size_t m_last_max_alloc_available = 0;
      /**
       * \brief Maximum object allocation length.
       *
       * This is at end as it is not needed for allocation (cache allignment).
       **/
      size_t m_maximum_alloc_length = 0;
      /**
       * \brief Free list for this block.
       *
       * Uses control allocator for control data.
       **/
      ::boost::container::flat_set<mcppalloc::details::object_state_base_t *,
                                   mcppalloc::details::os_size_compare,
                                   typename allocator::template rebind<mcppalloc::details::object_state_base_t *>::other>
          m_free_list;
    };

    template <typename Allocator_Policy>
    inline bool allocator_block_t<Allocator_Policy>::full() const noexcept
    {
      return m_next_alloc_ptr == nullptr && m_free_list.empty();
    }
    template <typename Block_Policy>
    void allocator_block_t<Block_Policy>::clear()
    {
      m_free_list.clear();
      m_next_alloc_ptr = nullptr;
      m_end = nullptr;
      m_start = nullptr;
    }
  }
}
#include "allocator_block_impl.hpp"
