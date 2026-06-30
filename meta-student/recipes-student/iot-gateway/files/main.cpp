#include "sender.hpp"
#include <iostream>

int main() {
    std::cout << "--- Linux Embedded IoT Gateway Initializing ---\n";
    double mock_sensor_temp = 38.4;
    
    // Simulating sending data to a local edge dashboard server
    send_embedded_telemetry("http://192.168.1.50", 8080, mock_sensor_temp);
    
    return 0;
}
