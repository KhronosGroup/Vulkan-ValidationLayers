function(common_compile_and_link_options TARGET)
	if(MSVC)
	    target_compile_options(${TARGET} PRIVATE /bigobj)
	elseif(MINGW)
	    target_compile_options(${TARGET} PRIVATE -Wa,-mbig-obj)
	endif()

	# Khronos validation additional dependencies
	if (USE_ROBIN_HOOD_HASHING)
	    target_link_libraries(${TARGET} PRIVATE robin_hood::robin_hood)
	endif()

	# Using mimalloc on non-Windows OSes currently results in unit test instability with some
	# OS version / driver combinations. On 32-bit systems, using mimalloc cause an increase in
	# the amount of virtual address space needed, which can also cause stability problems.
	if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
	   find_package(mimalloc CONFIG)
	   option(USE_MIMALLOC "Use mimalloc, a fast malloc/free replacement library" ${mimalloc_FOUND})
	   if (USE_MIMALLOC)
	      target_compile_definitions(${TARGET} PRIVATE USE_MIMALLOC)
	      target_link_libraries(${TARGET} PRIVATE mimalloc-static)
	   endif()
	endif()
endfunction()