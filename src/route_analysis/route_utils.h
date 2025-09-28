#include <string>
#include <polylineencoder.h>
#include <nlohmann/json.hpp>

#ifndef ROUTE_UTILS
#define ROUTE_UTILS

using json = nlohmann::json;

namespace RouteUtils {
    using Point = gepaf::PolylineEncoder<>::Point;
    std::vector<Point> parsePolylineData(const std::string& polylineString, bool verbose=false);

    bool areRoutesSame(const std::string& first, const std::string& second, bool verbose=false, double threshold=0.8);

    std::optional<json> getRoutes(const std::string& path);

    std::optional<json> getAvgRouteStats(const std::string& path, const std::string& activityPath);
}

#endif