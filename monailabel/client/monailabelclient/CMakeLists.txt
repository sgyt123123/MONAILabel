aux_source_directory(. DIR_LIB_SRCS)
add_library(monaiclient SHARED ${DIR_LIB_SRCS})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
target_link_libraries(monaiclient PRIVATE Qt5::Core Qt5::Widgets)

add_library(monaiclient_static STATIC ${DIR_LIB_SRCS})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
target_link_libraries(monaiclient_static PRIVATE Qt5::Core Qt5::Widgets)


