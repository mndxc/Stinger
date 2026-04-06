idf_component_register(SRCS "main.c" "stinger_app.c"
                    INCLUDE_DIRS ".")

# Automatically generate pin_map.h from master_pins.json
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/pin_map.h
    COMMAND ${CMAKE_COMMAND} -E env python ${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate_pins.py
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../master_pins.json
    COMMENT "Generating pin_map.h and updating SPEC.md from master_pins.json"
)

# Force the generation target to run before the main component compiles
add_custom_target(generate_pins ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pin_map.h)
add_dependencies(${COMPONENT_LIB} generate_pins)
