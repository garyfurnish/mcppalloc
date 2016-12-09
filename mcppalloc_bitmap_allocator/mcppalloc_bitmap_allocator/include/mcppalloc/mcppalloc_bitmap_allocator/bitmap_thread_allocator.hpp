#pragma once
#include "bitmap_package.hpp"
#include "bitmap_state.hpp"
#include "declarations.hpp"
#include <mcppalloc/block.hpp>
#include <mcpputil/mcpputil/boost/container/flat_map.hpp>
namespace mcppalloc::bitmap_allocator::details
{
  template <typename Allocator_Policy>
  class bitmap_thread_allocator_t
  {
  public:
    using allocator_policy_type = Allocator_Policy;
    using package_type = bitmap_package_t<allocator_policy_type>;
    using block_type = block_t<allocator_policy_type>;
    using internal_allocator_type = typename allocator_policy_type::internal_allocator_type;
    using internal_allocator_traits = typename ::std::allocator_traits<internal_allocator_type>;
    bitmap_thread_allocator_t(bitmap_allocator_t<allocator_policy_type> &allocator);
    bitmap_thread_allocator_t(const bitmap_thread_allocator_t &) = delete;
    bitmap_thread_allocator_t(bitmap_thread_allocator_t &&) noexcept;
    bitmap_thread_allocator_t &operator=(const bitmap_thread_allocator_t &) = delete;
    bitmap_thread_allocator_t &operator=(bitmap_thread_allocator_t &&) = default;
    ~bitmap_thread_allocator_t();

    auto allocate(size_t sz, type_id_t package_type = 0) -> block_type;
    auto allocate(size_t sz, package_type &package) -> block_type;
    auto deallocate(void *v) noexcept -> bool;
    auto deallocate(void *v, package_type &package) noexcept -> bool;

    void set_max_in_use(size_t max_in_use);
    void set_max_free(size_t max_free);
    auto max_in_use() const noexcept -> size_t;
    auto max_free() const noexcept -> size_t;

    void do_maintenance(package_type &package);
    void do_maintenance();

    template <typename Predicate>
    void for_all_state(Predicate &predicate, package_type &package);
    template <typename Predicate>
    void for_all_state(Predicate &predicate);
    /**
     * \brief Tell thread to perform maintenance at next opportunity.
     **/
    void set_force_maintenance();

    /**
     * \brief Put information about allocator into a property tree.
     * @param level Level of information to give.  Higher is more verbose.
     **/
    void to_ptree(::boost::property_tree::ptree &ptree, int level);

  private:
    auto get_package_by_type(type_id_t type_id) -> package_type &;

    void _check_maintenance();
    using locals_pair_type = typename ::std::pair<type_id_t, package_type>;
    using locals_allocator_type = typename internal_allocator_traits::template rebind_alloc<locals_pair_type>;
    ::boost::container::flat_map<type_id_t, package_type, ::std::less<type_id_t>, locals_allocator_type> m_locals;
    ::std::atomic<bool> m_force_maintenance;
    mcpputil::rebind_vector_t<void *, internal_allocator_type> m_free_list;
    bitmap_allocator_t<allocator_policy_type> &m_allocator;
    ::std::array<size_t, package_type::cs_num_vectors> m_popcount_max;
    size_t m_max_in_use{10};
    size_t m_max_free{5};
    bool m_in_destructor{false};
  };
}
#include "bitmap_thread_allocator_impl.hpp"
