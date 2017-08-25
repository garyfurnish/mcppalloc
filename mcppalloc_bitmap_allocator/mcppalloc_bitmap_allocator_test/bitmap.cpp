#include <mcppalloc/mcppalloc_bitmap_allocator/bitmap_allocator.hpp>
#include <mcppalloc/mcppalloc_slab_allocator/slab_allocator.hpp>
#include <mcpputil/mcpputil/bandit.hpp>
#include <mcpputil/mcpputil/security.hpp>

using bitmap_allocator =
    mcppalloc::bitmap_allocator::details::bitmap_allocator_t<mcppalloc::default_allocator_policy_t<std::allocator<void>>>;
template <>
::std::vector<bitmap_allocator::thread_allocator_type *> bitmap_allocator::m_thread_allocator_by_manager_id{};

using namespace bandit;
using namespace ::snowhouse;
using namespace ::mcpputil::literals;

static void bitmap_state_test0()
{
  ::mcppalloc::bitmap_allocator::details::bitmap_package_t<::mcppalloc::default_allocator_policy_t<::std::allocator<void>>>
      *null_package = nullptr;
  using ::mcppalloc::bitmap_allocator::details::c_bitmap_block_size;
  using allocator_type =
      ::mcppalloc::bitmap_allocator::bitmap_allocator_t<::mcppalloc::default_allocator_policy_t<::std::allocator<void>>>;
  allocator_type bitmap_allocator{};
  bitmap_allocator.initialize(2000000, 2000000);
  auto &fast_slab = bitmap_allocator.underlying_memory();
  fast_slab.align_next(c_bitmap_block_size);
  constexpr const size_t packed_size = c_bitmap_block_size - 2048;
  uint8_t *ret = reinterpret_cast<uint8_t *>(fast_slab.allocate_raw(packed_size));
  AssertThat(reinterpret_cast<uintptr_t>(ret) % 4096, Equals(allocator_type::slab_allocator_type::cs_header_sz));
  constexpr const size_t expected_entries = 512;
  using ps_type = ::mcppalloc::bitmap_allocator::details::bitmap_state_t;
  static_assert(sizeof(ps_type) <= packed_size, "");
  AssertThat(reinterpret_cast<uintptr_t>(ret) % 32, Equals(0_sz));
  auto ps = new (ret) ps_type();
  (void)ps;

  constexpr const mcppalloc::bitmap_allocator::details::bitmap_state_info_t state{1ul, 64ul, expected_entries, 0, 0, nullptr,
                                                                                  0,   0,    {0, 0, 0}};
  ps->m_internal.m_info = state;
  ps->_compute_size();
  AssertThat(ps->size(), Equals(expected_entries));
  AssertThat(static_cast<size_t>(ps->end() - ret), IsLessThanOrEqualTo(packed_size));
  auto max_end = ret + packed_size;
  AssertThat(max_end, IsGreaterThanOrEqualTo(ps->end()));
  AssertThat(ps->size(), Equals(expected_entries));
  ps->initialize(0, 0, null_package);
  AssertThat(ps->any_free(), IsTrue());
  AssertThat(ps->none_free(), IsFalse());
  AssertThat(ps->first_free(), Equals(0_sz));
  ps->set_free(0, false);
  AssertThat(ps->first_free(), Equals(1_sz));
  for (size_t i = 0; i < 64; ++i) {
    ps->set_free(i, false);
  }
  AssertThat(ps->first_free(), Equals(64_sz));
  AssertThat(ps->is_free(53), IsFalse());
  AssertThat(ps->is_free(64), IsTrue());
  for (size_t i = 0; i < 255; ++i) {
    ps->set_free(i, false);
  }
  AssertThat(ps->first_free(), Equals(255_sz));
  ps->set_free(69, true);
  AssertThat(ps->first_free(), Equals(69_sz));

  ps->clear_mark_bits();
  AssertThat(ps->any_marked(), IsFalse());
  AssertThat(ps->none_marked(), IsTrue());
  ps->set_marked(0);
  AssertThat(ps->any_marked(), IsTrue());
  AssertThat(ps->none_marked(), IsFalse());
  for (size_t i = 0; i < 64; ++i) {
    if ((i % 2) != 0u) {
      ps->set_marked(i);
    }
  }
  AssertThat(ps->is_marked(0), IsTrue());
  AssertThat(ps->is_marked(31), IsTrue());
  AssertThat(ps->is_marked(33), IsTrue());
  for (size_t i = 200; i < 255; ++i) {
    if ((i % 2) != 0u) {
      ps->set_marked(i);
    }
  }
  for (size_t i = 1; i < 255; ++i) {
    if (((i % 2) != 0u) && (i < 64 || (i >= 200))) {
      AssertThat(ps->is_marked(i), IsTrue());
    } else {
      AssertThat(ps->is_marked(i), IsFalse());
    }
  }
  {
    ps->initialize(0, 0, null_package);
    ps->clear_mark_bits();
    AssertThat(ps->is_free(0), IsTrue());
    AssertThat(ps->is_free(1), IsTrue());
    void *ptr = ps->allocate();
    AssertThat(ptr != nullptr, IsTrue());
    AssertThat(ps->is_free(0), IsFalse());
    AssertThat(ps->is_free(1), IsTrue());
    AssertThat(ps->deallocate(ptr), IsTrue());
    AssertThat(ps->is_free(0), IsTrue());
    AssertThat(ps->is_free(1), IsTrue());
  }
  ps->initialize(0, 0, null_package);
  ps->clear_mark_bits();
  ::std::vector<void *> ptrs;
  bool keep_going = true;
  AssertThat(ps->num_blocks() * ps_type::bits_array_type::size(), Equals(8_sz));
  while (keep_going) {
    void *ptr = ps->allocate();
    if (ptr != nullptr) {
      ptrs.push_back(ptr);
    } else {
      keep_going = false;
    }
  }
  AssertThat(ptrs.size(), Equals(expected_entries));
  for (size_t i = 0; i < ptrs.size(); ++i) {
    if ((i % 3) != 0u) {
      AssertThat(ps->deallocate(ptrs[i]), IsTrue());
    }
  }
  for (size_t i = 0; i < ptrs.size(); ++i) {
    if ((i % 3) != 0u) {
      AssertThat(ps->is_free(i), IsTrue());
    } else {
      AssertThat(ps->is_free(i), IsFalse());
    }
  }
  AssertThat(ps->is_free(3), IsFalse());
  AssertThat(ps->is_free(6), IsFalse());
  ps->set_marked(3);
  AssertThat(ps->is_marked(3), IsTrue());
  AssertThat(ps->is_marked(6), IsFalse());
  ps->free_unmarked();
  AssertThat(ps->is_free(3), IsFalse());
  AssertThat(ps->is_free(6), IsTrue());
  ps->set_free(3, true);
  AssertThat(ps->is_free(3), IsTrue());
  fast_slab.deallocate_raw(ret);
}

