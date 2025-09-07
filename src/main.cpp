#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include <iostream>
#include <fstream>
#include <cstdlib> 
#include <string>
#include <format>
#include <optional>

#include <utils.h>

using json = nlohmann::json;


std::optional<json> makeRequest(httplib::Client& client, httplib::Headers& headers, const std::string& endpoint, bool printJson) {
    auto res = client.Get(endpoint, headers);
    if (res && res->status == 200) {
        json j = json::parse(res->body);
        if (printJson) {
            std::cout << j.dump(2) << '\n';
        }
        return j;
    }
    std::cout << "failure with status code: " << res->status;
    return {};
}

int main(int, char**){
    plog::init(plog::debug, "Logfile.txt");
    PLOGD << "main called";

    httplib::Client cli("https://www.strava.com");
    std::string accessToken {getValidAccessToken(cli)};

    httplib::Headers headers = {
        {"Authorization", accessToken}
    };

    std::ifstream inFile("strava_tokens.json");
    if (inFile) {
        json j;
        inFile >> j;
        std::string id {std::to_string(j["athlete"]["id"].get<long>())};
        std::string endpoint = {std::format("/api/v3/athletes/{}/stats", id)};
        makeRequest(cli, headers, endpoint, true);
    }
}
