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

            curPage++;
            totalNumActivities += j.size();
            if (j.is_array()) {
                for (const auto& activity : j) {
                    activityData.push_back(activity);
                    std::string type {activity["sport_type"].get<std::string>()};
                    activities[type]++;
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
    std::cout << std::boolalpha;
    std::string p1 {"yttaHvoviV@aDCkDBYIkDGUMKiBDQEEcD@uAF{ACsBEQQYWK[AqDDKSGEs@E}@F_@GSDkDxBo@j@aA^o@Lw@H[H[?_@MU?aCDqCG{A@u@Hc@OUO{EmAqAOiBAqE@gBBeBCuABWHc@DuAZaBt@m@NmAh@eCvASPSLUFgDhBIFe@f@kAn@k@LiBx@gAn@iGhFw@x@Y`@[p@o@nBCXFl@AH}BdIMr@m@tAAN_@jAm@jAq@|@]X_@P[@qAAs@Dq@GkCB_EAUAKEGKE]?_CjA_PZ}C\\}Ej@}FCQJ_@BBACOF_@Dk@A}@@@?ACKE}FGs@@yAEcGGw@Co@BiBCe@Co@?m@Ia@Cu@@cBA[E}@?aBEe@Bw@AW?c@Gc@@kBGcBGeCFoAQ}@?s@Cg@Em@@q@GaBDoAImACe@EQIeCqAiCkA}DmBsAq@A?D@_B}@a@MkBIgHCcF?gACoBYS?]CKZKHK?cAIoCCc@K]So@m@IMAYGCQg@]eBSwAU{@Sg@m@sAyMuPcC{EkAwAs@k@]OoGwAiAs@s@oAYaAGc@ImADcEPyF?_ACw@OoBa@_Bw@sAkAgAu@UMKCKBMX]x@e@r@g@`DuCxAiAlCcCjDwCl@m@HOBYGmEEe@Ak@Dw@FmCK_GMiANw@BoBGiBGsAKw@EiABgBH_BB_BJwB@sAB]AgE?cAD_ACcFC}@Be@AyPDaBAmA?iHB_AHmAEa@CaAAgEIwD@WDYBk@BK?i@KmB"};
    std::string p2 {"yvuaHxwviVODML_@A_@Fe@K]Aq@Kc@CwABUQe@SMCiAGQMSEu@k@EAc@JUCmA^e@BCBo@x@U`@WhAE\\a@t@O`@s@~@]D}@My@NUKYWIa@W_@[WYAOWEAKBeAFiAP}@HiAFe@?ODI`@CzCY`@UDq@Ms@Da@?{@BkA?kB@UD_@VQ`@C^BrBId@Ch@Dh@Nt@l@|@`@^`@d@Dp@@v@Fz@C|EEl@Wd@a@Te@\\[h@a@b@[l@u@fAa@f@Af@BH@dBCt@@|EAt@H~BRd@^`@hAv@Z`@|BtBTVXPDHCj@DLFr@G`CBpAEfAIZSV]n@m@vAaAdAWf@a@h@a@XQRc@p@]Rc@Cc@Io@Fi@?ME[E}@Du@RKz@Dd@AVDZEX@Z?xEAtAG`@@zAF`AKj@WLe@Hm@B{BDw@Ac@CKF?Ce@BgAAm@\\_@n@Kn@A~@ANFbB?|@Lv@Vd@b@N`BANHxAH\\C`@OPAVDh@@ZBnAG^@LFr@[VAr@Jd@Kd@Ah@BLCd@Hb@Bz@Kf@@nA?|AH\\JPJXAb@NZE\\B|@Lj@Av@D|@Gb@KDDLBPJVD\\C~BCn@E\\@d@FXEf@@h@C\\BXFj@G`@BnBGdCRh@I`CDb@Ed@?`@Gx@@v@Hd@ERSz@cA|BsBVa@\\_@d@UTQv@}@t@e@`AcALEn@?h@i@ZFHNvA_B`@s@Ay@NcALk@Js@b@u@JiBVu@Jg@d@g@LGL]Gi@J]B_AU}A?EKq@?QBe@Gq@@g@FUBQFEBWEUEi@@w@Cm@@QEWDaAEu@EoAKs@Hm@B_@Kq@FaCLY^_@h@Q^QHwALk@Ve@^Qb@KL?VXGW?[Fi@@}@BYOq@Fe@"};
    std::cout << RouteUtils::areRoutesSame(p1, p2, true);
}
