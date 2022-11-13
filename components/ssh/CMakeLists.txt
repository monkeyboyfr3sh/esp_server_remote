idf_component_register(SRCS "ssh.c" "cmd_ssh.c"
                    INCLUDE_DIRS .
                    REQUIRES libssh2 console)
