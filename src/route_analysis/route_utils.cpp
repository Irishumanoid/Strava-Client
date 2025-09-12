#include <polylineencoder.h>
#include "route_utils.h"

#include <iostream>
#include <cmath>
#include <numbers>
#include <algorithm>

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

    // distance between two geographic points using Haversine formula
    double getDistance(Point first, Point second) {
        double lngDiff = abs(first.longitude() - second.longitude()) * std::numbers::pi / 180;
        double latDiff = abs(first.latitude() - second.latitude()) * std::numbers::pi / 180;
        return 2 * std::asin(
            std::pow(std::sin(latDiff / 2), 2) 
            + std::cos(first.latitude()) * std::cos(second.latitude()) * std::pow(std::sin(lngDiff / 2), 2));
    }

    // uses Dynamic Time Warping algorithm
    bool areRoutesSame(const std::string& first, const std::string& second, bool verbose, double threshold) {
        auto parsedFirst = parsePolylineData(first, verbose);
        auto parsedSecond = parsePolylineData(second, verbose);

        auto index = [&](int i, int j) {
            return i * parsedSecond.size() + j;
        };

        // generate distance matrix (1D for efficiency)
        std::vector<double> distances(parsedFirst.size() * parsedSecond.size(), 0.0);
        for (int i = 0; i < parsedFirst.size(); i++) {
            for (int j = 0; j < parsedSecond.size(); j++) {
                double dist = getDistance(parsedFirst.at(i), parsedSecond.at(j));
                distances[index(i, j)] = dist;
            }
        }

        //cumulative distances
        std::vector<double> cost(parsedFirst.size() * parsedSecond.size(), 0.0);
        cost[0] = distances[0];
        for (int i = 1; i < parsedFirst.size(); i++) {
            cost[index(i, 0)] = distances[index(i, 0)] + cost[index(i-1, 0)];
        }
        for (int j = 1; j < parsedSecond.size(); j++) {
            cost[index(0, j)] = distances[index(0, j)] + cost[index(0, j-1)];
        }
        for (int i = 1; i < parsedFirst.size(); i++) {
            for (int j = 1; j < parsedSecond.size(); j++) {
                double min = std::min(cost[index(i-1, j)], std::min(cost[index(i, j-1)], cost[index(i-1, j-1)]));
                cost[index(i, j)] = distances[index(i, j)] + min;
            }
        }

        // calculate optimal warping path
        int n = parsedFirst.size() - 1;
        int m = parsedSecond.size() - 1;
        std::vector<std::tuple<int, int>> path = {{n, m}};
        while (n > 0 || m > 0) {
            std::tuple<int, int> minValue {};
            if (n == 0) {
                minValue = {0, m - 1}; 
            } else if (m == 0) {
                minValue = {n - 1, 0};
            } else {
                double value = std::min(cost[index(n-1, m-1)], std::min(cost[index(n-1, m)], cost[index(n, m-1)]));
                if (value == cost[index(n-1, m-1)]) {
                    minValue = {n-1, m-1};
                } else if (value == distances[index(n-1, m)]) {
                    minValue = {n-1, m};
                } else {
                    minValue = {n, m-1};
                }
            }
            path.push_back(minValue);
            n = std::get<0>(minValue);
            m = std::get<1>(minValue);
        }
        std::reverse(path.begin(), path.end());
        double totalCost {};
        for (auto [i, j] : path) {
            totalCost += cost[index(i, j)];
        }
        double avCost = totalCost / path.size();
        double score = 1 / (1 + avCost);
        std::cout << "score is: " << score;
        return (score > threshold);
    }
}