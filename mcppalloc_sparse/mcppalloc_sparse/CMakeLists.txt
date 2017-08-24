include_directories(include)
include_directories(../../mcppalloc/mcppalloc/include)
list (APPEND SRC_FILES
include/mcppalloc/mcppalloc_sparse/allocator.hpp  
include/mcppalloc/mcppalloc_sparse/allocator_block_impl.hpp 
include/mcppalloc/mcppalloc_sparse/allocator_impl.hpp
include/mcppalloc/mcppalloc_sparse/functor.hpp
include/mcppalloc/mcppalloc_sparse/sparse_allocator_block_set_verifier.hpp
include/mcppalloc/mcppalloc_sparse/thread_allocator_abs_data.hpp
include/mcppalloc/mcppalloc_sparse/allocator_block.hpp
include/mcppalloc/mcppalloc_sparse/allocator_block_set.hpp
include/mcppalloc/mcppalloc_sparse/debug.hpp
include/mcppalloc/mcppalloc_sparse/mcppalloc_sparse.hpp
include/mcppalloc/mcppalloc_sparse/sparse_allocator_verifier.hpp
include/mcppalloc/mcppalloc_sparse/thread_allocator_abs_data_impl.hpp
include/mcppalloc/mcppalloc_sparse/allocator_block_handle.hpp
include/mcppalloc/mcppalloc_sparse/allocator_block_set_impl.hpp
include/mcppalloc/mcppalloc_sparse/declarations.hpp
include/mcppalloc/mcppalloc_sparse/sparse_allocator_block_base.hpp
include/mcppalloc/mcppalloc_sparse/thread_allocator.hpp
include/mcppalloc/mcppalloc_sparse/thread_allocator_impl.hpp
  src/allocator.cpp
  src/thread_allocator.cpp
  src/allocator_block_set.cpp
  src/allocator_block.cpp
  src/sparse_allocator_verifier.cpp
  src/sparse_allocator_block_set_verifier.cpp
)
add_library(mcppalloc_sparse_allocator ${SRC_FILES})
INSTALL(DIRECTORY "include/mcppalloc" DESTINATION "include")
INSTALL(TARGETS mcppalloc_sparse_allocator
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)
