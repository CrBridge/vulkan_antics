cmake_minimum_required (VERSION 3.8)
project ("vulkan_antics")

set(CMAKE_CXX_STANDARD 17)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/libs)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")

set(VULKAN_SDK ${EXTERNAL_DIR}/VulkanSDK)
set(CMAKE_PREFIX_PATH ${VULKAN_SDK})
find_package(Vulkan REQUIRED)

include_directories(${EXTERNAL_DIR}/glfw/include)
set(GLFW_LIB ${EXTERNAL_DIR}/glfw/lib-vc2022/glfw3.lib)

include_directories(${EXTERNAL_DIR}/glm)

include_directories(${EXTERNAL_DIR})

add_executable (vulkan_antics "main.cpp")
target_link_libraries(vulkan_antics PRIVATE Vulkan::Vulkan ${GLFW_LIB})