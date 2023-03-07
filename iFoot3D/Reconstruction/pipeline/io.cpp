#include "io.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <open3d/Open3D.h>
#include "util.h"


namespace ifoot3d {
	std::vector<std::string> readLines(const std::string& filePath)
    {
        if (filePath.empty())
        {
            std::cout << "ifoot3d::readLines: filePath.empty()" << std::endl;
            return std::vector<std::string>();
        }

		std::ifstream inputStream(filePath);

		std::vector<std::string> lines;
		std::string line;
		if (inputStream.is_open()) 
        {
			while (inputStream.good()) 
            {
				std::getline(inputStream, line);
				lines.push_back(line);
			}
		}
		else 
        {
			throw std::runtime_error("Failed to open file " + filePath);
		}
		inputStream.close();
		return lines;
	}

	bool readIntrinsicsExtrinsics(const std::string& inputPath, cv::Mat& intrinsic, cv::Mat&extrinsic) 
    {
        using namespace std;
        using namespace cv;
		vector<string> lines = readLines(inputPath);

        if (lines.empty())
        {
            std::cout << "ifoot3d::readIntrinsicsExtrinsics: readLines error" << std::endl;
            return false;
        }

		vector<string> intrinsic_lines (lines.begin() + 1, lines.begin() + 4);
		vector<string> extrinsic_lines (lines.begin() + 6, lines.begin() + 10);
		
		vector<float> intrinsicValues = parseFloatData(intrinsic_lines, ',');
        if (intrinsicValues.size() != 9)
        {
            std::cout << "ifoot3d::readIntrinsicsExtrinsics: (intrinsicValues.size() != 9" << std::endl;
            return false;
        }

		vector<float> extrinsicValues = parseFloatData(extrinsic_lines, ',');
        if (extrinsicValues.size() != 16)
        {
            std::cout << "ifoot3d::readIntrinsicsExtrinsics: (intrinsicValues.size() != 9" << std::endl;
            return false;
        }
     
        intrinsic = cv::Mat(3, 3, CV_64F);
        extrinsic = cv::Mat(4, 4, CV_64F);
        double* intrData = (double*)intrinsic.data;
        double* extrData = (double*)extrinsic.data;
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

		return true;
	}

