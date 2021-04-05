#pragma once

#include <string>
#include <vector>

void init_grbl_uart();
std::vector<std::string> get_received_lines();
