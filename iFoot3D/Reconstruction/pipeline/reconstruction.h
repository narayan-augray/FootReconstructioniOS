#pragma once

#include "open3d/Open3D.h"
#include "util.h"
#include <opencv2/core.hpp>

namespace ifoot3d {
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructLeg(std::vector<std::vector<std::vector<cv::Mat>>>& inputData, const std::string& logPath);
    bool reconstructAndSaveLeg(std::vector<std::vector<std::vector<cv::Mat>>>& inputData, const std::string& path);
}
