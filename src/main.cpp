#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include <iostream>
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
            for (json::iterator it = j.begin(); it != j.end(); ++it) {
                std::cout << "key: " << it.key() << " value: " << it.value() << '\n'; 
            } 
        }
        return j;
    }
    std::cout << "failure with status code: " << res->status;
    return {};
}

//TODO OAuth to get full data
int main(int, char**){
    plog::init(plog::debug, "Logfile.txt");
    PLOGD << "main called";
    std::string s {readEnvFile("STRAVA_ACCESS_TOKEN")};

    std::string accessToken {};
    if (s != "") {
        accessToken = "Bearer " + static_cast<std::string>(s);
    } else {
        std::cerr << "Could not find access token";
        std::exit(EXIT_FAILURE);
    }

    httplib::Client cli("https://www.strava.com");
    httplib::Headers headers = {
        {"Authorization", accessToken}
    };

    json j = makeRequest(cli, headers, "/api/v3/athlete", true);
    /*
    std::string endpoint = {std::format("/api/v3/{}/stats", id)};
    makeRequest(cli, headers, endpoint, true);
    */
}
