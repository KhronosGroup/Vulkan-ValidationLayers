include(FetchContent)

macro(FetchRepo name url tag)
    set(argn_ ${ARGN})
    list(LENGTH argn_ num_extra_args_)
    if (num_extra_args_ EQUAL 2)
        list(GET argn_ 0 source_dir)
        list(GET argn_ 1 bin_dir)
        message("Using local directory ${source_dir} as source directory and ${bin_dir} as binary directory for ${name}")
        FetchContent_GetProperties(${name})
        string(TOLOWER "${name}" lc_name_)
        if (NOT ${lc_name_}_POPULATED)
            FetchContent_Populate(${name}
                SOURCE_DIR ${source_dir}
                BINARY_DIR ${bin_dir}
            )
            add_subdirectory(${source_dir} ${bin_dir})
        endif()
    else()
        message("Fetching ${name} @ ${tag} from ${url}")
        FetchContent_Declare(
            ${name}
            GIT_REPOSITORY ${url}
            GIT_TAG ${tag}
        )
        FetchContent_MakeAvailable(${name})
    endif()
endmacro()
