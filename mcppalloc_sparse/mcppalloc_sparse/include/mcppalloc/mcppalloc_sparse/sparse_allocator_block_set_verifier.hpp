#pragma once
#include "debug.hpp"
#include "declarations.hpp"
#include "functor.hpp"
#include <algorithm>
#include <iostream>
namespace mcppalloc::sparse::details
{
  class sparse_allocator_block_set_verifier_t
  {
  public:
    template <typename Allocator_Block_Set>
    static void verify_magic_numbers(Allocator_Block_Set &abs)
    {
      if
        constexpr(debug_level == 0)
        {
          return;
        }

      if (mcpputil_unlikely(abs.m_magic_prefix != Allocator_Block_Set::cs_magic_prefix)) {
        ::std::cerr << "ABS MEMORY CORRUPTION 965b54ab-0eef-4878-8a0b-d4a4c5e62a0f\n";
        ::std::abort();
      }
    }
    template <typename Allocator_Block_Set>
    static void verify_available_blocks(Allocator_Block_Set &abs)
    {
      if_constexpr(debug_level == 0)
      {
        return;
      }

      for (auto &&ab : abs.m_available_blocks) {
        sparse_allocator_block_set_verifier_t::verify_available_block(abs, ab);
      }
    }
    template <typename Allocator_Block_Set, typename Available_Block>
    static void verify_available_block(Allocator_Block_Set &abs, Available_Block &ab)
    {
      if_constexpr(debug_level == 0)
      {
        return;
      }

      auto &block = *ab.second;
      if (mcpputil_unlikely(block.full())) {
        ::std::cerr << "mcppalloc: allocator_block_set available block full. cf9583b3-28aa-436e-83fc-ddf4f2300922\n";
        ::std::abort();
      }
      if (mcpputil_unlikely(block.last_max_alloc_available() != ab.first)) {
        ::std::cerr << "ABS CONSISTENCY ERROR e344d88d-87f6-47b3-bd04-2622241cb2bf\n";
        ::std::cerr << &block << ::std::endl;
        ::std::abort();
      }
      if (mcpputil_unlikely(block.max_alloc_available() != ab.first)) {
        ::std::cerr << "ABS CONSISTENCY ERROR e93cef0c-716f-4948-b036-76caa873299f\n";
        ::std::cerr << "min/max allocation sizes: (" << abs.allocator_min_size() << ", " << abs.allocator_max_size() << ")\n";
        ::std::cerr << "available:  " << ab.first << ::std::endl;
        ::std::cerr << &block << " " << block.valid() << " " << block.last_max_alloc_available() << ::std::endl;
        ::std::cerr << block.secondary_memory_used() << " " << block.memory_size() << " " << block.full() << ::std::endl;
        ::std::cerr << "recomp max alloc " << block.max_alloc_available() << ::std::endl;
        ::std::cerr << "free list size " << block.m_free_list.size() << ::std::endl;

        ::std::cerr << "available blocks\n";
        for (auto &&pair : abs.m_available_blocks) {
          ::std::cerr << pair.first << " " << pair.second << "\n";
        }

        ::std::abort();
        return;
      }

      if (mcpputil_unlikely(&block == abs.last_block())) {
        ::std::cerr << "ABS CONSISTENCY ERROR c5f0d1d7-d7b3-4572-8c0c-dcb0847fcc47\n";
        ::std::cerr << "Last block in available blocks\n";
        ::std::abort();
        return;
      }
      if (mcpputil_unlikely(block.full())) {
        ::std::cerr << "ABS CONSISTENCY ERROR 0d451014-2727-4a5d-a6e5-dce7e287f6d3\n";
        ::std::cerr << "Block full\n";
        ::std::abort();
        return;
      }
    }
    template <typename Allocator_Block_Set>
    static void verify_available_blocks_sorted(Allocator_Block_Set &abs)
    {
      const auto compare = lexographic_less_t();
      if_constexpr(!debug_verify_allocator_block_set_available_blocks_sorted)
      {
        return;
      }
      if (mcpputil_unlikely(!::std::is_sorted(abs.m_available_blocks.begin(), abs.m_available_blocks.end(), compare))) {
        ::std::cerr << "ABS CONSISTENCY ERROR 09b8c372-cd82-4980-aa97-b65fc441a499\n";
        ::std::cerr << "available blocks not sorted\n";
        ::std::cerr << "bad: \n";
        for (auto it = abs.m_available_blocks.begin(); it != abs.m_available_blocks.end() - 1; ++it) {
          if (!compare(*it, *(it + 1)))
            ::std::cerr << static_cast<size_t>(it - abs.m_available_blocks.begin()) << " " << it->first << " " << it->second
                        << " " << (it + 1)->first << " " << (it + 1)->second;
        }
        ::std::cerr << "all: \n";
        for (auto &&ab : abs.m_available_blocks)
          ::std::cerr << ab.first << " " << &ab.second << " ";
        ::std::abort();
        return;
      }
    }
    template <typename Allocator_Block_Set>
    static void verify_no_adjacent_duplicates(Allocator_Block_Set &abs)
    {
      if_constexpr(debug_level == 0)
      {
        return;
      }
      // make sure there are no duplicates in available blocks (since sorted, not a problem).
      if (mcpputil_unlikely(::std::adjacent_find(abs.m_available_blocks.begin(), abs.m_available_blocks.end()) !=
                            abs.m_available_blocks.end())) {
        ::std::cerr << "ABS CONSISTENCY ERROR aa35c18e-9bef-4602-a25a-24154153279a\n";
        ::std::cerr << "available blocks contains duplicates\n";
        ::std::abort();
      }
    }
    template <typename Allocator_Block_Set>
    static void verify_all(Allocator_Block_Set &abs)
    {
      if_constexpr(debug_level)
      {
        sparse_allocator_block_set_verifier_t::verify_magic_numbers(abs);
        sparse_allocator_block_set_verifier_t::verify_available_blocks(abs);
        sparse_allocator_block_set_verifier_t::verify_available_blocks_sorted(abs);
        sparse_allocator_block_set_verifier_t::verify_no_adjacent_duplicates(abs);
      }
    }
  };
}
