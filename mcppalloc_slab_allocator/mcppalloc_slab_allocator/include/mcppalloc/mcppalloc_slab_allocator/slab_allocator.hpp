#pragma once
#include "slab_allocator_dll.hpp"
#include <array>
#include <boost/property_tree/ptree_fwd.hpp>
#include <mcppalloc/object_state.hpp>
#include <mcpputil/mcpputil/alignment.hpp>
#include <mcpputil/mcpputil/backed_ordered_map.hpp>
#include <mcpputil/mcpputil/concurrency.hpp>
#include <mcpputil/mcpputil/function_iterator.hpp>
#include <mcpputil/mcpputil/memory_range.hpp>
#include <mcpputil/mcpputil/posix_slab.hpp>
#include <mcpputil/mcpputil/win32_slab.hpp>
namespace mcppalloc::slab_allocator::details
{
  using mutex_type = mcpputil::mutex_t;
  using slab_allocator_object_t = ::mcppalloc::details::object_state_base_t;
  static_assert(::std::is_pod<slab_allocator_object_t>::value, "slab_allocator_object_t is not POD");
  /**
   * \brief This is a thread safe reentrant* slab allocator.
   *
   * This is essentially a thin layer over slab.
   * It is required that the lock not be held for reenterancy to be true.
   * It differs from allocator_t in that this allocator is completely in place.
   * This is both less memory and runtime efficient, but can be used to back the allocator_t.
   **/
  class slab_allocator_t
  {
  public:
    /**
     * \brief Type of pointer for slab allocator.
     **/
    using pointer_type = uint8_t *;
    /**
     * \brief Type of memory range.
     **/
    using memory_range_type = mcpputil::memory_range_t<pointer_type>;

    static inline constexpr const size_t cs_alignment = 64;
    static inline constexpr const size_t cs_header_sz = mcpputil::cs_align(sizeof(slab_allocator_object_t), cs_alignment);
    static constexpr size_t alignment() noexcept;
    static_assert(cs_header_sz == 64, "");
    /**
     * \brief Constructor.
     *
     * @param size Minimum size.
     * @param size_hint Suggested expand size.
     **/
    slab_allocator_t(size_t size, size_t size_hint);
    slab_allocator_t(const slab_allocator_t &) = delete;
    slab_allocator_t(slab_allocator_t &&) = delete;
    ~slab_allocator_t();
    void _verify() REQUIRES(!m_mutex);
    /**
     * \brief Align the next allocation to the given size.
     * This is only guarenteed to work if done before any deallocation.
     **/
    void align_next(size_t sz) REQUIRES(!m_mutex);
    /**
     * \brief Return start address of memory slab.
     **/
    uint8_t *begin() const noexcept;
    /**
     * \brief Return end address of memory slab.
     **/
    uint8_t *end() const noexcept;
    /**
     * \brief Return memory range of memory slab.
     **/
    auto memory_range() const noexcept -> memory_range_type;
    /**
     * \brief Object state begin iterator.
     **/
    mcpputil::next_iterator<slab_allocator_object_t> _u_object_begin();
    /**
     * \brief Object state end iterator.
     **/
    mcpputil::next_iterator<slab_allocator_object_t> _u_object_end();
    /**
     * \brief Object state current end iterator.
     **/
    mcpputil::next_iterator<slab_allocator_object_t> _u_object_current_end();
    /**
     * \brief Return true if no memory allocated.
     **/
    bool _u_empty();
    /**
     * \brief Return lock for this allocator.
     **/
    mutex_type &_mutex() RETURN_CAPABILITY(m_mutex)
    {
      return m_mutex;
    }
    /**
     * \brief Split an allocator object state.
     *
     * Requires holding lock.
     * @param object Object to split
     * @param sz Size of object allocation required.
     * @return Start of object memory.
     **/
    void *_u_split_allocate(slab_allocator_object_t *object, size_t sz) REQUIRES(m_mutex);
    /**
     * \brief Allocate memory at end.
     *
     * Requires holding lock.
     * @param sz Size of object allocation required.
     * @return Start of object memory.
     **/
    void *_u_allocate_raw_at_end(ptrdiff_t sz) REQUIRES(m_mutex);
    /**
     * \brief Allocate memory.
     *
     * Requires holding lock.
     * @param sz Size of object allocation required.
     * @return Start of object memory.
     **/
    void *allocate_raw(size_t sz) REQUIRES(!m_mutex);
    /**
     * \brief Allocate memory.
     *
     * Deallocate memory.
     * @param v Memory to deallocate.
     **/
    void deallocate_raw(void *v) REQUIRES(!m_mutex);
    /**
     * \brief Return offset from start of slab for pointer.
     **/
    ptrdiff_t offset(void *v) const noexcept;
    /**
     * \brief Return currently used size.
     **/
    auto current_size() const noexcept -> size_t;
    /**
     * \brief Put information about slab allocator into a property tree.
     * @param level Level of information to give.  Higher is more verbose.
     **/
    void to_ptree(::boost::property_tree::ptree &ptree, int level) const;

  private:
    void _u_add_free(slab_allocator_object_t *v) REQUIRES(m_mutex);
    void _u_remove_free(slab_allocator_object_t *v) REQUIRES(m_mutex);
    void _u_generate_free_list() REQUIRES(m_mutex);
    void _u_move_free(slab_allocator_object_t *orig, slab_allocator_object_t *new_obj) REQUIRES(m_mutex);
    /**
     * \brief Mutex for allocator.
     **/
    mutex_type m_mutex;
    /**
     * \brief Underlying slab.
     **/
    mcpputil::slab_t m_slab;
    /**
     * \brief Current position of end object state (invalid).
     **/
    slab_allocator_object_t *m_end;
    /**
     * \brief True if free map overflowed and dumped free positions on the floor.
     **/
    bool m_free_map_needs_regeneration{false};
    using free_map_type = mcpputil::containers::backed_ordered_multimap<size_t, slab_allocator_object_t *>;
    /**
     * \brief Map of free positions.
     **/
    free_map_type m_free_map;
    /**
     * \brief Array backing for free map.
     **/
    ::std::array<typename free_map_type::value_type, 1500> m_free_map_back;
  };
  constexpr inline size_t slab_allocator_t::alignment() noexcept
  {
    return cs_alignment;
  }
  inline uint8_t *slab_allocator_t::begin() const noexcept
  {
    return m_slab.begin();
  }
  inline uint8_t *slab_allocator_t::end() const noexcept
  {
    return m_slab.end();
  }
  inline auto slab_allocator_t::memory_range() const noexcept -> memory_range_type
  {
    return memory_range_type{begin(), end()};
  }
  inline mcpputil::next_iterator<slab_allocator_object_t> slab_allocator_t::_u_object_begin()
  {
    return mcpputil::make_next_iterator(reinterpret_cast<slab_allocator_object_t *>(begin()));
  }
  inline mcpputil::next_iterator<slab_allocator_object_t> slab_allocator_t::_u_object_end()
  {
    return mcpputil::make_next_iterator(reinterpret_cast<slab_allocator_object_t *>(end()));
  }
  inline mcpputil::next_iterator<slab_allocator_object_t> slab_allocator_t::_u_object_current_end()
  {
    return mcpputil::make_next_iterator(reinterpret_cast<slab_allocator_object_t *>(m_end));
  }
  inline bool slab_allocator_t::_u_empty()
  {
    return _u_object_current_end() == _u_object_begin();
  }
}
