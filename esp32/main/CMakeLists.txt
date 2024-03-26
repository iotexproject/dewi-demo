idf_component_register(SRCS "hd44780_I2C.c" "esp32_nat_router.c"
                            "http_server.c"
                    INCLUDE_DIRS ".")

set_source_files_properties(http_server.c
    PROPERTIES COMPILE_FLAGS
    -Wno-unused-function
)
