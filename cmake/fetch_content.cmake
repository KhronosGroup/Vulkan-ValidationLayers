include(FetchContent)

macro(FetchRepo name url tag)
    set(argn_ ${ARGN})
    list(LENGTH argn_ num_extra_args_)
    set(need_fetch_ FALSE)
    if (num_extra_args_ GREATER 0)
        foreach (t ${argn_})
            find_package(${t})
            if (NOT ${t}_FOUND)
                set(need_fetch_ TRUE)
                break()
            endif()
        endforeach()
    endif()

    if (need_fetch_)
        message("Fetching ${name} @ ${tag} from ${url}")
        FetchContent_Declare(
            ${name}
            GIT_REPOSITORY ${url}
            GIT_TAG ${tag}
        )
        FetchContent_MakeAvailable(${name})
    endif()
endmacro()
