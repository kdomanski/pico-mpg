/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"

#include <iostream>
#include <memory>

#include "display.hpp"
#include "uart.hpp"

int main() {
    stdio_init_all();

    init_grbl_uart();

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    auto display = std::make_unique<Display>();
    display->Fill(display->blue);
    display->Rect(4, 4, LCD_X - 8, LCD_Y - 8, display->white);
    display->Show();

    while (true) {
        std::vector<std::string> lines = get_received_lines();
        if (!lines.empty()) {
            for (std::string line : lines)
                std::cout << line;
            std::cout << std::flush;
        }

        tight_loop_contents();
    }
}
