#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::string readEnvFile(std::string_view key) {
    std::ifstream inFile(".env");
    if (!inFile) {
        PLOGD << "unable to open env file";
        exit(EXIT_FAILURE);
    }

    std::string line;
    std::string out {};
    while (std::getline(inFile, line)) {
        if (line.empty() || line.find(key) == std::string::npos) continue;
        out = line.substr(key.length()+1, line.length()-1);
        PLOGD << "found target value" << out;
    }

    inFile.close();
    return out;
}