static void multiple_slab_test0()
{
  using allocator_type =
      ::mcppalloc::bitmap_allocator::bitmap_allocator_t<::mcppalloc::default_allocator_policy_t<::std::allocator<void>>>;
  allocator_type bitmap_allocator{};
  bitmap_allocator.initialize(2000000, 2000000);
  auto &fast_slab = bitmap_allocator.underlying_memory();
  constexpr const size_t packed_size = 4096 * 16;
  uint8_t *ret = reinterpret_cast<uint8_t *>(fast_slab.allocate_raw(packed_size));
  AssertThat(reinterpret_cast<uintptr_t>(ret) % 4096, Equals(allocator_type::slab_allocator_type::cs_header_sz));
  constexpr const size_t entry_size = 128ul;
  constexpr const size_t expected_entries = packed_size / entry_size - 2;
  using ps_type = ::mcppalloc::bitmap_allocator::details::bitmap_state_t;
  static_assert(sizeof(ps_type) <= packed_size, "");
  AssertThat(reinterpret_cast<uintptr_t>(ret) % 32, Equals(0_sz));
  auto ps = new (ret) ps_type();
  (void)ps;

  constexpr const mcppalloc::bitmap_allocator::details::bitmap_state_info_t state{
      1ul, entry_size, expected_entries, sizeof(mcppalloc::bitmap_allocator::details::bitmap_state_info_t) + 128, 0, nullptr,
      0,   0,          {0, 0, 0}};
  ps->m_internal.m_info = state;
  AssertThat(ps->size(), Equals(expected_entries));
  AssertThat(ps->total_size_bytes(),
             Equals(expected_entries * entry_size + sizeof(mcppalloc::bitmap_allocator::details::bitmap_state_info_t) + 128));
  AssertThat(static_cast<size_t>(ps->end() - ret), Is().LessThanOrEqualTo(packed_size));
  fast_slab.deallocate_raw(ret);
}

