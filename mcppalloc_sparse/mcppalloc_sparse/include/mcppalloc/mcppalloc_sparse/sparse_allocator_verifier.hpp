#pragma once
#include "debug.hpp"
#include "declarations.hpp"
#include "functor.hpp"
#include <algorithm>
#include <iostream>
namespace mcppalloc::sparse::details
{
  class sparse_allocator_verifier_t
  {
  public:
    template <typename Allocator, typename Block>
    static void verify_block_new(Allocator &allocator, Block &block);
    // this is really expensive, but verify that blocks are sorted.
    template <typename Allocator>
    static void verify_blocks_sorted(Allocator &allocator);
  };
  // Definitions
  template <typename Allocator, typename Block>
  void sparse_allocator_verifier_t::verify_block_new(Allocator &allocator, Block &block)
  {
    if_constexpr(c_debug_level == 0)
    {
      return;
    }

    for (auto &&it : allocator.m_blocks) {
      // it is a fatal error to try to double add and something is inconsistent.  Terminate before memory corruption
      // spreads.
      if (it.m_block == &block)
        ::std::abort();
      // it is a fatal error to try to double add and something is inconsistent.  Terminate before memory corruption
      // spreads.
      if (it.m_begin == block.begin()) {
        ::std::cerr << " Attempt to double register block. 77dbea01-7e0f-49da-81f1-9ad7f4616eea\n";
        ::std::cerr << "77dbea01-7e0f-49da-81f1-9ad7f4616eea " << &block << " " << reinterpret_cast<void *>(block.begin())
                    << ::std::endl;
        ::std::abort();
      }
    }
  }
  template <typename Allocator>
  void sparse_allocator_verifier_t::verify_blocks_sorted(Allocator &allocator)
  {
    if_constexpr(!debug_verify_allocator_blocks_sorted)
    {
      return;
    }
    if (!allocator.m_blocks.empty() &&
        ::std::is_sorted(allocator.m_blocks.begin(), allocator.m_blocks.end(), block_handle_begin_compare_t{})) {
      ::std::cerr << "sparse allocator blocks out of order. 6df4aa74-b7b4-453d-a044-41f6d5e38b9b" << ::std::endl;
      ::std::abort();
    }
  }
}
