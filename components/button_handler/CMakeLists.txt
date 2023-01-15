idf_component_register(SRCS "src/button_handler_shared.c" "src/button_pad_handler.c" "src/gpio_example_handler.c"
                            "cmd/cmd_button_handler.c"
                    INCLUDE_DIRS . "./inc/" "./cmd/"
                    PRIV_REQUIRES driver periph console ssh
                    REQUIRES )
