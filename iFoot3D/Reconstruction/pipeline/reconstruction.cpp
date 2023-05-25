
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>

#include "reconstruction.h"

#include "open3d/Open3D.h"
#include "util.h"
#include "segmentation.h"
#include "io.h"
#include "stitching.h"
#include "alignment.h"
#include "postprocessing.h"

#include "logger.h"

namespace ifoot3d {
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructLeg(
        const std::vector<std::vector<std::vector<cv::Mat>>>& inputData,
        const std::string& logPath)
    {
        using namespace std;
        using namespace open3d;

        LOG_TRACE("reconstructLeg : start");

        if (inputData.empty())
        {
            LOG_ERROR("reconstructLeg : inputData.empty()");
            return std::shared_ptr<open3d::geometry::TriangleMesh>();
        }

        for (const auto& di : inputData)
        {
            for (const auto& frame_data : di)
            {
                if (frame_data.size() < 3)
                {
                    LOG_ERROR("reconstructLeg : frame_data.size() < 3");
                    return std::shared_ptr<open3d::geometry::TriangleMesh>();
                }
            }
        }

        bool save_logs = !logPath.empty() && logger_getLevel() <= LogLevel::LogLevel_DEBUG;
        LOG_TRACE("reconstructLeg : save logs: %d", int(save_logs));

        if (save_logs && !std::filesystem::exists(std::filesystem::path(logPath)))
        {
            std::filesystem::create_directories(std::filesystem::path(logPath));
        }

        vector<shared_ptr<geometry::PointCloud>> rightLegs, leftLegs;
        vector<Plane> rightFloors, leftFloors;

        LOG_DEBUG("reconstructLeg : process right data start");

        for (int i = 0; i < inputData[0].size(); i++)
        {
            LOG_TRACE("reconstructLeg: process image # %d", i);

            auto pcd = generatePointCLoud(inputData[0][i][0], inputData[0][i][1], inputData[0][i][2]);
           
            if (pcd->IsEmpty())
            {
                LOG_ERROR("reconstructLeg : pcd empty right");
                continue;
            }

            int num_points = pcd->points_.size();
            LOG_TRACE("reconstructLeg: num points = %d", num_points);

            auto legPCDFloor = segmentLeg(pcd, { 0,0,0 });
            auto& leg = get<0>(legPCDFloor);

            if (leg->IsEmpty())
            {
                LOG_ERROR("reconstructLeg :leg pcd empty right");
                continue;
            }

            leg->EstimateNormals();
            leg->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            rightLegs.push_back(leg);
            rightFloors.push_back(get<1>(legPCDFloor));
        }

        LOG_DEBUG("reconstructLeg : process left data start");

        for (int i = 0; i < inputData[1].size(); i++)
        {
            LOG_TRACE("reconstructLeg: process image # %d", i);

            auto pcd = generatePointCLoud(inputData[1][i][0], inputData[1][i][1], inputData[1][i][2]);

            if (pcd->IsEmpty())
            {
                LOG_ERROR("reconstructLeg : pcd empty left");
                continue;
            }

            int num_points = pcd->points_.size();
            LOG_TRACE("reconstructLeg: num points = %d", num_points);

            auto legPCDFloor = segmentLeg(pcd, { 0,0,0 });
            auto leg = get<0>(legPCDFloor);

            if (leg->IsEmpty())
            {
                LOG_ERROR("reconstructLeg :leg pcd empty left");
                continue;
            }

            leg->EstimateNormals();
            leg->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            leftLegs.push_back(leg);
            leftFloors.push_back(get<1>(legPCDFloor));
        }

        auto stitchedSides = stitchLegsWithFloors(rightLegs, rightFloors, leftLegs, leftFloors);
        shared_ptr<geometry::PointCloud> leftSide = stitchedSides[1], rightSide = stitchedSides[0];
        
        if (leftSide->IsEmpty())
        {
            LOG_WARN("reconstructLeg :leftSide->IsEmpty()");
        }

        if (rightSide->IsEmpty())
        {
            LOG_WARN("reconstructLeg :rightSide->IsEmpty()");
        }
        
        shared_ptr<geometry::PointCloud> finalLeg(new geometry::PointCloud());
        *finalLeg += *leftSide;
        *finalLeg += *rightSide;

        if (save_logs)
        {
            LOG_TRACE("reconstructLeg: save left/right point clouds");
            io::WritePointCloud(logPath + "/logs_right_side.ply", *rightSide);
            io::WritePointCloud(logPath + "/logs_left_side.ply", *leftSide);
        }

        LOG_DEBUG("reconstructLeg : process soles data start");

        vector<shared_ptr<geometry::PointCloud>> soles;
        shared_ptr<geometry::PointCloud> referenceSole;
        for (int i = 0; i < inputData[2].size(); i++)
        {
            LOG_TRACE("reconstructLeg: process image # %d", i);

            auto pcd = generatePointCLoud(inputData[2][i][0], inputData[2][i][1], inputData[2][i][2]);

            if (pcd->IsEmpty())
            {
                LOG_ERROR("reconstructLeg : pcd empty sole");
                continue;
            }

            int num_points = pcd->points_.size();
            LOG_TRACE("reconstructLeg: num points = %d", num_points);

            auto sole = segmentSole(pcd, 0.1);

            if (sole->IsEmpty())
            {
                LOG_ERROR("reconstructLeg :sole pcd empty left");
                continue;
            }

            LOG_TRACE("reconstructLeg: num sole points = %d", int(sole->points_.size()));

            sole->EstimateNormals();
            sole->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            if (i == 0)
                referenceSole = sole;
            else
                soles.push_back(sole);

            if (save_logs)
            {
                io::WritePointCloud(logPath + "/logs_sole_" + to_string(i) + ".ply", *sole);
            }
        }

        LOG_TRACE("reconstructLeg: run sticth soles");

        stitchSoles(soles, referenceSole, logPath);

        shared_ptr<geometry::PointCloud> sole(new geometry::PointCloud());
        for (auto& segment : soles)
        {
            *sole += *segment;
        }

        if (save_logs)
        {
            LOG_TRACE("reconstructLeg: save stitched soles");
            io::WritePointCloud(logPath + "/logs_sole_stitched.ply", *sole);
        }

        if (sole->IsEmpty())
        {
            LOG_WARN("reconstructLeg : sole->IsEmpty()");
        }

        LOG_TRACE("reconstructLeg: run alignSoleWithLeg");

        alignSoleWithLeg(sole, finalLeg, rightFloors[0]);

        LOG_TRACE("reconstructLeg: run postprocessSides");

        postprocessSides(sole, stitchedSides, rightFloors[0], 0.01);

        if (save_logs)
        {
            LOG_TRACE("reconstructLeg: save final point cloud");
            io::WritePointCloud(logPath + "/logs_final_point_cloud.ply", *sole);
        }

        //visualization::DrawGeometries({ sole });

        auto legMesh = reconstructSurfacePoisson(sole, 6);
        legMesh->PaintUniformColor({ 0.8, 0.8, 0.8 });

        if (legMesh->IsEmpty())
        {
            LOG_WARN("reconstructLeg : legMesh->IsEmpty()");
        }

        LOG_TRACE("reconstructLeg : end");

        return legMesh;
    }

