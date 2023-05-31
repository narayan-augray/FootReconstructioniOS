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
    bool init_logger(int verbose_level, const std::string& file)
    {
        if (!file.empty() && !std::filesystem::exists(std::filesystem::path(file).parent_path()))
        {
            std::filesystem::create_directories(std::filesystem::path(file).parent_path());
        }

        int ret = file.empty() ? logger_initConsoleLogger(stderr) : logger_initFileLogger(file.c_str(), 1e7, 3);

        logger_setLevel(LogLevel(verbose_level));
        LOG_DEBUG("Logger initialized : %d", ret);

        return ret; 
    }

    //-------------------------------------------------------------------------------

	bool readLines(const std::string& filePath, std::vector<std::string>& lines)
    {
        if (filePath.empty())
        {
            LOG_ERROR("readLines: filePath.empty()");
            return false;
        }

        lines.clear();

		std::ifstream inputStream(filePath);

        if (!inputStream.is_open())
        {
            LOG_ERROR("readLines: Failed to open file %s", filePath.c_str());
            return false;
        }

		std::string line;

        while (inputStream.good())
        {
            std::getline(inputStream, line);
            lines.push_back(line);
        }

		inputStream.close();
		return true;
	}

	bool readIntrinsicsExtrinsics(const std::string& inputPath, cv::Mat& intrinsic, cv::Mat&extrinsic) 
    {
        using namespace std;
        using namespace cv;
		vector<string> lines;

        if(!readLines(inputPath, lines))
        {
            LOG_ERROR("readIntrinsicsExtrinsics: readLines error");
            return false;
        }

        if (lines.size() < 10)
        {
            LOG_ERROR("readIntrinsicsExtrinsics: lines.size() < 10");
            return false;
        }

		vector<string> intrinsic_lines (lines.begin() + 1, lines.begin() + 4);
		vector<string> extrinsic_lines (lines.begin() + 6, lines.begin() + 10);
		
        vector<float> intrinsicValues;
        if (!parseFloatData(intrinsic_lines, intrinsicValues, ','))
        {
            LOG_ERROR("readIntrinsicsExtrinsics: intrinsicValues empty");
            return false;
        }

        if (intrinsicValues.size() != 9)
        {
            LOG_ERROR("readIntrinsicsExtrinsics: intrinsicValues  != 9");
            return false;
        }

        vector<float> extrinsicValues;
        if (!parseFloatData(extrinsic_lines, extrinsicValues, ','))
        {
            LOG_ERROR("readIntrinsicsExtrinsics: extrinsicValues empty");
            return false;
        }

        if (extrinsicValues.size() != 16)
        {
            LOG_ERROR("readIntrinsicsExtrinsics: extrinsicValues  != 16");
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

    bool lidTxtToArray(const std::string& fileName, cv::Mat& matrix)
    {
        using namespace std;
        using namespace cv;
        int nx = -1;

        ifstream in(fileName);

        if (!in.is_open())
        {
            LOG_ERROR("lidTxtToArray: file open error");
            return false;
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
        matrix.create(ny, nx, CV_64F);
        memcpy(matrix.data, data.data(), ny * nx * sizeof(double));

        return true;
    }

    bool createIntrinsicsLidar(const cv::Mat& mInt, cv::Mat& mOut, const cv::Size2i& shI, const cv::Size2i& shL) 
    {
        using namespace std;
        using namespace cv;

        if (mInt.empty())
        {
            LOG_ERROR("createIntrinsicsLidar: mInt.empty");
            return false;
        }

        if (mInt.rows !=3 || mInt.cols != 3)
        {
            LOG_ERROR("createIntrinsicsLidar: mInt.rows !=3 || mInt.cols != 3");
            return false;
        }

        mOut = mInt.clone();
        if (shI != shL) 
        {
            mOut.row(0) *= shL.width * 1.0 / shI.width;
            mOut.row(1) *= shL.height * 1.0 / shI.height;
        }
        return true;
    }

    std::vector<cv::Mat> readInputData(
        const std::string& imagePath,
        const std::string& depthPath,
        const std::string& calibrationPath)
    {
        using namespace std;
        using namespace cv;

        if (imagePath.empty() || depthPath.empty() || calibrationPath.empty())
        {
            LOG_ERROR("readInputData: imagePath.empty() || depthPath.empty() || calibrationPath.empty()");
            return std::vector<cv::Mat>();
        }

        Mat img = imread(imagePath);

        if (img.empty())
        {
            LOG_ERROR("readInputData: img.empty()");
            return std::vector<cv::Mat>();
        }

        Mat depth;
        if(!lidTxtToArray(depthPath, depth) || depth.empty())
        {
            LOG_ERROR("readInputData: depth error");
            return std::vector<cv::Mat>();
        }

        Mat mInt, mExtr;
        if (!readIntrinsicsExtrinsics(calibrationPath, mInt, mExtr))
        {
            LOG_ERROR("readInputData: read intrinsic/extrinsic erorr");
            return std::vector<cv::Mat>();
        }      

        return { img, depth, mInt, mExtr};
    }

    bool fixExtrinsics(const cv::Mat& extrinsic, Eigen::Matrix4d& fixed)
    {
        if (extrinsic.empty() || extrinsic.cols != 4 || extrinsic.rows != 4)
        {
            LOG_ERROR("fixExtrinsics: extrinsic.empty() || extrinsic.cols != 4 || extrinsic.rows != 4");
            return false;
        }

        double fixData[16] = { 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 };
        cv::Mat fixMatrix = cv::Mat(4, 4, CV_64F, fixData);

        cv::Mat fixedExtrinsic = extrinsic * fixMatrix;
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                fixed(i,j) = fixedExtrinsic.at<double>(i, j);
            }
        }
        fixed = fixed.inverse().eval();

        return true;
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
            LOG_ERROR("generatePointCLoud: image.empty() || depth.empty() || intrinsic.empty()");
            return std::make_shared<open3d::geometry::PointCloud>();
        }

        //rotate(image, image, ROTATE_90_COUNTERCLOCKWISE);
        Mat imgSmall, imgSmallRgb;
        resize(image, imgSmall, depth.size());
        cvtColor(imgSmall, imgSmallRgb, COLOR_BGR2RGB);
        Mat mIntLid;
        if (!createIntrinsicsLidar(intrinsic, mIntLid, image.size(), depth.size()))
        {
            LOG_ERROR("generatePointCLoud: wrong output from createIntrinsicsLidar()");
            return std::make_shared<open3d::geometry::PointCloud>();
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

        if (rgbdImage->IsEmpty())
        {
            LOG_ERROR("generatePointCLoud: empty rgbdImage");
            return std::make_shared<open3d::geometry::PointCloud>();
        }

        open3d::camera::PinholeCameraIntrinsic intrinsicO3D;
        intrinsicO3D.width_ = imW;
        intrinsicO3D.height_ = imH;
        intrinsicO3D.intrinsic_matrix_ = Eigen::Map<Eigen::Matrix3d>((double*)mIntLid.data).transpose();

        shared_ptr<open3d::geometry::PointCloud> pcd;
        if (extrinsic.empty())
        {
            LOG_TRACE("generatePointCLoud: CreateFromRGBDImage without extrinsics");
            pcd = open3d::geometry::PointCloud::CreateFromRGBDImage(*rgbdImage, intrinsicO3D);
        }
        else
        {
            LOG_TRACE("generatePointCLoud: CreateFromRGBDImage with extrinsics");
            Eigen::Matrix4d extrinsic_eigen;
            if (fixExtrinsics(extrinsic, extrinsic_eigen))
            {
                pcd = open3d::geometry::PointCloud::CreateFromRGBDImage(*rgbdImage, intrinsicO3D, extrinsic_eigen);
            }
        }

        if (pcd->IsEmpty())
        {
            LOG_ERROR("generatePointCLoud: empty pcd from RGBD");
            return std::make_shared<open3d::geometry::PointCloud>();
        }

        return pcd;
    }

    // main interface 
    std::vector<std::vector<std::vector<cv::Mat>>> readMultipleInputData(
        const std::vector<std::vector<std::string>>& rightSidePaths, 
        const std::vector<std::vector<std::string>>& leftSidePaths, 
        const std::vector<std::vector<std::string>>& solePaths)
    {
        using namespace std;
        using namespace cv;

        LOG_TRACE("readMultipleInputData : start");

        if (rightSidePaths.empty() || leftSidePaths.empty() || solePaths.empty())
        {
            LOG_ERROR("readMultipleInputData : empty inputs");
            return std::vector<std::vector<std::vector<cv::Mat>>>();
        }
      
        std::stringstream ss;
        ss << "List of files: \nRight side paths\n{\n";
        //LOG_TRACE("readMultipleInputData : Right side paths");

        vector<vector<Mat>> rightLegData, leftLegData, soleData;
        for (const auto& inputPaths : rightSidePaths) 
        {
            if (inputPaths.size() != 3)
            {
                LOG_ERROR("readMultipleInputData : data size != 3");
            }

            rightLegData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));

            ss << "{\"" + inputPaths[0] + "\", \"" + inputPaths[1] + "\", \"" + inputPaths[2] + "\"},\n";
            //LOG_DEBUG(std::string(inputPaths[0] + "  " + inputPaths[1] + "  " + inputPaths[2]).c_str());
        }

        ss << "}\nLeft side paths\n{\n";
        //LOG_TRACE("readMultipleInputData : Left side paths");

        for (const auto& inputPaths : leftSidePaths)
        {
            if (inputPaths.size() != 3)
            {
                LOG_ERROR("readMultipleInputData : data size != 3");
            }

            leftLegData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));

            ss << "{\"" + inputPaths[0] + "\", \"" + inputPaths[1] + "\", \"" + inputPaths[2] + "\"},\n";
            //LOG_DEBUG(std::string(inputPaths[0] + "  " + inputPaths[1] + "  " + inputPaths[2]).c_str());
        }

        ss << "}\nSole paths\n{\n";
        //LOG_TRACE("readMultipleInputData : Sole paths");

        for (const auto& inputPaths : solePaths) 
        {
            if (inputPaths.size() != 3)
            {
                LOG_ERROR("readMultipleInputData : data size != 3");
            }
            soleData.push_back(readInputData(inputPaths[0], inputPaths[1], inputPaths[2]));

            //LOG_DEBUG(std::string(inputPaths[0] + "  " + inputPaths[1] + "  " + inputPaths[2]).c_str());
            ss << "{\"" + inputPaths[0] + "\", \"" + inputPaths[1] + "\", \"" + inputPaths[2] + "\"},\n";
        }

        ss << "}\n";
        LOG_DEBUG(ss.str().c_str());

        LOG_TRACE("readMultipleInputData : end");

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
        const std::vector<std::vector<std::string>>& solePaths)
    {
        using namespace std;
        using namespace cv;

        // check inputs
        if (legDataPath.empty())
        {
            LOG_ERROR("readMultipleInputData : legDataPath.empty()");
            return std::vector<std::vector<std::vector<cv::Mat>>>();
        }

        if (rightSideIndexes.empty() || leftSideIndexes.empty())
        {
            LOG_ERROR("readMultipleInputData : rightSideIndexes.empty() || leftSideIndexes.empty()");
            return std::vector<std::vector<std::vector<cv::Mat>>>();
        }

        if (solePaths.empty())
        {
            LOG_ERROR("readMultipleInputData : solePaths.empty()");
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
        return readMultipleInputData(rightSidePaths, leftSidePaths, solePaths);
    }
}