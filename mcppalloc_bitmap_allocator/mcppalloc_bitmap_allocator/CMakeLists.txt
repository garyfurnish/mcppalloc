include_directories(include)
include_directories(../../mcppalloc/mcppalloc/include)
include_directories(../../mcppalloc_bitmap/mcppalloc_bitmap/include)
include_directories(../../mcppalloc_slab_allocator/mcppalloc_slab_allocator/include)
list (APPEND SRC_FILES
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_allocator.hpp   
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_package.hpp    
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_state_impl.hpp    
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_type_info.hpp
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_allocator_impl.hpp   
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_package_impl.hpp 
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_thread_allocator.hpp    
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_type_info_impl.hpp
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_allocator_stats.hpp 
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_state.hpp        
  include/mcppalloc/mcppalloc_bitmap_allocator/bitmap_thread_allocator_impl.hpp 
  include/mcppalloc/mcppalloc_bitmap_allocator/declarations.hpp
  src/bitmap_allocator_stats.cpp
  src/bitmap_state.cpp
  src/bitmap_type_info.cpp
  src/bitmap_package.cpp
  src/bitmap_thread_allocator.cpp
  src/bitmap_allocator.cpp
  src/main.cpp
)
add_library(mcppalloc_bitmap_allocator ${SRC_FILES})
INSTALL(DIRECTORY "include/mcppalloc" DESTINATION "include")
INSTALL(TARGETS mcppalloc_bitmap_allocator
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)
