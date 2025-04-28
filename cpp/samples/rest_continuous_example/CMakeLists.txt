set(EXECUTABLE_NAME rest_continuous_example)

add_executable(${EXECUTABLE_NAME} main.cc)

target_link_libraries(${EXECUTABLE_NAME}
        SmartSpectra::Container
        SmartSpectra::Gui
)

if (INSTALL_SAMPLES)
    install(TARGETS ${EXECUTABLE_NAME}
            EXPORT ${PROJECT_NAME}Targets
            FILE_SET HEADERS
    )
endif ()
