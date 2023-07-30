idf_component_register(SRCS "main.c" "wifi_event.c" "input.c" "display_helper.c" "msg_handler.c"
                    INCLUDE_DIRS ".")

set_source_files_properties("main.c" "wifi_event.c" "input.c" "display_helper.c" "msg_handler.c"
    PROPERTIES COMPILE_FLAGS
    --coverage)