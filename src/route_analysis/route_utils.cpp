#include <polylineencoder.h>
#include "route_utils.h"

#include <iostream>

namespace RouteUtils {
    void PolylineManager::parsePolylineData(const std::string& polylineString, bool verbose) {
        auto polyline = gepaf::PolylineEncoder<>::decode(polylineString);
        if (verbose) {
            for (const auto& point : polyline) {
                std::cout << "lat: " << point.latitude() << ", lng: " << point.longitude() << '\n';
            }
        }
    }
}