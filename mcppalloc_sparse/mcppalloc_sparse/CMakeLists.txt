include_directories(include)
include_directories(../../mcppalloc/mcppalloc/include)
list (APPEND SRC_FILES
  src/sparse_allocator_verifier.cpp
  src/sparse_allocator_block_set_verifier.cpp
)
add_library(mcppalloc_sparse_allocator ${SRC_FILES})
INSTALL(DIRECTORY "include/mcppalloc" DESTINATION "include")
INSTALL(TARGETS mcppalloc_sparse_allocator
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)
