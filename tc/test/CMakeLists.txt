include(GoogleTest)

add_executable(test_solve test_solve.cpp)
target_link_libraries(test_solve tc GTest::gtest_main)

add_executable(test_lang test_lang.cpp)
target_link_libraries(test_lang PUBLIC tc::tc GTest::gtest_main)

set(MIN_DEBUG_CPS 200000)
set(MIN_RELEASE_CPS 1000000)

target_compile_definitions(
    test_solve PUBLIC
    MIN_COS_PER_SEC=$<IF:$<CONFIG:Debug>,${MIN_DEBUG_CPS},${MIN_RELEASE_CPS}>
)

gtest_discover_tests(test_solve)
gtest_discover_tests(test_lang)