    cv::Mat lidTxtToArray(const std::string& fileName) 
    {
        using namespace std;
        using namespace cv;
        int nx = -1;

        ifstream in(fileName);

        if (!in.is_open())
        {
            std::cout << "ifoot3d::lidTxtToArray: file open error" << std::endl;
            return cv::Mat();
        }

        vector<double> data;
        string line, s;
        int ix, iy;
        for (iy = 0;; ++iy)
        {
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

        if (mInt.empty())
        {
            std::cout << "ifoot3d::createIntrinsicsLidar: mInt.empty()" << std::endl;
            return cv::Mat();
        }
        Mat m = mInt.clone();
        if (shI != shL) {
            m.row(0) *= shL.width * 1.0 / shI.width;
            m.row(1) *= shL.height * 1.0 / shI.height;
        }
        return m;
    }

    std::vector<cv::Mat> readInputData(
        const std::string& imagePath,
        const std::string& depthPath,
        const std::string& calibrationPath)
    {
        using namespace std;
        using namespace cv;

        Mat img = imread(imagePath);

        if (img.empty())
        {
            std::cout << "readInputData : img.empty()" << std::endl;
            return std::vector<cv::Mat>();
        }

        Mat depth = lidTxtToArray(depthPath);

        if (depth.empty())
        {
            std::cout << "readInputData : depth.empty()" << std::endl;
            return std::vector<cv::Mat>();
        }

        Mat mInt, mExtr;
        if (!readIntrinsicsExtrinsics(calibrationPath, mInt, mExtr))
        {
            std::cout << "readIntrinsicsExtrinsics : read error" << std::endl;
            return std::vector<cv::Mat>();
        }      

        return { img, depth, mInt, mExtr};
    }

    Eigen::Matrix4d fixExtrinsics(const cv::Mat& extrinsic) 
    {
        if (extrinsic.empty() || extrinsic.cols != 4 || extrinsic.rows != 4)
        {
            std::cout << "fixExtrinsics : read error" << std::endl;
            return Eigen::Matrix4d::Identity();
        }

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

    std::shared_ptr<open3d::geometry::PointCloud> generatePointCLoud(
        const cv::Mat& image, 
        const cv::Mat& depth,
        const cv::Mat& intrinsic,
        const cv::Mat& extrinsic) 
    {
        using namespace std;
        using namespace cv;

        if (image.empty() || depth.empty() || intrinsic.empty())
        {
            std::cout << "generatePointCLoud : image.empty() || depth.empty() || intrinsic.empty()" << std::endl;
            return nullptr;
        }

        //rotate(image, image, ROTATE_90_COUNTERCLOCKWISE);
        Mat imgSmall, imgSmallRgb;
        resize(image, imgSmall, depth.size());
        cvtColor(imgSmall, imgSmallRgb, COLOR_BGR2RGB);
        Mat mIntLid = createIntrinsicsLidar(intrinsic, image.size(), depth.size());

        if (mIntLid.empty())
        {
            std::cout << "generatePointCLoud : wrong output from createIntrinsicsLidar()" << std::endl;
            return nullptr;
        }

        int imW = depth.cols, imH = depth.rows;
        open3d::geometry::Image colorRaw;
        colorRaw.Prepare(imW, imH, 3, 1);  // int8 rgb
        memcpy(colorRaw.data_.data(), imgSmallRgb.data, imW * imH * 3);

        Mat depth32F;
        depth.convertTo(depth32F, CV_32F);
        //CV_Assert(depth32F.type() == CV_32F);
        open3d::geometry::Image depthRaw;
        depthRaw.Prepare(imW, imH, 1, 4);  // fp32 1-channel
        memcpy(depthRaw.data_.data(), depth32F.data, imW * imH * 4);

        // RGBD, PCD
        std::shared_ptr<open3d::geometry::RGBDImage> rgbdImage;
        rgbdImage = (open3d::geometry::RGBDImage::CreateFromColorAndDepth(colorRaw, depthRaw, 1., 1000., false));

        if (rgbdImage == nullptr)
        {
            std::cout << "generatePointCLoud : wrong output from open3d::CreateFromColorAndDepth()" << std::endl;
            return nullptr;
        }

        open3d::camera::PinholeCameraIntrinsic intrinsicO3D;
        intrinsicO3D.width_ = imW;
        intrinsicO3D.height_ = imH;
        intrinsicO3D.intrinsic_matrix_ = Eigen::Map<Eigen::Matrix3d>((double*)mIntLid.data).transpose();

        shared_ptr<open3d::geometry::PointCloud> pcd;
        if (extrinsic.empty())
        {
            pcd = open3d::geometry::PointCloud::CreateFromRGBDImage(*rgbdImage, intrinsicO3D);
        }
        else
        {
            pcd = open3d::geometry::PointCloud::CreateFromRGBDImage(*rgbdImage, intrinsicO3D, fixExtrinsics(extrinsic));
        }

        return pcd;
    }

    // main interface 
    std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(
        const std::vector<std::vector<std::string>>& rightSidePaths, 
        const std::vector<std::vector<std::string>>& leftSidePaths, 
        const std::vector<std::vector<std::string>>& solePaths,
        const std::string& logFolderPath)
    {
        using namespace std;
        using namespace cv;

        if (rightSidePaths.empty() || leftSidePaths.empty() || solePaths.empty())
        {
            std::cout << "readMultipleInputData : wrong inputs" << std::endl;
            return std::vector<std::vector<std::vector<cv::Mat>>>();
        }

        //-----------------------------------------------
        // initialize  logger
        bool save_logs = !logFolderPath.empty();
        ofstream logFile;
        if (save_logs)
        {
            if (!std::filesystem::exists(std::filesystem::path(logFolderPath)))
            {
                std::filesystem::create_directories(std::filesystem::path(logFolderPath));
            }

            logFile.open(logFolderPath + "/logs.txt");

            if (!logFile.is_open())
            {
                std::cout << "readMultipleInputData : !logFile.is_open()" << std::endl;
            }

            logFile << "Right side paths\n{\n";
        }
        //-----------------------------------------------

        vector<vector<Mat>> rightLegData, leftLegData, soleData;
        for (const auto& inputPaths : rightSidePaths) 
        {
            rightLegData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));

            if (save_logs)
            {
                logFile << "{\"" + inputPaths[0] + "\", \"" + inputPaths[1] + "\", \"" + inputPaths[2] + "\"},\n";
            }
        }

        if (save_logs)
        {
            logFile << "}\n";
            logFile << "Left side paths\n{\n";
        }

        for (const auto& inputPaths : leftSidePaths)
        {
            leftLegData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));

