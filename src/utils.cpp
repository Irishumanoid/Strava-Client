#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <chrono>
#include <optional>

#include "utils.h"

const std::string tokenFile = "strava_tokens.json";

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
        PLOGD << "found target value: " << out;
    }

    inFile.close();
    return out;
}

std::optional<json> readTokensFromJson() {
    std::ifstream inFile(tokenFile);
    if (!inFile) {
        PLOGD << "no tokens file";
        return {};
    }
    json j;
    inFile >> j;
    return j;
}

void saveTokens(json& j) {
    std::ofstream outFile(tokenFile);
    if (!outFile.is_open()) {
        PLOGD << "error opening token json file";
        exit(EXIT_FAILURE);
    }
    outFile << j.dump(2);
}

json makePostRequest(httplib::Client& client, httplib::Params& params) {
    auto res = client.Post("/oauth/token", params);
    if (!res || res->status != 200) {
        throw std::runtime_error("failed to exchange auth code");
    }

    json j = json::parse(res->body);
    saveTokens(j);
    return j;
}

json exchangeAuthCode(httplib::Client& client, const std::string& authCode) {
    httplib::Params params = {
        {"client_id", readEnvFile("CLIENT_ID")},
        {"client_secret", readEnvFile("CLIENT_SECRET")},
        {"code", authCode},
        {"grant_type", "authorization_code"}
    };

    json j = makePostRequest(client, params);
    return j;
}

json updateAccessToken(httplib::Client& client, const std::string& refreshToken) {
    httplib::Params params = {
        {"client_id", readEnvFile("CLIENT_ID")},
        {"client_secret", readEnvFile("CLIENT_SECRET")},
        {"grant_type", "refresh_token"},
        {"refresh_token", refreshToken}
    };

    json j = makePostRequest(client, params);
    return j;
}

std::string getValidAccessToken(httplib::Client& client, const std::optional<std::string>& authCode) {
    json tokens;
    if (authCode && authCode != "") {
        PLOGD << "exchanging auth code";
        tokens = exchangeAuthCode(client, *authCode);
    } else {
        auto j = readTokensFromJson();
        if (!j) {
            throw std::runtime_error("no auth code or tokens found");
        }
        tokens = *j;

        long now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        long expiration = tokens["expires_at"];

        if (now >= expiration) {
            PLOGD << "access token expired, generating new";
            tokens = updateAccessToken(client, tokens["refresh_token"]);
        }
    }

    return  "Bearer " + tokens["access_token"].get<std::string>();
}

