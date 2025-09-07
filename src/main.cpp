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
#include <map>

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

void getAthleteStats(httplib::Client& client, httplib::Headers& headers) {
    std::ifstream inFile("strava_tokens.json");
    if (inFile) {
        json j;
        inFile >> j;
        std::string id {std::to_string(j["athlete"]["id"].get<long>())};
        std::string endpoint = {std::format("/api/v3/athletes/{}/stats", id)};
        makeRequest(client, headers, endpoint, true);
    }
}

//TODO write summary at top of json with total #activities and # for each sport_type; write to file
void athleteActivitiesToJson(httplib::Client& client, httplib::Headers& headers) {
    int totalNumActivities {}, curPage {1};
    int numPerPage {50};
    std::map<std::string, int> activities;

    while (true) {
        if (curPage == 3) break; //temp for debug to avoid looking at all activities

        std::string endpoint {std::format("/api/v3/athlete/activities?per_page={}&page={}", numPerPage, curPage)};
        auto out = makeRequest(client, headers, endpoint, true);
        if (out) {
            json j = *out;
            if (j.empty()) break;

            curPage++;
            totalNumActivities += j.size();
            if (j.is_array()) {
                for (const auto& activity : j) {
                    std::string type {activity["sport_type"].get<std::string>()};
                    activities[type]++;
                }
            }
        }
    }

    std::cout << "total num activities: " << totalNumActivities;
    for (const auto &[sport, count] : activities) {
        std::cout << "sport: " << sport << " has activity count: " << count;
    }
}

int main(int, char**){
    plog::init(plog::debug, "Logfile.txt");
    PLOGD << "main called";

    httplib::Client client("https://www.strava.com");
    std::string accessToken {getValidAccessToken(client)};

    httplib::Headers headers = {
        {"Authorization", accessToken}
    };
    athleteActivitiesToJson(client, headers);
}
