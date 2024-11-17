idf_build_get_property(target IDF_TARGET)


idf_component_register(SRCS main.c my_tcp.c my_functions.c bme280.c
                                INCLUDE_DIRS "."
                                REQUIRES ${requires})
