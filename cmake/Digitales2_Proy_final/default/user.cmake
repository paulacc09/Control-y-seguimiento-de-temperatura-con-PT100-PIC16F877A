# Excluir lcd.c del build: driver LCD paralelo legado (PORTB); el firmware usa I2C_LCD.c.
# lcd.c permanece en el repositorio pero no se enlaza.
set(_lcd_legacy "${CMAKE_CURRENT_LIST_DIR}/../../../lcd.c")
get_filename_component(_lcd_legacy "${_lcd_legacy}" ABSOLUTE)
set(_lcd_legacy_gen "${CMAKE_CURRENT_LIST_DIR}/.generated/../../../lcd.c")
get_filename_component(_lcd_legacy_gen "${_lcd_legacy_gen}" ABSOLUTE)

list(REMOVE_ITEM Digitales2_Proy_final_default_default_XC8_FILE_TYPE_compile "${CMAKE_CURRENT_LIST_DIR}/../../../lcd.c")
list(REMOVE_ITEM Digitales2_Proy_final_default_default_XC8_FILE_TYPE_compile "${_lcd_legacy}")
list(REMOVE_ITEM Digitales2_Proy_final_default_default_XC8_FILE_TYPE_compile "${_lcd_legacy_gen}")

if(TARGET Digitales2_Proy_final_default_default_XC8_compile)
    target_sources(Digitales2_Proy_final_default_default_XC8_compile PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../../lcd.c" REMOVE)
    target_sources(Digitales2_Proy_final_default_default_XC8_compile PRIVATE "${_lcd_legacy}" REMOVE)
    target_sources(Digitales2_Proy_final_default_default_XC8_compile PRIVATE "${_lcd_legacy_gen}" REMOVE)
endif()
