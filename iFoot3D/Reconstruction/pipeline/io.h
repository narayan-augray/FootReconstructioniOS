#pragma once
#include <string>
#include "open3d/Open3D.h"
#include <opencv2/core.hpp>


namespace ifoot3d {
	std::vector<std::string> readLines(const std::string& filePath);

	bool readIntrinsicsExtrinsics(const std::string& inputPath, cv::Mat& intrinsic, cv::Mat& extrinsic);

	cv::Mat lidTxtToArray(const std::string& fileName);

	cv::Mat createIntrinsicsLidar(const cv::Mat& mInt, const cv::Size2i& shI, const cv::Size2i& shL);

	std::vector<cv::Mat> readInputData(
		const std::string& imagePath, 
		const std::string& depthPath, 
		const std::string& calibrationPath);
	
	Eigen::Matrix4d fixExtrinsics(const cv::Mat& extrinsic);

	std::shared_ptr<open3d::geometry::PointCloud> generatePointCLoud(
		const cv::Mat& image,
		const cv::Mat& depth,
		const cv::Mat& intrinsic,
		const cv::Mat& extrinsic = cv::Mat());
	
	/** @brief  main interface to read sources for foot reconstruction. 
	Array of RGB images, Depth maps, Camera intrinsics and Extrinsics will returned in format:  
	{
	{im1, depth1, intrinsic1, extrinsic1}, 
	{im2, depth2, intrinsic2, extrinsic2},
	...}
	@param rightSidePaths  - vector of filenames for Right Side reconstruction.
	@param leftSidePaths  - vector of filenames for Right Side reconstruction.
	@param solePaths  - vector of filenames for Right Side reconstruction.
	@param logFolderPath - path for output logfile, in case of empty argument - logging will be ignored
	Input format : 
	{
	{"original_image1.png", "depth_logs1.txt", "depth_calibration_logs1.txt"},
	{"original_image2.png", "depth_logs2.txt", "depth_calibration_logs2.txt"}
	...}
	*/
	std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(
		const std::vector<std::vector<std::string>>& rightSidePaths,
		const std::vector<std::vector<std::string>>& leftSidePaths,
		const std::vector<std::vector<std::string>>& solePaths,
		const std::string& logFolderPath = "");

	/** @brief  helper function to read source files.
	* */
	std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(
		const std::string& legDataPath,
		const std::string& soleDataPath,
		const std::vector<int>& rightSideIndexes,
		const std::vector<int>& leftSideIndexes,
		const std::vector<int>& soleIndexes);
	
	/** @brief  helper function to read source files.
	* */
	std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(
		const std::string& legDataPath,
		const std::vector<int>& rightSideIndexes,
		const std::vector<int>& leftSideIndexes,
		const std::vector<std::vector<std::string>>& solePaths, 
		const std::string& logFolderPath = "");
}
