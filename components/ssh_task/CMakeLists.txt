idf_component_register(SRCS "./src/ssh_task.c" "./src/ssh_driver.c"
                    INCLUDE_DIRS . "./inc/" "./cmd/"
                    PRIV_REQUIRES driver periph console libssh2
                    REQUIRES )
