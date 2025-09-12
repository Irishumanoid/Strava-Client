#include <polylineencoder.h>
#include "route_utils.h"

#include <iostream>
#include <cmath>
#include <numbers>
#include <algorithm>
#include <vector>
#include <tuple>

namespace RouteUtils {
    using Point = gepaf::PolylineEncoder<>::Point;

    std::vector<Point> parsePolylineData(const std::string& polylineString, bool verbose) {
        auto polyline = gepaf::PolylineEncoder<>::decode(polylineString);
        if (verbose) {
            for (const auto& point : polyline) {
                std::cout << "lat: " << point.latitude() << ", lng: " << point.longitude() << '\n';
            }
        }
        return polyline;
    }

    double degToRad(double deg) {
        return deg * std::numbers::pi / 180.0;
    }

    // Haversine distance in kilometers
    double getDistance(const Point& first, const Point& second) {
        constexpr double R_km = 6371.0;
        const double lat1 = degToRad(first.latitude());
        const double lat2 = degToRad(second.latitude());
        const double lon1 =  degToRad(first.longitude());
        const double lon2 = degToRad(second.longitude());

        const double dlat = lat2 - lat1;
        const double dlon = lon2 - lon1;

        const double sinDlat = std::sin(dlat / 2.0);
        const double sinDlon = std::sin(dlon / 2.0);
        const double a = std::pow(sinDlat, 2) +
                             std::cos(lat1) * std::cos(lat2) * std::pow(sinDlon, 2);

        const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
        return R_km * c; 
    }

    bool areRoutesSame(const std::string& first, const std::string& second, bool verbose, double threshold) {
        auto parsedFirst = parsePolylineData(first, verbose);
        auto parsedSecond = parsePolylineData(second, verbose);

        // handle empty inputs
        if (parsedFirst.empty() || parsedSecond.empty()) {
            if (verbose) std::cout << "One of the polylines is empty.\n";
            return false;
        }

        const size_t nPointsA = parsedFirst.size();
        const size_t nPointsB = parsedSecond.size();

        auto index = [&](size_t i, size_t j) {
            return i * nPointsB + j;
        };

        // flattened for efficiency
        std::vector<double> distances(nPointsA * nPointsB, 0.0);
        for (size_t i = 0; i < nPointsA; ++i) {
            for (size_t j = 0; j < nPointsB; ++j) {
                distances[index(i, j)] = getDistance(parsedFirst[i], parsedSecond[j]);
            }
        }

        std::vector<double> cost(nPointsA * nPointsB, 0.0);
        cost[index(0, 0)] = distances[index(0, 0)];

        for (size_t i = 1; i < nPointsA; ++i) {
            cost[index(i, 0)] = distances[index(i, 0)] + cost[index(i-1, 0)];
        }
        for (size_t j = 1; j < nPointsB; ++j) {
            cost[index(0, j)] = distances[index(0,j)] + cost[index(0, j-1)];
        }
        for (size_t i = 1; i < nPointsA; ++i) {
            for (size_t j = 1; j < nPointsB; ++j) {
                double minPrev = std::min({cost[index(i-1, j-1)], cost[index(i-1, j)], cost[index(i, j-1)]});
                cost[index(i, j)] = distances[index(i, j)] + minPrev;
            }
        }

        // backtracking
        size_t i = nPointsA - 1;
        size_t j = nPointsB - 1;
        std::vector<std::pair<size_t, size_t>> path;
        path.emplace_back(i, j);

        while (i > 0 || j > 0) {
            if (i == 0) {
                --j;
            } else if (j == 0) {
                --i;
            } else {
                double diag = cost[index(i-1, j-1)];
                double up = cost[index(i-1, j)];     
                double left = cost[index(i, j-1)];    

                if (diag <= up && diag <= left) {
                    --i; --j;
                } else if (up <= left) {
                    --i;
                } else {
                    --j;
                }
            }
            path.emplace_back(i, j);
        }
        std::reverse(path.begin(), path.end());

        // D is the sum of local distances along the path
        double D = 0.0;
        for (auto [i, j] : path) {
            D += distances[index(i, j)];
        }
        double L = static_cast<double>(path.size());
        double avgCost = D / L;
        double score = 1.0 / (1.0 + avgCost);

        if (verbose) {
            std::cout << "D (total local cost) = " << D << " km\n";
            std::cout << "L (path length) = " << path.size() << "\n";
            std::cout << "avgCost = " << avgCost << " km\n";
            std::cout << "similarity score = " << score << "\n";
        }

        return score >= threshold;
    }
}