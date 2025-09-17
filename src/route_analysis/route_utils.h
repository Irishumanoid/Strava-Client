#include <string>
#include <polylineencoder.h>

#ifndef ROUTE_UTILS
#define ROUTE_UTILS

namespace RouteUtils {
    using Point = gepaf::PolylineEncoder<>::Point;
    std::vector<Point> parsePolylineData(const std::string& polylineString, bool verbose=false);

    bool areRoutesSame(const std::string& first, const std::string& second, bool verbose=false, double threshold=0.8);
}

#endif