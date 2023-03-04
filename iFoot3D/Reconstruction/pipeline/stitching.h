#pragma once

#include "open3d/Open3D.h"
#include <vector>
#include "util.h"

namespace ifoot3d {
    auto pairwise_registration(const open3d::geometry::PointCloud& source,
        const open3d::geometry::PointCloud& target,
        const double max_correspondence_distance_coarse,
        const double max_correspondence_distance_fine);

    auto full_registration(const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& PCDs,
        const double max_correspondence_distance_coarse,
        const double max_correspondence_distance_fine);

    std::shared_ptr<open3d::geometry::PointCloud> stitchLegs(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& rightLegs, std::vector<Plane>& rightFloors,
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& leftLegs, std::vector<Plane>& leftFloors);

    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> stitchLegsWithFloors(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& rightLegs, std::vector<Plane>& rightFloors,
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& leftLegs, std::vector<Plane>& leftFloors);

    void stitchAllLegs(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legs);
    
    void stitchSoles(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& soles, std::shared_ptr<open3d::geometry::PointCloud>& referenceSole);

    std::shared_ptr<open3d::geometry::PointCloud> stitchLegsSeparate(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& rightLegs,
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& leftLegs);

    struct StitchingException : public std::exception {
        const char* what() const throw () {
            return "ICP failed";
        }
    };
}