static void multiple_slab_test0b()
{
  using allocator_type =
      ::mcppalloc::bitmap_allocator::bitmap_allocator_t<::mcppalloc::default_allocator_policy_t<::std::allocator<void>>>;
  allocator_type bitmap_allocator{};
  bitmap_allocator.initialize(2000000, 2000000);
  auto &fast_slab = bitmap_allocator.underlying_memory();
  constexpr const size_t packed_size = 4096 * 32;
  uint8_t *ret = reinterpret_cast<uint8_t *>(fast_slab.allocate_raw(packed_size));
  AssertThat(!!ret, IsTrue());
  AssertThat(reinterpret_cast<uintptr_t>(ret) % 4096, Equals(allocator_type::slab_allocator_type::cs_header_sz));
  constexpr const size_t entry_size = 128ul;
  constexpr const size_t expected_entries = packed_size / entry_size - 3;
  using ps_type = ::mcppalloc::bitmap_allocator::details::bitmap_state_t;
  static_assert(sizeof(ps_type) <= packed_size, "");
  AssertThat(reinterpret_cast<uintptr_t>(ret) % 32, Equals(0_sz));
  auto ps = new (ret) ps_type();
  (void)ps;

  constexpr const mcppalloc::bitmap_allocator::details::bitmap_state_info_t state{
      2ul, entry_size, expected_entries, sizeof(mcppalloc::bitmap_allocator::details::bitmap_state_info_t) + 256, 0, nullptr,
      0,   0,          {0, 0, 0}};
  ps->m_internal.m_info = state;
  AssertThat(ps->size(), Equals(expected_entries));
  AssertThat(ps->total_size_bytes(),
             Equals(expected_entries * entry_size + sizeof(mcppalloc::bitmap_allocator::details::bitmap_state_info_t) + 256));
  AssertThat(static_cast<size_t>(ps->end() - ret), Is().LessThanOrEqualTo(packed_size));
  fast_slab.deallocate_raw(ret);
}

static void multiple_slab_test1()
{
  using allocator_type =
      ::mcppalloc::bitmap_allocator::bitmap_allocator_t<::mcppalloc::default_allocator_policy_t<::std::allocator<void>>>;
  allocator_type bitmap_allocator{};
  bitmap_allocator.initialize(2000000, 2000000); // TODO: Commensting this out yields a crash
  bitmap_allocator.add_type(::mcppalloc::bitmap_allocator::details::bitmap_type_info_t(0, 0));
  allocator_type::package_type package1(0);
  allocator_type::package_type package2(0);
  package1.insert(::std::move(package2));
  auto info0 = allocator_type::package_type::_get_info(0);
  AssertThat(info0.m_num_blocks, Equals(allocator_type::package_type::cs_total_size / 512 / 32));
  AssertThat(info0.m_data_entry_sz, Equals(32_sz));
  auto info1 = allocator_type::package_type::_get_info(1);
  AssertThat(info1.m_num_blocks, Equals(allocator_type::package_type::cs_total_size / 512 / 64));
  AssertThat(info1.m_data_entry_sz, Equals(64_sz));

  auto &poa = bitmap_allocator;
  auto &ta = poa.initialize_thread();
  {
    void *v = ta.allocate(128).m_ptr;
    AssertThat(v != nullptr, IsTrue());
    AssertThat(ta.deallocate(v), IsTrue());
    uint8_t *v2 = reinterpret_cast<uint8_t *>(ta.allocate(128).m_ptr);
    uint8_t *v3 = reinterpret_cast<uint8_t *>(ta.allocate(128).m_ptr);
    AssertThat(reinterpret_cast<void *>(v), Equals(reinterpret_cast<void *>(v2)));
    AssertThat(reinterpret_cast<void *>(v3), Equals(reinterpret_cast<void *>(v2 + 128)));
    ta.deallocate(v2);
    uint8_t *v4 = reinterpret_cast<uint8_t *>(ta.allocate(128).m_ptr);
    AssertThat(reinterpret_cast<void *>(v2), Equals(reinterpret_cast<void *>(v4)));
    ta.deallocate(v3);
    ta.deallocate(v4);
  }
  poa.destroy_thread();
  // make sure destroying twice does not crash.
  poa.destroy_thread();
  AssertThat(poa.num_free_blocks(), Equals(1_sz));
  for (size_t i = 0; i < allocator_type::package_type::cs_num_vectors; ++i)
    AssertThat(poa.num_globals(i, 0), Equals(0_sz));
  // ok, now take and check that destroying with a valid block puts it in globals not freed.
  auto &ta2 = poa.initialize_thread();
  {
    void *v = ta2.allocate(128).m_ptr;
    AssertThat(v != nullptr, IsTrue());
  }
  poa.destroy_thread();
  AssertThat(poa.num_free_blocks(), Equals(0_sz));
  AssertThat(poa.num_globals(mcppalloc::bitmap_allocator::details::get_bitmap_size_id(128), 0), Equals(1_sz));
  AssertThat(mcppalloc::bitmap_allocator::details::get_bitmap_size_id(31), Equals(0_sz));
  AssertThat(mcppalloc::bitmap_allocator::details::get_bitmap_size_id(32), Equals(0_sz));
  AssertThat(mcppalloc::bitmap_allocator::details::get_bitmap_size_id(33), Equals(1_sz));
  AssertThat(mcppalloc::bitmap_allocator::details::get_bitmap_size_id(512), Equals(4_sz));
  AssertThat(mcppalloc::bitmap_allocator::details::get_bitmap_size_id(513), Equals(::std::numeric_limits<size_t>::max()));
}

