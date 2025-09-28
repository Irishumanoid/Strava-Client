#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib> 
#include <string>
#include <format>
#include <optional>
#include <map>

#include <utils.h>
#include <route_analysis/route_utils.h>

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


void callEndpoint(httplib::Client& client, httplib::Headers& headers, const std::string& endpoint, const std::string& outPath, bool verbose) {
    auto out = makeRequest(client, headers, endpoint, verbose);
    std::ofstream outFile(outPath);
    if (outFile.is_open()) {
        json j = *out;
        outFile << j.dump(2);
        outFile.close();
    }
}

void getAthlete(httplib::Client& client, httplib::Headers& headers, const std::string& outPath, bool verbose) {
    std::string endpoint = "/api/v3/athlete";
    callEndpoint(client, headers, endpoint, outPath, verbose);
}

void getAthleteStats(httplib::Client& client, httplib::Headers& headers, std::string_view athleteId, const std::string& outPath, bool verbose) {
    std::string endpoint = {std::format("/api/v3/athletes/{}/stats", athleteId)};
    callEndpoint(client, headers, endpoint, outPath, verbose);
}

void getAthleteActivities(httplib::Client& client, httplib::Headers& headers, int numPerPage=200, std::optional<int> pageLimit=std::nullopt) {
    int totalNumActivities {}, curPage {1};
    std::map<std::string, int> activities;

    int limit {};
    if (pageLimit) {
        limit = *pageLimit;
    }

    json activityData {json::array()};
    while (true) {
        if (limit == curPage) break;
        std::string endpoint {std::format("/api/v3/athlete/activities?per_page={}&page={}", numPerPage, curPage)};
        auto out = makeRequest(client, headers, endpoint, true);
        if (out) {
            json j = *out;
            if (j.empty()) break;

            ++curPage;
            totalNumActivities += j.size();
            if (j.is_array()) {
                for (const auto& activity : j) {
                    activityData.push_back(activity);
                    std::string type {activity["sport_type"].get<std::string>()};
                    ++activities[type];
                }
            }
        }
    }
    json summary;
    summary["sports"] = {};
    std::cout << "total num activities: " << totalNumActivities;
    for (const auto &[sport, count] : activities) {
        summary["sports"][sport] = count;
        std::cout << "sport: " << sport << " has activity count: " << count << '\n';
    }
    summary["data"] = activityData;
    std::ofstream outFile("activity_data.json");
    if (outFile.is_open()) {
        outFile << summary.dump(2);
        outFile.close();
    }
}


int main(int, char**){
    plog::init(plog::debug, "Logfile.txt");
    PLOGD << "main called";

    /*httplib::Client client("https://www.strava.com");
    std::string accessToken {getValidAccessToken(client)};

    httplib::Headers headers = {
        {"Authorization", accessToken}
    };
    
    getAthlete(client, headers, "json_data/athlete_data_iris.json", true);*/
    

    /*json out = RouteUtils::getRoutes("json_data/activity_data_iris.json");
    std::ofstream outFile ("test_distinct_routes.json");
    outFile << out.dump(2);
    outFile.close();*/
    auto routes = RouteUtils::getAvgRouteStats("activity_data_iris.json", "test_distinct_routes.json");
    if (routes) {
        json rJson = *routes;
        std::ofstream out {"avg_route_data_iris.json"};
        out << rJson;
    }
}
