# Get the name of the folder
get_filename_component(FolderName ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" FolderName ${FolderName})

# Add your project files
file(GLOB SRCFILES *.cpp)
file(GLOB HPPFILES *.hpp)

include_directories(${CMAKE_HOME_DIRECTORY}/common ${EIGEN3_INCLUDE_DIR} ${VTK_INCLUDE_DIR})

add_definitions(-DNOMINMAX)
add_executable(vispro_${FolderName} ${SRCFILES} ${HPPFILES})
target_link_libraries(vispro_${FolderName} ${VTK_LIBRARIES} vispro_common)
vtk_module_autoinit(TARGETS vispro_${FolderName} MODULES ${VTK_LIBRARIES})