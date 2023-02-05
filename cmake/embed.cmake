find_program(EMBED_LD ${CMAKE_LINKER})
find_program(EMBED_OBJCOPY ${CMAKE_OBJCOPY})

function(_generate_embed_source EMBED_NAME)
  set(options)
  set(oneValueArgs SOURCE HEADER)
  set(multiValueArgs FILES)
  cmake_parse_arguments(PARSE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(VIEW_DECLARATIONS)
  set(VOID_DECLARATIONS)
  set(DEFINITIONS)

  foreach (FILE IN LISTS PARSE_FILES)
    get_filename_component(FILE_NAME "${FILE}" NAME)

    string(MAKE_C_IDENTIFIER "_binary_${FILE}" SYMBOL)
    string(MAKE_C_IDENTIFIER "${FILE_NAME}" IDENTIFIER)

    string(APPEND VIEW_DECLARATIONS
      "  /// ${FILE} Text Contents\n"
      "  extern std::string_view const ${IDENTIFIER};\n")

    string(APPEND VOID_DECLARATIONS
      "  /// ${FILE} Binary Contents\n"
      "  extern void* const ${IDENTIFIER};\n")

    string(APPEND DEFINITIONS
      "// ${IDENTIFIER} (${FILE})\n"
      "extern \"C\" const char ${SYMBOL}_start[], ${SYMBOL}_end[];\n"
      "std::string_view const ${EMBED_NAME}::${IDENTIFIER}(${SYMBOL}_start, ${SYMBOL}_end);\n"
      "void* const ${EMBED_NAME}::bin::${IDENTIFIER} = (void *) ${SYMBOL}_start;\n\n")
  endforeach ()

  file(WRITE "${PARSE_HEADER}"
    "#pragma once\n"
    "#include <string>\n\n"
    "namespace ${EMBED_NAME} {\n${VIEW_DECLARATIONS}}\n\n"
    "namespace ${EMBED_NAME}::bin {\n${VOID_DECLARATIONS}}\n")

  file(WRITE "${PARSE_SOURCE}"
    "#include <${EMBED_NAME}.hpp>\n\n"
    "${DEFINITIONS}")
endfunction()

function(_embed_file OUTPUT_OBJECT FILE)
  set(OBJECT "${CMAKE_CURRENT_BINARY_DIR}/${FILE}.o")

  set(${OUTPUT_OBJECT} ${OBJECT} PARENT_SCOPE)

  add_custom_command(
    COMMENT "Embedding ${FILE} in ${OBJECT}"
    OUTPUT "${FILE}.o" DEPENDS "${FILE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${EMBED_LD} -r -o "${OBJECT}" --format=binary "${FILE}"
    COMMAND ${EMBED_OBJCOPY} --rename-section .data=.rodata,alloc,load,readonly,data,contents "${OBJECT}"
    VERBATIM
  )
endfunction()

function(add_embed_library EMBED_NAME)
  set(FILES ${ARGN})

  set(EMBED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/_embed/${EMBED_NAME})
  set(EMBED_SOURCE "${EMBED_ROOT}/${EMBED_NAME}.cpp")
  set(EMBED_INCLUDE "${EMBED_ROOT}/include")
  set(EMBED_HEADER "${EMBED_INCLUDE}/${EMBED_NAME}.hpp")

  set(OBJECTS)
  foreach (FILE ${ARGN})
    _embed_file(OBJECT ${FILE})
    list(APPEND OBJECTS ${OBJECT})
  endforeach ()

  message(STATUS "Generating embedding library ${EMBED_NAME}")
  _generate_embed_source(
    ${EMBED_NAME}
    SOURCE ${EMBED_SOURCE}
    HEADER ${EMBED_HEADER}
    FILES ${FILES})

  add_library(${EMBED_NAME} STATIC ${OBJECTS} "${EMBED_SOURCE}")
  target_include_directories(${EMBED_NAME} PUBLIC "${EMBED_INCLUDE}")
  set_target_properties(${EMBED_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()
