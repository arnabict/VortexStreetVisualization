# Get the name of the folder
get_filename_component(FolderName ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" FolderName ${FolderName})

# Find header files
file(GLOB HPPFILES *.hpp)

# Find source files
file(GLOB CPPFILES *.cpp)

# Add the include path
include_directories(${CMAKE_HOME_DIRECTORY}/include ${EIGEN3_INCLUDE_DIR} ${EXTERN_INCLUDE_DIRS})

# Create static library
add_library(vispro_${FolderName} STATIC ${HPPFILES} ${CPPFILES})
target_link_libraries(vispro_${FolderName} ${VTK_LIBRARIES} eigen3::eigen)
vtk_module_autoinit(TARGETS vispro_${FolderName} MODULES ${VTK_LIBRARIES})