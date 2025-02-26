cmake_minimum_required(VERSION 3.16)

function(get_migration_index FILE_NAME OUT_INDEX)
    string(REGEX MATCH "^([0-9]{3})\\.sql$" MATCHED ${FILE_NAME})
    if (MATCHED)
        set(${OUT_INDEX} ${CMAKE_MATCH_1} PARENT_SCOPE)
    else()
        set(${OUT_INDEX} -1 PARENT_SCOPE)
    endif()
endfunction()

function(collect_migrations OUT_FILES)
    file(GLOB SORTED_FILES "${MIGRATIONS_SRC_DIR}/*.sql")
    set(${OUT_FILES} ${SORTED_FILES} PARENT_SCOPE)
endfunction()

function(validate_migrations MIGRATION_FILES OUT_NUM_MIGRATIONS)
    set(EXPECTED_IDX 0)

    foreach(FILE ${MIGRATION_FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        get_migration_index(${FILENAME} MIGRATION_INDEX)

        math(EXPR PADDED_IDX "1000 + ${EXPECTED_IDX}")
        string(SUBSTRING ${PADDED_IDX} 1 -1 EXPECTED_FILE_NAME)
        set(EXPECTED_FILE_NAME "${EXPECTED_FILE_NAME}.sql")

        if (NOT "${FILENAME}" STREQUAL "${EXPECTED_FILE_NAME}")
            message(FATAL_ERROR "[bot][migrations] Отсутствует миграция ${EXPECTED_FILE_NAME}")
        endif()

        math(EXPR EXPECTED_IDX "${EXPECTED_IDX} + 1")
    endforeach()

    set(${OUT_NUM_MIGRATIONS} ${EXPECTED_IDX} PARENT_SCOPE)
endfunction()

function(generate_migration_header FILE_PATH FILE_INDEX)
    set(HEADER_PATH "${MIGRATIONS_BUILD_DIR}/${FILE_INDEX}.hpp")

    file(READ ${FILE_PATH} MIGRATION_CONTENT)
    file(WRITE ${HEADER_PATH}
        "#pragma once\n\n"
        "#include <string_view>\n\n"
        "namespace bot::sql {\n\n"
        "inline const std::string migration_${FILE_INDEX} = R\"(\nBEGIN TRANSACTION;\n\n${MIGRATION_CONTENT}\nCOMMIT;\n)\";\n\n"
        "}\n"
    )
endfunction()

function(generate_all_hpp MIGRATION_FILES)
    set(ALL_HEADERS "")

    foreach(FILE ${MIGRATION_FILES})
        get_filename_component(FILENAME ${FILE} NAME_WE)
        set(ALL_HEADERS "${ALL_HEADERS}#include \"${FILENAME}.hpp\"\n")
    endforeach()

    set(ALL_HPP "${MIGRATIONS_BUILD_DIR}/all.hpp")
    file(WRITE ${ALL_HPP}
        "#pragma once\n\n"
        "${ALL_HEADERS}"
    )
endfunction()

function(generate_migrations_all_hpp NUM_MIGRATIONS)
    if (NUM_MIGRATIONS EQUAL 0)
        message(FATAL_ERROR "[bot][migrations] Нет миграций для генерации migrations_all.hpp")
    endif()

    set(ARRAY_ENTRIES "")

    math(EXPR LAST_INDEX "${NUM_MIGRATIONS} - 1")

    foreach(INDEX RANGE 0 ${LAST_INDEX})
        math(EXPR PADDED_IDX "1000 + ${INDEX}")
        string(SUBSTRING ${PADDED_IDX} 1 -1 INDEX_STR)

        if (INDEX EQUAL LAST_INDEX)
            set(ARRAY_ENTRIES "${ARRAY_ENTRIES}    migration_${INDEX_STR},")
        else()
            set(ARRAY_ENTRIES "${ARRAY_ENTRIES}    migration_${INDEX_STR},\n")
        endif()
    endforeach()

    set(ALL_ARRAY_HPP "${MIGRATIONS_BUILD_DIR}/migrations_all.hpp")
    file(WRITE ${ALL_ARRAY_HPP}
        "#pragma once\n\n"
        "#include <array>\n\n"
        "#include \"migrations/all.hpp\"\n\n"
        "namespace bot::sql {\n\n"
        "inline const std::array<std::string, ${NUM_MIGRATIONS}> migrations = {\n"
        "${ARRAY_ENTRIES}\n"
        "};\n\n"
        "}\n"
    )
endfunction()

function(generate_all_migration_files)
    if (NOT DEFINED MIGRATIONS_SRC_DIR)
        message(FATAL_ERROR "[bot][migrations] MIGRATIONS_SRC_DIR не определён")
    endif()

    if (NOT DEFINED MIGRATIONS_BUILD_DIR)
        message(FATAL_ERROR "[bot][migrations] MIGRATIONS_BUILD_DIR не определён")
    endif()

    if (NOT EXISTS ${MIGRATIONS_SRC_DIR})
        message(FATAL_ERROR "[bot][migrations] Директория ${MIGRATIONS_SRC_DIR} не существует")
    endif()

    file(MAKE_DIRECTORY ${MIGRATIONS_BUILD_DIR})

    collect_migrations(SORTED_MIGRATION_FILES)

    if (SORTED_MIGRATION_FILES)
        validate_migrations("${SORTED_MIGRATION_FILES}" NUM_MIGRATIONS)

        foreach(FILE ${SORTED_MIGRATION_FILES})
            get_filename_component(FILENAME ${FILE} NAME_WE)
            generate_migration_header(${FILE} ${FILENAME})
        endforeach()

        generate_all_hpp("${SORTED_MIGRATION_FILES}")
        generate_migrations_all_hpp(${NUM_MIGRATIONS})
        message (STATUS "[bot][migrations] Сгенерированы файлы миграций")
    else()
        message(FATAL_ERROR "[bot][migrations] В папке ${MIGRATIONS_SRC_DIR} нет файлов")
    endif()
endfunction()
