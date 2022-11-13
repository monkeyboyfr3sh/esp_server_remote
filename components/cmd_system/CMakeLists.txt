idf_component_register(SRCS "cmd_system.c" "cmd_ssh.c"
                    INCLUDE_DIRS .
                    REQUIRES console spi_flash ssh)
