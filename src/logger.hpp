#pragma once

#include <string>
#include <fstream>

#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_GRAY   "\033[90m"

std::string get_timestamp_string();

void log_connect(std::ofstream& file, int conn_id);
void log_request(std::ofstream& file, int conn_id, const std::string& request);
void log_answer(std::ofstream& file, int conn_id, int status_code, const std::string& answer);
void log_disconnect(std::ofstream& file, int conn_id);
