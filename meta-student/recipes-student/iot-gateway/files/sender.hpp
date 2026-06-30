#ifndef SENDER_HPP
#define SENDER_HPP
#include <string>

bool send_embedded_telemetry(const std::string& host, int port, double temperature);

#endif
