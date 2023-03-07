
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


namespace ifoot3d {
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructLeg(
        const std::vector<std::vector<std::vector<cv::Mat>>>& inputData,
        const std::string& logPath)
    {
        using namespace std;
        using namespace open3d;

        if (inputData.empty())
        {
            std::cout << "ifoot3d::reconstructLeg: inputData.empty()" << std::endl;
            return nullptr;
        }

        for (const auto di : inputData)
        {
            for (const auto& frame_data : di)
            {
                if (frame_data.size() < 3)
                {
                    std::cout << "ifoot3d::reconstructLeg: wrong input data" << std::endl;
                    return nullptr;
                }
            }
        }

        bool save_logs = !logPath.empty();
        if (save_logs && !std::filesystem::exists(std::filesystem::path(logPath)))
        {
            std::filesystem::create_directories(std::filesystem::path(logPath));
        }

        vector<shared_ptr<geometry::PointCloud>> rightLegs, leftLegs;
        vector<Plane> rightFloors, leftFloors;

        for (int i = 0; i < inputData[0].size(); i++)
        {
            auto pcd = generatePointCLoud(inputData[0][i][0], inputData[0][i][1], inputData[0][i][2]);
            if (pcd->IsEmpty())
            {
                std::cout << "ifoot3d::reconstructLeg: pcd empty right" << std::endl;
                continue;
            }

            auto legPCDFloor = segmentLeg(pcd, { 0,0,0 });
            auto leg = get<0>(legPCDFloor);

            if (leg->IsEmpty())
            {
                std::cout << "ifoot3d::reconstructLeg: leg pcd empty right" << std::endl;
                continue;
            }

            leg->EstimateNormals();
            leg->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            rightLegs.push_back(leg);
            rightFloors.push_back(get<1>(legPCDFloor));
        }

        for (int i = 0; i < inputData[1].size(); i++)
        {
            auto pcd = generatePointCLoud(inputData[1][i][0], inputData[1][i][1], inputData[1][i][2]);

            if (pcd->IsEmpty())
            {
                std::cout << "ifoot3d::reconstructLeg: pcd empty left" << std::endl;
                continue;
            }

            auto legPCDFloor = segmentLeg(pcd, { 0,0,0 });
            auto leg = get<0>(legPCDFloor);

            if (leg->IsEmpty())
            {
                std::cout << "ifoot3d::reconstructLeg: leg pcd empty left" << std::endl;
                continue;
            }

            leg->EstimateNormals();
            leg->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            leftLegs.push_back(leg);
            leftFloors.push_back(get<1>(legPCDFloor));
        }

        auto stitchedSides = stitchLegsWithFloors(rightLegs, rightFloors, leftLegs, leftFloors);
        shared_ptr<geometry::PointCloud> leftSide = stitchedSides[1], rightSide = stitchedSides[0];
        
        if (leftSide->IsEmpty() || rightSide->IsEmpty())
        {
            std::cout << "ifoot3d::reconstructLeg: leftSide->IsEmpty() || rightSide->IsEmpty()" << std::endl;
        }
        
        shared_ptr<geometry::PointCloud> finalLeg(new geometry::PointCloud());
        *finalLeg += *leftSide;
        *finalLeg += *rightSide;

        if (save_logs)
        {
            io::WritePointCloud(logPath + "/logs_right_side.pcd", *rightSide);
            io::WritePointCloud(logPath + "/logs_left_side.pcd", *leftSide);
        }

        vector<shared_ptr<geometry::PointCloud>> soles;
        shared_ptr<geometry::PointCloud> referenceSole;
        for (int i = 0; i < inputData[2].size(); i++)
        {
            auto pcd = generatePointCLoud(inputData[2][i][0], inputData[2][i][1], inputData[2][i][2]);

            if (pcd->IsEmpty())
            {
                std::cout << "ifoot3d::reconstructLeg: pcd empty sole" << std::endl;
                continue;
            }

            auto sole = segmentSole(pcd, 0.1);

            if (sole->IsEmpty())
            {
                std::cout << "ifoot3d::reconstructLeg: sole pcd empty left" << std::endl;
                continue;
            }

            sole->EstimateNormals();
            sole->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            if (i == 0)
                referenceSole = sole;
            else
                soles.push_back(sole);

            if (save_logs)
            {
                io::WritePointCloud(logPath + "/logs_sole_" + to_string(i) + ".pcd", *sole);
            }
        }

        // todo : debug 
        stitchSoles(soles, referenceSole, logPath);

        shared_ptr<geometry::PointCloud> sole(new geometry::PointCloud());
        for (auto& segment : soles)
        {
            *sole += *segment;
        }

        if (save_logs)
        {
            io::WritePointCloud(logPath + "/logs_sole_stitched.pcd", *sole);
        }

        if (sole->IsEmpty())
        {
            std::cout << "ifoot3d::reconstructLeg: sole pcd empty after stitching" << std::endl;
        }

        alignSoleWithLeg(sole, finalLeg, rightFloors[0]);

        postprocessSides(sole, stitchedSides, rightFloors[0], 0.01);

        if (save_logs)
        {
            io::WritePointCloud(logPath + "/logs_final_point_cloud.pcd", *sole);
        }

        //visualization::DrawGeometries({ sole });

        auto legMesh = reconstructSurfacePoisson(sole, 6);
        legMesh->PaintUniformColor({ 0.7, 0.7, 0.7 });

        return legMesh;
    }

    // deprecated function ?
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructLegExtrinsics(std::vector<std::vector<std::vector<cv::Mat>>>& inputData) {
        using namespace std;
        using namespace open3d;

        vector<shared_ptr<geometry::PointCloud>> rightLegs, leftLegs;
        vector<Plane> rightFloors, leftFloors;

        Eigen::Vector4d cameraPositionCamera{ 0,0,0,1 };

        for (int i = 0; i < inputData[0].size(); i++) {
            auto pcd = generatePointCLoud(inputData[0][i][0], inputData[0][i][1], inputData[0][i][2], inputData[0][i][3]);

            auto extrinsics = fixExtrinsics(inputData[0][i][3]);
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

            auto extrinsics = fixExtrinsics(inputData[1][i][3]);
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

//        visualization::DrawGeometries({ beforeStitching });

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

        auto legMesh = reconstructSurfacePoisson(finalLeg, 7);

        return legMesh;
    }

    bool reconstructAndSaveLeg(
        const std::vector<std::vector<std::vector<cv::Mat>>>& inputData, 
        const std::string& path,
        bool logging_mode)
    {
        using namespace std;

        if (!std::filesystem::exists(std::filesystem::path(path)))
        {
            std::filesystem::create_directories(std::filesystem::path(path));
        }

        try {
            auto legMesh = reconstructLeg(inputData, logging_mode ? path : "");
            open3d::io::WriteTriangleMesh(path + "/foot.obj", *legMesh);
            return true;
        }
        catch (StitchingException& e) {
            return false;
        }
    }
}
