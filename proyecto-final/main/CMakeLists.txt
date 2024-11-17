idf_component_register(SRCS "my_tcp.c" "my_functions.c" "bme280.c" "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES config.html info.html)
