option(WITH_EXAMPLES "Build with examples" OFF)
option(WITH_SHARED_LIB "Build with a shared library" OFF)
option(WITH_STATIC_LIB "Build with a static library" ON)
option(WITH_CMOCKERY_SUPPORT "Install a cmockery header" OFF)
option(UNIT_TESTING "Build with unit testing" OFF)

if (WITH_EXAMPLES)
    set(WITH_SHARED_LIB)
endif()

if (UNIT_TESTING)
    set(WITH_STATIC_LIB ON)
endif()
