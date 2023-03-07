#pragma once

#include "open3d/Open3D.h"
#include "util.h"
#include <opencv2/core.hpp>

namespace ifoot3d {
	/** @brief  reconstruct foot and return 3d model.
	@param inputData  - vector of source data for reconstruction.
	data format
	{
	{im1, depth1, intrinsic1, extrinsic1},
	{im2, depth2, intrinsic2, extrinsic2},
	...}
	@param logPath  - path for output logfile, in case of empty argument - logging will be ignored
	*/
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructLeg(
        const std::vector<std::vector<std::vector<cv::Mat>>>& inputData,
        const std::string& logPath);
    
	/** @brief  reconstruct foot and save 3d model to file.
	@param inputData  - vector of source data for reconstruction.
	data format
	{
	{im1, depth1, intrinsic1, extrinsic1},
	{im2, depth2, intrinsic2, extrinsic2},
	...}
	@param path  - output directory where to save 3d model
	@param logging_mode  - boolean flag to save log data
	*/
    bool reconstructAndSaveLeg(
        const std::vector<std::vector<std::vector<cv::Mat>>>& inputData, 
        const std::string& path,
		bool logging_mode = false
	);
}
