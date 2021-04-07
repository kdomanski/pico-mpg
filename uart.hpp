#pragma once

#include <string>
#include <vector>

void init_grbl_uart();
void grbl_uart_send(const std::string &s);
std::vector<std::string> get_received_lines();
