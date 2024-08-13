include(FetchContent)

FetchContent_Declare(
    gtav_classes
    GIT_REPOSITORY https://github.com/gir489returns/GTAV-Classes.git
    GIT_TAG        d700a082cbca28e2ca8a0251e3663c582a24b276
    GIT_PROGRESS TRUE
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
message("GTAV-Classes")
if(NOT gtav_classes_POPULATED)
    FetchContent_Populate(gtav_classes)
endif()
