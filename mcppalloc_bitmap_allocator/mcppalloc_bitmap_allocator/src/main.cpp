#include <mcppalloc/mcppalloc_bitmap_allocator/declarations.hpp>

#ifdef _WIN32
namespace mcppalloc::bitmap_allocator
{
  __declspec(dllexport) void force_import()
  {
  }
} // namespace mcppalloc::bitmap_allocator
#endif