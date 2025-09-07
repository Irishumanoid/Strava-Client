#ifndef UTILS
#define UTILS

using json = nlohmann::json;

// read environment variables
std::string readEnvFile(std::string_view key);

// load oauth tokens from file
std::optional<json> readTokensFromJson();

// write oauth tokens to file
void saveTokens(json& j);

// exchange authorization code for access and refresh tokens and write tokens to file
json exchangeAuthCode(httplib::Client& client, const std::string& authCode);

// get new access token via refresh token
json updateAccessToken(httplib::Client& client, const std::string& refreshToken);

// get current valid access token (generate with refresh token if stale)
std::string getValidAccessToken(httplib::Client& client, const std::optional<std::string>& authCode="");

#endif