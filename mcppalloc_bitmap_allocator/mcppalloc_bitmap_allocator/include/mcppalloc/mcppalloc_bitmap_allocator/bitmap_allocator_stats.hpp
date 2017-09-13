#pragma once
#include "declarations.hpp"
namespace mcppalloc::bitmap_allocator
{
  class bitmap_allocator_stats_t
  {
  public:
    auto num_free_globals() -> ptrdiff_t;
    void set_num_free_globals(ptrdiff_t num_free_globals);

  private:
    ptrdiff_t m_num_free_globals{0};
  };
  auto bitmap_allocator_stats_t::num_free_globals() -> ptrdiff_t
  {
    return m_num_free_globals;
  }
  void bitmap_allocator_stats_t::set_num_free_globals(ptrdiff_t num_free_globals)
  {
    m_num_free_globals = num_free_globals;
  }
} // namespace mcppalloc::bitmap_allocator