            if (save_logs)
            {
                logFile << "{\"" + inputPaths[0] + "\", \"" + inputPaths[1] + "\", \"" + inputPaths[2] + "\"},\n";
            }
        }

        if (save_logs)
        {
            logFile << "}\n";
            logFile << "Sole paths\n{\n";
        }

        for (const auto& inputPaths : solePaths) 
        {
            soleData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));

            if (save_logs)
            {
                logFile << "{\"" + inputPaths[0] + "\", \"" + inputPaths[1] + "\", \"" + inputPaths[2] + "\"},\n";
            }
        }

        if (save_logs)
        {
            logFile << "}\n";
            logFile.close();
        }

        return { rightLegData, leftLegData, soleData };
    }

    std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(
        const std::string& legDataPath,
        const std::string& soleDataPath,
        const std::vector<int>& rightSideIndexes, 
        const std::vector<int>& leftSideIndexes,
        const std::vector<int>& soleIndexes)
    {
        using namespace std;
        using namespace cv;

        // generate inputs for data parsing
        std::vector<std::vector<std::string>> rightSidePaths, leftSidePaths, solePaths;

        for (int i : rightSideIndexes)
        {
            rightSidePaths.push_back({
                legDataPath + "original_" + to_string(i) + ".png",
                legDataPath + "depth_logs_" + to_string(i) + ".txt",
                legDataPath + "depth_calibration_logs_" + to_string(i) + ".txt"
                });
        }

        for (int i : leftSideIndexes)
        {
            leftSidePaths.push_back({
                legDataPath + "original_" + to_string(i) + ".png",
                legDataPath + "depth_logs_" + to_string(i) + ".txt",
                legDataPath + "depth_calibration_logs_" + to_string(i) + ".txt"
                });
        }

        for (int i : soleIndexes)
        {
            solePaths.push_back({
                soleDataPath + "original_" + to_string(i) + ".png",
                soleDataPath + "depth_logs_" + to_string(i) + ".txt",
                soleDataPath + "depth_calibration_logs_" + to_string(i) + ".txt"
                });
        }

        // read source data for reconstruction
        return readMultipleInputData(rightSidePaths, leftSidePaths, solePaths);
    }

    std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(
        const std::string& legDataPath,
        const std::vector<int>& rightSideIndexes,
        const std::vector<int>& leftSideIndexes,
        const std::vector<std::vector<std::string>>& solePaths,
        const std::string& logFolderPath)
    {
        using namespace std;
        using namespace cv;

        // check inputs
        if (legDataPath.empty())
        {
            std::cout << "readMultipleInputData : legDataPath.empty()" << std::endl;
            return std::vector<std::vector<std::vector<cv::Mat>>>();
        }

        if (rightSideIndexes.empty() || leftSideIndexes.empty())
        {
            std::cout << "readMultipleInputData : rightSideIndexes.empty() || leftSideIndexes.empty()" << std::endl;
            return std::vector<std::vector<std::vector<cv::Mat>>>();
        }

        if (solePaths.empty())
        {
            std::cout << "readMultipleInputData : solePaths.empty()" << std::endl;
            return std::vector<std::vector<std::vector<cv::Mat>>>();
        }

        // generate inputs for data parsing
        std::vector<std::vector<std::string>> rightSidePaths, leftSidePaths;

        for (int i : rightSideIndexes)
        {
            rightSidePaths.push_back({
                legDataPath + "original_" + to_string(i) + ".png",
                legDataPath + "depth_logs_" + to_string(i) + ".txt",
                legDataPath + "depth_calibration_" + to_string(i) + ".txt"
                });
        }

        for (int i : leftSideIndexes)
        {
            leftSidePaths.push_back({
                legDataPath + "original_" + to_string(i) + ".png",
                legDataPath + "depth_logs_" + to_string(i) + ".txt",
                legDataPath + "depth_calibration_" + to_string(i) + ".txt"
                });
        }

        // read source data for reconstruction
        return readMultipleInputData(rightSidePaths, leftSidePaths, solePaths, logFolderPath);
    }
}