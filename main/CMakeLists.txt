idf_component_register(SRCS "main.c"
                            "../steering_wheel/src/steering_wheel.c"
                       INCLUDE_DIRS "."
                                    "../steering_wheel/include"
                       REQUIRES hw_adc)
