idf_component_register(SRCS "src/button_handler.c"
                            "cmd/cmd_button_handler.c"
                    INCLUDE_DIRS . "./inc/" "./cmd/"
                    PRIV_REQUIRES driver periph console ssh
                    REQUIRES )
