#pragma once
namespace mcppalloc::sparse::details
{
  /**
   * \brief Functor to compare block handles by beginning locations.
   **/
  struct block_handle_begin_compare_t {
    template <typename T1>
    bool operator()(const T1 &h1, const T1 &h2)
    {
      return h1.m_begin < h2.m_begin;
    }
  };
}
