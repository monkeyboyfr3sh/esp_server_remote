idf_component_register(SRCS "cmd_wifi.c" "terminal.c" "main.c" "connect.c"
                    INCLUDE_DIRS ".")

set_source_files_properties( "main.c"
    PROPERTIES COMPILE_FLAGS
    -g
)