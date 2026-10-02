add_executable(sciter_event_handler_tests event_handler_tests.cpp)
target_link_libraries(sciter_event_handler_tests PRIVATE sciterui ${CMAKE_DL_LIBS})
target_include_directories(sciter_event_handler_tests PRIVATE
    ../src/sciterui ../src/3rd_party/sciter-js-sdk-main/include)
add_test(NAME sciter_event_handler_lifetime COMMAND sciter_event_handler_tests)

if(APPLE)
    add_executable(sciter_window_lifetime_tests sciter_window_lifetime_tests.mm)
    target_link_libraries(sciter_window_lifetime_tests PRIVATE sciterui)
    target_include_directories(sciter_window_lifetime_tests PRIVATE
        ../src/sciterui ../src/3rd_party/sciter-js-sdk-main/include)
    add_test(NAME sciter_native_window_lifetime COMMAND sciter_window_lifetime_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/../src/3rd_party/sciter-js-sdk-main/bin/macosx/libsciter.dylib")
    set_tests_properties(sciter_native_window_lifetime PROPERTIES TIMEOUT 30 RUN_SERIAL TRUE)
endif()
