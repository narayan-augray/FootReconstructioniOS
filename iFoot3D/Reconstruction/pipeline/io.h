#pragma once
#include <string>
#include "open3d/Open3D.h"
#include <opencv2/core.hpp>


namespace ifoot3d {
	std::vector<std::string> readLines(std::string filePath);

	std::vector<cv::Mat> readIntrinsicsExtrinsics(std::string inputPath);

	cv::Mat lidTxtToArray(const std::string& fileName);

	cv::Mat createIntrinsicsLidar(const cv::Mat& mInt, const cv::Size2i& shI, const cv::Size2i& shL);

	std::vector<cv::Mat> readInputData(const std::string& imagePath, const std::string& depthPath, const std::string& calibrationPath);
	
	Eigen::Matrix4d fixExtrinsics(const cv::Mat& extrinsic);

	std::shared_ptr<open3d::geometry::PointCloud> generatePointCLoud(cv::Mat& image, cv::Mat& depth, cv::Mat& intrinsic);

	std::shared_ptr<open3d::geometry::PointCloud> generatePointCLoud(cv::Mat& image, cv::Mat& depth, cv::Mat& intrinsic, const cv::Mat& extrinsic);

	std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(std::string legDataPath, std::string soleDataPath, std::vector<int>& rightSideIndexes, std::vector<int>& leftSideIndexes, std::vector<int>& soleIndexes);

	std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(const std::vector<std::vector<std::string>>& rightSidePaths, const std::vector<std::vector<std::string>>& leftSidePaths, const std::vector<std::vector<std::string>>& solePaths);
	
	std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(std::string legDataPath,
		std::vector<int>& rightSideIndexes,
		std::vector<int>& leftSideIndexes,
		const std::vector<std::vector<std::string>>& solePaths, const std::string& logFolderPath);
}
