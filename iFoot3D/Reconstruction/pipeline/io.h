#pragma once
#include <string>
#include "open3d/Open3D.h"
#include <opencv2/core.hpp>

#include "logger.h"

namespace ifoot3d {

	/** @brief  initialize logger. status will be returned
	@param verbosity level  - Message levels lower than this value will be discarded.
	The default log level is INFO (2).
	List of verbosity levels
		LogLevel_TRACE = 0,
		LogLevel_DEBUG = 1,
		LogLevel_INFO  = 2,
		LogLevel_WARN  = 3,
		LogLevel_ERROR = 4,
		LogLevel_FATAL = 5,
	@param file  - path for output logfile, in case of empty argument - console logger will be used
	*/
	bool init_logger(int verbose_level = 2, const std::string& file = "");

	bool readLines(const std::string& filePath, std::vector<std::string>& lines);

	bool readIntrinsicsExtrinsics(const std::string& inputPath, cv::Mat& intrinsic, cv::Mat& extrinsic);

	bool lidTxtToArray(const std::string& fileName, cv::Mat& matrix);

	bool createIntrinsicsLidar(const cv::Mat& mInt, cv::Mat& mOut, const cv::Size2i& shI, const cv::Size2i& shL);

	std::vector<cv::Mat> readInputData(
		const std::string& imagePath, 
		const std::string& depthPath, 
		const std::string& calibrationPath);
	
	bool fixExtrinsics(const cv::Mat& extrinsic, Eigen::Matrix4d& fixed);

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
		const std::vector<std::vector<std::string>>& solePaths);

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
		const std::vector<std::vector<std::string>>& solePaths);
}
