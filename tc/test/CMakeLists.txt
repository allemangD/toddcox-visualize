add_executable(grouptests grouptest.cpp)
target_link_libraries(grouptests PUBLIC tc)

add_test(NAME GroupIterateRef COMMAND grouptests iterate_ref)
set_tests_properties(GroupIterateRef PROPERTIES TIMEOUT 30)
add_test(NAME GroupIterateConst COMMAND grouptests iterate_const)
set_tests_properties(GroupIterateConst PROPERTIES TIMEOUT 30)
add_test(NAME GroupIterate COMMAND grouptests iterate)
set_tests_properties(GroupIterate PROPERTIES TIMEOUT 30)
add_test(NAME GroupView COMMAND grouptests view)
set_tests_properties(GroupView PROPERTIES TIMEOUT 30)

add_custom_target(alltests DEPENDS grouptests)
