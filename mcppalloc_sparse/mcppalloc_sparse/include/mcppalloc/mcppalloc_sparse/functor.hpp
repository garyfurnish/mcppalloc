#pragma once
#include <utility>
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
  struct first_is_less_t {
    template <typename A>
    constexpr auto operator()(const A &lhs, const A &rhs) const -> bool
    {
      return lhs.first < rhs.first;
    }
  };
  struct lexographic_less_t {
    template <typename A, typename B>
    constexpr auto operator()(const ::std::pair<A, B> &lhs, const ::std::pair<A, B> &rhs) const -> bool
    {
      if (lhs.first < rhs.first) {
        return true;
      }
      if (lhs.first == rhs.first) {
        return lhs.second < rhs.second;
      }
      return false;
    }
  };
} // namespace mcppalloc::sparse::details
