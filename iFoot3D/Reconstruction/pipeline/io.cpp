#include "io.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <open3d/Open3D.h>
#include "util.h"


namespace ifoot3d {
	std::vector<std::string> readLines(std::string filePath) {
		std::ifstream inputStream(filePath);
		std::vector<std::string> lines;
		std::string line;
		if (inputStream.is_open()) {
			while (inputStream.good()) {
				std::getline(inputStream, line);
				lines.push_back(line);
			}
		}
		else {
			throw std::runtime_error("Failed to open file " + filePath);
		}
		inputStream.close();
		return lines;
	}

	std::vector<cv::Mat> readIntrinsicsExtrinsics(std::string inputPath) {
        using namespace std;
        using namespace cv;
		vector<string> lines = readLines(inputPath);
		vector<string> intrinsic_lines (lines.begin() + 1, lines.begin() + 4);
		vector<string> extrinsic_lines (lines.begin() + 6, lines.begin() + 10);
		
		vector<float> intrinsicValues = parseFloatData(intrinsic_lines, ",");
		vector<float> extrinsicValues = parseFloatData(extrinsic_lines, ",");

        Mat intrinsic(3, 3, CV_64F), extrinsic(4, 4, CV_64F);
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                intrinsic.at<double>(i,j) = intrinsicValues[3 * i + j];
            }
        }
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                extrinsic.at<double>(i, j) = extrinsicValues[4 * i + j];
            }
        }

		return { intrinsic, extrinsic };
	}

    cv::Mat lidTxtToArray(const std::string& fileName) {
        using namespace std;
        using namespace cv;
        int nx = -1;

        ifstream in(fileName);
        CV_Assert(in.good());
        vector<double> data;
        string line, s;
        int ix, iy;
        for (iy = 0;; ++iy) {
            if (!getline(in, line))
                break;
            istringstream iss(line);
            for (ix = 0;; ++ix) {
                if (!getline(iss, s, ','))
                    break;
                data.push_back(stod(s));
            }
            if (nx == -1)
                nx = ix;
            else
                CV_Assert(nx == ix);
        }
        int ny = iy;
        Mat m(ny, nx, CV_64F);
        memcpy(m.data, data.data(), ny * nx * sizeof(double));

        return m;
    }

    cv::Mat createIntrinsicsLidar(const cv::Mat& mInt, const cv::Size2i& shI, const cv::Size2i& shL) {
        using namespace std;
        using namespace cv;
        Mat m = mInt.clone();
        if (shI != shL) {
            m.row(0) *= shL.width * 1.0 / shI.width;
            m.row(1) *= shL.height * 1.0 / shI.height;
        }
        return m;
    }

    std::vector<cv::Mat> readInputData(const std::string& imagePath, const std::string& depthPath, const std::string& calibrationPath) {
        using namespace std;
        using namespace cv;

        Mat img = imread(imagePath);
        Mat depth = lidTxtToArray(depthPath);
        vector<Mat> IntExt = readIntrinsicsExtrinsics(calibrationPath);
        Mat mInt = IntExt[0];
        Mat mExtr = IntExt[1];

        return { img, depth, mInt, mExtr};
    }

    Eigen::Matrix4d fixExtrinsics(const cv::Mat& extrinsic) {
        double fixData[16] = { 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 };
        cv::Mat fixMatrix = cv::Mat(4, 4, CV_64F, fixData);

        cv::Mat fixedExtrinsic = extrinsic * fixMatrix;
        
        Eigen::Matrix4d res;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                res(i,j) = fixedExtrinsic.at<double>(i, j);
            }
        }
        res = res.inverse().eval();
        return res;
    }

    std::shared_ptr<open3d::geometry::PointCloud> generatePointCLoud(cv::Mat& image, cv::Mat& depth, cv::Mat& intrinsic, const cv::Mat& extrinsic) {
        using namespace std;
        using namespace cv;

        //rotate(image, image, ROTATE_90_COUNTERCLOCKWISE);
        Mat imgSmall, imgSmallRgb;
        resize(image, imgSmall, depth.size());
        cvtColor(imgSmall, imgSmallRgb, COLOR_BGR2RGB);
        Mat mIntLid = createIntrinsicsLidar(intrinsic, image.size(), depth.size());
        int imW = depth.cols, imH = depth.rows;
        open3d::geometry::Image colorRaw;
        colorRaw.Prepare(imW, imH, 3, 1);  // int8 rgb
        memcpy(colorRaw.data_.data(), imgSmallRgb.data, imW * imH * 3);

        Mat depth32F;
        depth.convertTo(depth32F, CV_32F);
        CV_Assert(depth32F.type() == CV_32F);
        open3d::geometry::Image depthRaw;
        depthRaw.Prepare(imW, imH, 1, 4);  // fp32 1-channel
        memcpy(depthRaw.data_.data(), depth32F.data, imW * imH * 4);

        // RGBD, PCD
        shared_ptr<open3d::geometry::RGBDImage> rgbdImage =
            open3d::geometry::RGBDImage::CreateFromColorAndDepth(colorRaw, depthRaw, 1., 1000., false);

        open3d::camera::PinholeCameraIntrinsic intrinsicO3D;
        intrinsicO3D.width_ = imW;
        intrinsicO3D.height_ = imH;
        intrinsicO3D.intrinsic_matrix_ = Eigen::Map<Eigen::Matrix3d>((double*)mIntLid.data).transpose();
        shared_ptr<open3d::geometry::PointCloud> pcd = open3d::geometry::PointCloud::CreateFromRGBDImage(*rgbdImage, intrinsicO3D, fixExtrinsics(extrinsic));
        return pcd;
    }

    std::shared_ptr<open3d::geometry::PointCloud> generatePointCLoud(cv::Mat& image, cv::Mat& depth, cv::Mat& intrinsic) {
        using namespace std;
        using namespace cv;

        //rotate(image, image, ROTATE_90_COUNTERCLOCKWISE);
        Mat imgSmall, imgSmallRgb;
        resize(image, imgSmall, depth.size());
        cvtColor(imgSmall, imgSmallRgb, COLOR_BGR2RGB);
        Mat mIntLid = createIntrinsicsLidar(intrinsic, image.size(), depth.size());
        int imW = depth.cols, imH = depth.rows;
        open3d::geometry::Image colorRaw;
        colorRaw.Prepare(imW, imH, 3, 1);  // int8 rgb
        memcpy(colorRaw.data_.data(), imgSmallRgb.data, imW * imH * 3);

        Mat depth32F;
        depth.convertTo(depth32F, CV_32F);
        CV_Assert(depth32F.type() == CV_32F);
        open3d::geometry::Image depthRaw;
        depthRaw.Prepare(imW, imH, 1, 4);  // fp32 1-channel
        memcpy(depthRaw.data_.data(), depth32F.data, imW * imH * 4);

        // RGBD, PCD
        shared_ptr<open3d::geometry::RGBDImage> rgbdImage =
            open3d::geometry::RGBDImage::CreateFromColorAndDepth(colorRaw, depthRaw, 1., 1000., false);

        open3d::camera::PinholeCameraIntrinsic intrinsicO3D;
        intrinsicO3D.width_ = imW;
        intrinsicO3D.height_ = imH;
        intrinsicO3D.intrinsic_matrix_ = Eigen::Map<Eigen::Matrix3d>((double*)mIntLid.data).transpose();
        shared_ptr<open3d::geometry::PointCloud> pcd = open3d::geometry::PointCloud::CreateFromRGBDImage(*rgbdImage, intrinsicO3D);
        return pcd;
    }

    std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(const std::vector<std::vector<std::string>>& rightSidePaths, const std::vector<std::vector<std::string>>& leftSidePaths, const std::vector<std::vector<std::string>>& solePaths) {
        using namespace std;
        using namespace cv;

        vector<vector<Mat>> rightLegData, leftLegData, soleData;
        for (const auto& inputPaths : rightSidePaths) {
            rightLegData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));
        }
        for (const auto& inputPaths : leftSidePaths) {
            leftLegData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));
        }
        for (const auto& inputPaths : solePaths) {
            soleData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));
        }

        return { rightLegData, leftLegData, soleData };
    }

    std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(std::string legDataPath, std::string soleDataPath, std::vector<int>& rightSideIndexes, std::vector<int>& leftSideIndexes, std::vector<int>& soleIndexes) {
        using namespace std;
        using namespace cv;

        vector<vector<Mat>> rightLegData, leftLegData, soleData;
        for (int i : rightSideIndexes) {
            auto legInputData = readInputData(legDataPath + "original_" + to_string(i) + ".png", legDataPath + "depth_logs_" + to_string(i) + ".txt", legDataPath + "depth_calibration_logs_" + to_string(i) + ".txt");
            rightLegData.push_back(legInputData);
        }
        for (int i : leftSideIndexes) {
            auto legInputData = readInputData(legDataPath + "original_" + to_string(i) + ".png", legDataPath + "depth_logs_" + to_string(i) + ".txt", legDataPath + "depth_calibration_logs_" + to_string(i) + ".txt");
            leftLegData.push_back(legInputData);
        }
        for (int i : soleIndexes) {
            auto soleInputData = readInputData(soleDataPath + "original_" + to_string(i) + ".png", soleDataPath + "depth_logs_" + to_string(i) + ".txt", soleDataPath + "depth_calibration_logs_" + to_string(i) + ".txt");
            soleData.push_back(soleInputData);
        }
        return { rightLegData, leftLegData, soleData };
    }

    std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(std::string legDataPath,
                                                                         std::vector<int>& rightSideIndexes,
                                                                         std::vector<int>& leftSideIndexes,
                                                                         const std::vector<std::vector<std::string>>& solePaths) {
        using namespace std;
        using namespace cv;
        
        vector<vector<Mat>> rightLegData, leftLegData, soleData;
        for (int i : rightSideIndexes) {
            auto legInputData = readInputData(legDataPath + "original_" + to_string(i) + ".png", legDataPath + "depth_logs_" + to_string(i) + ".txt", legDataPath + "depth_calibration_" + to_string(i) + ".txt");
            rightLegData.push_back(legInputData);
        }
        for (int i : leftSideIndexes) {
            auto legInputData = readInputData(legDataPath + "original_" + to_string(i) + ".png", legDataPath + "depth_logs_" + to_string(i) + ".txt", legDataPath + "depth_calibration_" + to_string(i) + ".txt");
            leftLegData.push_back(legInputData);
        }
        for (const auto& inputPaths : solePaths) {
            soleData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));
        }
        return { rightLegData, leftLegData, soleData };
    }
}
