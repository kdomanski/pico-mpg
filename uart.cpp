#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"

#include "uart.hpp"

#define UART_ID uart1
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define UART_TX_PIN 4
#define UART_RX_PIN 5

std::vector<std::string> uart_rx_lines;
auto_init_mutex(my_mutex);

void on_uart_rx() {
    static std::vector<std::string> temp_queue;
    static std::string uart_rx_buffer;

    bool busy = mutex_try_enter(&my_mutex, NULL);

    if (!busy) {
        if (!temp_queue.empty()) {
            std::copy(temp_queue.begin(), temp_queue.end(),
                      std::back_inserter(uart_rx_lines));
            temp_queue.clear();
        }
    }

    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        uart_rx_buffer.push_back(ch);

        if (ch == '\n') {
            if (busy)
                temp_queue.push_back(uart_rx_buffer);
            else
                uart_rx_lines.push_back(uart_rx_buffer);

            uart_rx_buffer.clear();
        }
    }

    if (busy)
        mutex_exit(&my_mutex);
}

void init_grbl_uart() {
    uart_init(UART_ID, 2400);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_baudrate(UART_ID, BAUD_RATE);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, true);

    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // enable the UART interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);
}

std::vector<std::string> get_received_lines() {
    std::vector<std::string> result;

    while (mutex_try_enter(&my_mutex, NULL))
        tight_loop_contents();

    if (!uart_rx_lines.empty()) {
        result.assign(uart_rx_lines.begin(), uart_rx_lines.end());
        uart_rx_lines.clear();
    }

    mutex_exit(&my_mutex);

    return result;
}