void exhaustive_test()
{
  using allocator_type =
      ::mcppalloc::bitmap_allocator::bitmap_allocator_t<::mcppalloc::default_allocator_policy_t<::std::allocator<void>>>;
  allocator_type bitmap_allocator{};
  bitmap_allocator.initialize(2000000, 2000000);
  bitmap_allocator.add_type(::mcppalloc::bitmap_allocator::details::bitmap_type_info_t(0, 0));
  auto &poa = bitmap_allocator;

  auto &ta = poa.initialize_thread();
  ::std::vector<void *> ptrs;
  const size_t allocation_size = 128;
  void *v;
  do {
    v = nullptr;
    try {
      auto allocation = ta.allocate(allocation_size);
      v = allocation.m_ptr;
      void *v_end = reinterpret_cast<uint8_t *>(v) + allocation.m_size;
      mcpputil::secure_zero(v, allocation.m_size);
      const auto state = ::mcppalloc::bitmap_allocator::details::get_state(v);
      if (mcpputil_unlikely(state == v)) {
        ::std::cerr << "Consistency error in bitmap exhaustive_test 5c9af962-57e6-4bf1-a843-12a239343c27\n";
        ::std::abort();
      }
      if (mcpputil_unlikely(v_end > state->end())) {
        ::std::cerr << "Consistency error in bitmap exhaustive_test 7caa9401-26f1-4cdc-9849-837b2767fa1e\n";
        ::std::abort();
      }
      if (!ptrs.empty()) {
        void *const prev = ptrs.back();
        void *const prev_end = reinterpret_cast<uint8_t *>(prev) + allocation_size;
        void *const prev_state = ::mcppalloc::bitmap_allocator::details::get_state(prev);
        if (state != prev_state && mcpputil_unlikely(prev_end >= state)) {
          ::std::cerr << prev << " " << state << " " << prev_end << "\n";
          ::std::cerr << "Consistency error in bitmap_exhaustive_test ad5058b9-85b0-461e-8c91-aca93faf4beb\n";
          ::std::abort();
        }
      }
      ptrs.push_back(v);
    } catch (::std::bad_alloc &) {
      // Do nothing
    }
  } while (v != nullptr);
  for (auto &&ptr : ptrs) {
    ta.deallocate(ptr);
  }
}

void bitmap_allocator_tests()
{
  auto manager = ::std::make_unique<mcpputil::thread_id_manager_t>();
  manager->set_max_threads(10);
  manager->set_max_tls_pointers(10);
  manager.release();
  mcpputil::get_thread_id_manager().add_current_thread();
  describe("bitmap_tests", []() {
    it("bitmap_state_test0", []() { bitmap_state_test0(); });
    it("multiple_slab_test0", []() { multiple_slab_test0(); });
    it("multiple_slab_test0b", []() { multiple_slab_test0b(); });
    it("multiple_slab_test1", []() { multiple_slab_test1(); });
    it("exhaustive_test", []() { exhaustive_test(); });
  });
}