    // deprecated function ?
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructLegExtrinsics(std::vector<std::vector<std::vector<cv::Mat>>>& inputData) {
        using namespace std;
        using namespace open3d;

        LOG_WARN("reconstructLegExtrinsics : called deprecated function");
        LOG_WARN("reconstructLegExtrinsics : function is not tested");

        vector<shared_ptr<geometry::PointCloud>> rightLegs, leftLegs;
        vector<Plane> rightFloors, leftFloors;

        Eigen::Vector4d cameraPositionCamera{ 0,0,0,1 };

        for (int i = 0; i < inputData[0].size(); i++) {
            auto pcd = generatePointCLoud(inputData[0][i][0], inputData[0][i][1], inputData[0][i][2], inputData[0][i][3]);

            Eigen::Matrix4d extrinsics;
            fixExtrinsics(inputData[0][i][3], extrinsics);
            Eigen::Vector4d cameraPosition = extrinsics.inverse() * cameraPositionCamera;
            Eigen::Vector3d cameraPositionWorld = cameraPosition.head<3>();
            auto legPCDFloor = segmentLeg(pcd, cameraPositionWorld);
            auto leg = get<0>(legPCDFloor);
            leg->EstimateNormals();
            leg->OrientNormalsTowardsCameraLocation(cameraPositionWorld);
            rightLegs.push_back(leg);
            rightFloors.push_back(get<1>(legPCDFloor));
        }

        for (int i = 0; i < inputData[1].size(); i++) {
            auto pcd = generatePointCLoud(inputData[1][i][0], inputData[1][i][1], inputData[1][i][2], inputData[1][i][3]);

            Eigen::Matrix4d extrinsics;
            fixExtrinsics(inputData[1][i][3], extrinsics);
            Eigen::Vector4d cameraPosition = extrinsics.inverse() * cameraPositionCamera;
            Eigen::Vector3d cameraPositionWorld = cameraPosition.head<3>();
            auto legPCDFloor = segmentLeg(pcd, cameraPositionWorld);
            auto leg = get<0>(legPCDFloor);
            leg->EstimateNormals();
            leg->OrientNormalsTowardsCameraLocation(cameraPositionWorld);
            leftLegs.push_back(leg);
            leftFloors.push_back(get<1>(legPCDFloor));
        }

        shared_ptr<geometry::PointCloud> beforeStitching(new geometry::PointCloud());
        for (auto& segment : leftLegs) {
            *beforeStitching += *segment;
        }
        for (auto& segment : rightLegs) {
            *beforeStitching += *segment;
        }

        //visualization::DrawGeometries({ beforeStitching });

        repairFloorNormals(rightLegs, rightFloors);

        auto finalLeg = stitchLegsSeparate(rightLegs, leftLegs);

        vector<shared_ptr<geometry::PointCloud>> soles;
        shared_ptr<geometry::PointCloud> referenceSole;
        for (int i = 0; i < inputData[2].size(); i++) {
            auto pcd = generatePointCLoud(inputData[2][i][0], inputData[2][i][1], inputData[2][i][2]);
            auto sole = segmentSole(pcd, 0.1);

            sole->EstimateNormals();
            sole->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            if (i == 0)
                referenceSole = sole;
            else
                soles.push_back(sole);
        }

        //stitchSoles(soles, referenceSole);

        shared_ptr<geometry::PointCloud> sole(new geometry::PointCloud());
        for (auto& segment : soles) {
            *sole += *segment;
        }

        alignSoleWithLeg(sole, finalLeg, rightFloors[0]);

        postprocess(sole, finalLeg, rightFloors[0], 0.01);

        auto legMesh = reconstructSurfacePoisson(finalLeg, 6);

        return legMesh;
    }

    bool reconstructAndSaveLeg(
        const std::vector<std::vector<std::vector<cv::Mat>>>& inputData, 
        const std::string& path,
        bool logging_mode)
    {
        using namespace std;

        LOG_TRACE("reconstructAndSaveLeg : start");

        if (!std::filesystem::exists(std::filesystem::path(path)))
        {
            std::filesystem::create_directories(std::filesystem::path(path));
        }

        try {
            auto legMesh = reconstructLeg(inputData, logging_mode ? path : "");

            LOG_TRACE("reconstructAndSaveLeg : save triangle mesh start");

            bool ret = open3d::io::WriteTriangleMesh(path + "/foot.obj", *legMesh);

            LOG_TRACE("reconstructAndSaveLeg : save triangle mesh ended");

            if (!ret)
            {
                LOG_ERROR("reconstructAndSaveLeg : write triangle mesh error");
            }

            return ret;
        }
        catch (StitchingException& e)
        {
            LOG_ERROR("reconstructAndSaveLeg : catched error");
            return false;
        }

        LOG_TRACE("reconstructAndSaveLeg : end");
    }
}