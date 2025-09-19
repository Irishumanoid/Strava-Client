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

/** Gets distinct routes from json of all user activities */
std::optional<json> getRoutes(const std::string& path) {
    std::ifstream inFile(path);
    PLOGD << "getRoutes called";
    if (inFile) {
        json j;
        inFile >> j;
        inFile.close();
        PLOGD << "test";

    if (j["data"].is_array()) {
        PLOGD << "read file";
        std::map<std::string, json> sportRoutes;

        for (const auto& activity : j["data"]) {
            if (activity.contains("map") && activity["map"].contains("summary_polyline")) {
                std::string sport = activity["sport_type"];
                std::string polyline = activity["map"]["summary_polyline"];
                PLOGD << "found activity of type: " << sport;

                if (!sportRoutes.contains(sport)) {
                    sportRoutes[sport] = json::array();
                }

                bool matched = false;
                for (auto& routeObj : sportRoutes[sport]) {
                    std::string otherPolyline = routeObj["polyline"];
                    if (RouteUtils::areRoutesSame(polyline, otherPolyline)) {
                        PLOGD << "same routes found";
                        routeObj["ids"].push_back(activity["id"]);
                        matched = true;
                        break; // no other matches will exist, since all same polylines will have ids in the same sub-json
                    }
                }

                if (!matched) {
                    PLOGD << "distinct routes found";
                    sportRoutes[sport].push_back({
                        {"ids", json::array({activity["id"]})},
                        {"polyline", polyline}
                    });
                }
            } else {
                PLOGD << "some json fields missing for activity: " << activity;
            }
        }

        json out;
        for (const auto& [sport, data] : sportRoutes) {
            out[sport] = data;
        }
        return out;
    }

    }
    return {};
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
    
    
    /*std::cout << std::boolalpha;
    std::string p1 {"yttaHvoviV@aDCkDBYIkDGUMKiBDQEEcD@uAF{ACsBEQQYWK[AqDDKSGEs@E}@F_@GSDkDxBo@j@aA^o@Lw@H[H[?_@MU?aCDqCG{A@u@Hc@OUO{EmAqAOiBAqE@gBBeBCuABWHc@DuAZaBt@m@NmAh@eCvASPSLUFgDhBIFe@f@kAn@k@LiBx@gAn@iGhFw@x@Y`@[p@o@nBCXFl@AH}BdIMr@m@tAAN_@jAm@jAq@|@]X_@P[@qAAs@Dq@GkCB_EAUAKEGKE]?_CjA_PZ}C\\}Ej@}FCQJ_@BBACOF_@Dk@A}@@@?ACKE}FGs@@yAEcGGw@Co@BiBCe@Co@?m@Ia@Cu@@cBA[E}@?aBEe@Bw@AW?c@Gc@@kBGcBGeCFoAQ}@?s@Cg@Em@@q@GaBDoAImACe@EQIeCqAiCkA}DmBsAq@A?D@_B}@a@MkBIgHCcF?gACoBYS?]CKZKHK?cAIoCCc@K]So@m@IMAYGCQg@]eBSwAU{@Sg@m@sAyMuPcC{EkAwAs@k@]OoGwAiAs@s@oAYaAGc@ImADcEPyF?_ACw@OoBa@_Bw@sAkAgAu@UMKCKBMX]x@e@r@g@`DuCxAiAlCcCjDwCl@m@HOBYGmEEe@Ak@Dw@FmCK_GMiANw@BoBGiBGsAKw@EiABgBH_BB_BJwB@sAB]AgE?cAD_ACcFC}@Be@AyPDaBAmA?iHB_AHmAEa@CaAAgEIwD@WDYBk@BK?i@KmB"};
    std::string p2 {"mttaHnqviVEmB?cNE[KO_B?a@IOuEFyBG}B[[qECi@KUUyAR_@GiDrB}@v@aAd@uCb@}@YwIB}AL}@k@gEmAwAOyQH_Dx@}DxA{DdCg@NqDtBW\\eAr@s@NgDxA_IvG_AzA}@hCH`As@vBm@tCqBtGa@`AeBnBuARcOOQi@AsAHeBhCw[d@aEfAyLXuAx@aCjAiCbAyAHk@@qPH}@McADaDGg@k@kAg@[yAy@}OyHsHgLuAaBgEyDwAaBqAmBi@yAg@yBUqCkBaPMk@Q{BIcEJkBhAwHHQbA{It@aD`@oDbAaDt@_AZ}@LCTaBTo@fBoDtF}NpAqCj@iBZkBMwADcA[kBh@q@h@sBVe@[yBJw@VYXKvF|@jEDfAR|CB~@[r@CdD?hBHiA@_@OGUBqOVkAJcBKuI]gLKwJJeGVsHx@gQp@w]\\oMVgFt@gIxEca@`TmgCpCa^pAwNDaBDC`C{UFcCN}AJo@Vm@p@cF|BoTn@qHv@aLHqBC_KO{@s@uBCm@TgCAq@Mm@w@{CcBqE_DkF_EaGqB{D{@aCgCwJuAmHm@IEMEq@eCgPcAaEmBwJc@oC_@{EOmDEwDLmEn@sID{AOcGBsANiCQKHd@@j@MjC@zCJlBW`I_@hFCpCDzDLlCNzBd@nDpBrJdAvFlB|L`@hBv@nD^v@fDzLrAzDpB`EpI|MlArCfArD`@~BAj@UpB@j@x@vCFh@IrNkAtPkAlLy@xFa@xE]|BNr@l@l@w@q@U^SdAO`BKpCiB~OqBfUoGrx@sO`jBuAnMyCfVaAzLs@`Y]tSM`Eu@fMYlK?|Ln@jXEbCI~@Yx@YZA|NCJ[BoBCyAb@s@FgCE_BWaDAkC[mBc@[PYz@Bj@V|AgArCi@v@^bBAvAJhAGj@{AzEsI~S{@|A[|@]bBg@fAc@h@e@|@{@nDW|Ci@rCaAvHUp@ObAGdAe@lDKlDLvDlA|IlApLXvAx@zBnBhCfInIfIzLtCnA~NrHTX\\hAHn@IjAFlBK|ACtED~IGVsBrDw@hBm@vBSdAw@|IKfDYhBuAnQHXPJfAVALUDaACYTs@rGJ`BNh@LNlDErGNxBORM`BuB~@kCtDaNKs@FWp@kBl@mAzF{ErCsBtBaAnAUbBoAtDsBtEqCxEcBzBUzKQjCBvC^jD~@|@h@fBQtKPjAKxAc@lG{DrBGd@v@b@JhECLLBXB~AKfCAvFRPxB@ZRBzAEdH"};
    std::cout << RouteUtils::areRoutesSame(p1, p2, true);*/
    json out = getRoutes("json_data/activity_data_iris.json");
    std::ofstream outFile ("test_distinct_routes.json");
    outFile << out.dump(2);
    outFile.close();
}
