#include <string>

#ifndef ROUTE_UTILS
#define ROUTE_UTILS

namespace RouteUtils {
    class PolylineManager {
        public:
            void parsePolylineData(const std::string& polylineString, bool verbose=false);
    };
}

#endif