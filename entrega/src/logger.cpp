#include "logger.hpp"
#include <iomanip>
#include <ctime>
#include <iostream>
#include <sstream>

std::string get_timestamp_string() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    char timestamp[25];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(timestamp);
}

std::string sanitize_string(const std::string& message) {
    std::string sanitized = message;
    if (!sanitized.empty() && sanitized.back() == '\n')
        sanitized.erase(sanitized.length() - 1);
    return sanitized;
}

void log_to_file(std::ofstream& file, int conn_id, const std::string& type, const std::string& message) {
    std::ostringstream log_message;
    log_message << "[" << get_timestamp_string() << "]" 
                << "[#" << std::setfill('0') << std::setw(5) << conn_id << "]" 
                << "[" << type << "]"
                << " " << message << std::endl;

    file << log_message.str();
}

void log_to_stdout(int conn_id, const std::string& type, const std::string& message) {
    std::ostringstream log_message;
    if (type == "CONNECT")
        log_message << COLOR_GRAY
                    << "[#" << std::setfill('0')
                    << std::setw(5) << conn_id << "][Conexão aberta]\n"
                    << COLOR_RESET;

    if (type == "REQUEST")
        log_message << "[#" << std::setfill('0')
                    << std::setw(5)
                    << conn_id
                    << "][Cliente enviou:] "
                    << message
                    << std::endl;

    if (type == "DISCONNECT")
        log_message << COLOR_GRAY 
                    << "[#" 
                    << std::setfill('0') 
                    << std::setw(5) 
                    << conn_id 
                    << "][Conexão encerrada]\n" 
                    << COLOR_RESET;

    std::cout << log_message.str();
}

void log_connect(std::ofstream& file, int conn_id) {
    log_to_file(file, conn_id, "CONNECT", "");
    log_to_stdout(conn_id, "CONNECT", "");
}

void log_request(std::ofstream& file, int conn_id, const std::string& request) {
    std::string sanitized_request = sanitize_string(request);
    log_to_file(file, conn_id, "REQUEST", sanitized_request);
    log_to_stdout(conn_id, "REQUEST", sanitized_request);
}

void log_answer(std::ofstream& file, int conn_id, int status_code, const std::string& answer) {
    std::string log_answer = std::to_string(status_code) + " " + sanitize_string(answer);
    log_to_file(file, conn_id, "ANSWER", log_answer);
    // log_to_stdout(conn_id, "ANSWER", log_answer);
}

void log_disconnect(std::ofstream& file, int conn_id) {
    log_to_file(file, conn_id, "DISCONNECT", "");
    log_to_stdout(conn_id, "DISCONNECT", "");
}
