# Copyright (c) 2022 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(oled_ssd1351 INTERFACE)

target_include_directories(oled_ssd1351 
    INTERFACE 
        .
        font
)

target_sources(oled_ssd1351 
    INTERFACE 
        oled_ssd1351.cpp
        opensans_font.c
)