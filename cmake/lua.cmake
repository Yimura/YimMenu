include(FetchContent)

message("LuaJIT")

set(LUAJIT_BUILD_TOOL OFF)

FetchContent_Declare(
    luajit
    GIT_REPOSITORY https://github.com/WohlSoft/LuaJIT.git
    GIT_TAG 27878965958986687f8caaf8a2b6b73bc5df85e0
    GIT_PROGRESS TRUE
)

add_compile_definitions("_CRT_SECURE_NO_WARNINGS")
FetchContent_MakeAvailable(luajit)
