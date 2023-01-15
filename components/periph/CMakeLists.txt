idf_component_register(SRCS "src/button_pad.c" "src/periph.c" 
                            "cmd/cmd_periph.c"
                    INCLUDE_DIRS . "./inc/" "./cmd/"
                    PRIV_REQUIRES driver console
                    REQUIRES )
