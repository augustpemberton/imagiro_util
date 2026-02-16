add_library(imagiro_util STATIC)

# important: copy compile definitions from the main project
target_compile_definitions(imagiro_util PUBLIC
        $<TARGET_PROPERTY:${ProjectName},COMPILE_DEFINITIONS>
)

target_sources(imagiro_util PRIVATE
        "include/imagiro_util/util.cpp"
        "include/imagiro_util/filewatcher/gin_filewatcher.mm"
        "include/imagiro_util/miniz/miniz.cpp"
)

# Link CoreServices on macOS for FSEvents file watching
if(APPLE)
    find_library(CORE_SERVICES_LIBRARY CoreServices)
    target_link_libraries(imagiro_util PUBLIC ${CORE_SERVICES_LIBRARY})
endif()

target_include_directories(imagiro_util PUBLIC
        include
)

target_link_libraries(imagiro_util PUBLIC
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_dsp
        nlohmann_json::nlohmann_json
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(imagiro_util PRIVATE
            -Wno-c99-extensions
            -Wno-gnu-statement-expression
            -Wno-shadow-field-in-constructor
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(imagiro_util PRIVATE
            -Wno-pedantic
    )
endif()