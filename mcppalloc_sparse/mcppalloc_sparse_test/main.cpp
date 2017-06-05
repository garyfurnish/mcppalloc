#include <mcpputil/mcpputil/declarations.hpp>
// This Must be first.
#include <mcppalloc/mcppalloc_sparse/mcppalloc_sparse.hpp>
#include <mcpputil/mcpputil/bandit.hpp>
#include <mcpputil/mcpputil/container_functions.hpp>
#ifdef _WIN32
#pragma optimize("", off)
#endif

using namespace ::bandit;
using namespace ::snowhouse;
extern void allocator_block_tests();
extern void allocator_block_set_tests();
extern void allocator_tests();
extern void slab_allocator_bandit_tests();

go_bandit([]() {
  slab_allocator_bandit_tests();
  describe("Pages", []() {
    it("test1", []() {
      mcpputil::slab_t page(mcpputil::slab_t::page_size() * 50,
                            mcpputil::slab_t::find_hole(mcpputil::slab_t::page_size() * 10000));
#ifndef __APPLE__
      AssertThat(page.expand(mcpputil::slab_t::page_size() * 5000), IsTrue());
      AssertThat(page.size() >= mcpputil::slab_t::page_size() * 5000, IsTrue());
#endif
      page.destroy();
    });
  });
  describe("Allocator", []() {
    allocator_block_tests();
    allocator_block_set_tests();
    allocator_tests();
    describe("thread_allocator", []() {
      void *memory1 = malloc(1000);
      void *memory2 = malloc(1000);
      it("find_block_set_id", []() {
        AssertThat(::mcppalloc::sparse::details::allocator_t<>::this_thread_allocator_t::find_block_set_id(16),
                   Equals(static_cast<size_t>(0)));
        AssertThat(::mcppalloc::sparse::details::allocator_t<>::this_thread_allocator_t::find_block_set_id(17),
                   Equals(static_cast<size_t>(1)));
        AssertThat(::mcppalloc::sparse::details::allocator_t<>::this_thread_allocator_t::find_block_set_id(32),
                   Equals(static_cast<size_t>(2)));
        AssertThat(::mcppalloc::sparse::details::allocator_t<>::this_thread_allocator_t::find_block_set_id(524287),
                   Equals(static_cast<size_t>(15)));
        AssertThat(::mcppalloc::sparse::details::allocator_t<>::this_thread_allocator_t::find_block_set_id(1000000),
                   Equals(static_cast<size_t>(16)));
        AssertThat(::mcppalloc::sparse::details::allocator_t<>::this_thread_allocator_t::find_block_set_id(1000000000),
                   Equals(static_cast<size_t>(17)));
      });
      free(memory1);
      free(memory2);
    });
  });
  describe("Algo", []() {
    it("insert_replace #1", []() {
      std::array<int, 5> in_array{{0, 1, 2, 3, 4}};
      std::array<int, 5> out_array{{0, 2, 3, 4, 55}};
      mcpputil::insert_replace(in_array.begin() + 1, in_array.begin() + 4, 55);
      AssertThat(in_array == out_array, IsTrue());
    });
    it("insert_replace #2", []() {
      std::array<int, 5> in_array{{0, 1, 2, 3, 4}};
      std::array<int, 5> out_array{{0, 2, 3, 3, 4}};
      mcpputil::insert_replace(in_array.begin() + 1, in_array.begin() + 2, 3);
      AssertThat(in_array == out_array, IsTrue());
    });
    it("insert_replace #3", []() {
      std::array<int, 2> in_array{{0, 1}};
      std::array<int, 2> out_array{{1, 2}};
      mcpputil::insert_replace(in_array.begin(), in_array.begin() + 1, 2);
      AssertThat(in_array == out_array, IsTrue());
    });
    it("insert_replace #3", []() {
      std::array<int, 2> in_array{{0, 1}};
      std::array<int, 2> out_array{{0, 5}};
      mcpputil::insert_replace(in_array.begin() + 1, in_array.begin() + 1, 5);
      AssertThat(in_array == out_array, IsTrue());
    });
    it("insert_replace #3", []() {
      std::array<int, 2> in_array{{0, 5}};
      std::array<int, 2> out_array{{2, 5}};
      mcpputil::insert_replace(in_array.begin(), in_array.begin(), 2);
      AssertThat(in_array == out_array, IsTrue());
    });
    it("rotate1", []() {
      ::std::array<int, 5> in_array{{0, 1, 2, 3, 4}};
      ::std::rotate(in_array.begin(), in_array.begin() + 2, in_array.end());
      ::std::array<int, 5> out_array{{2, 3, 4, 0, 1}};
      AssertThat(in_array == out_array, IsTrue());
    });
  });
});

int main(int argc, char *argv[])
{
  auto ret = bandit::run(argc, argv);
#ifdef _WIN32
  ::std::cin.get();
#endif
  return ret;
}
