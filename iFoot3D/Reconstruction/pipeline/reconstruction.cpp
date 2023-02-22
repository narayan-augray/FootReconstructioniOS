#include "reconstruction.h"

#include "open3d/Open3D.h"
#include "util.h"
#include "segmentation.h"
#include "io.h"
#include "stitching.h"
#include "alignment.h"
#include "postprocessing.h"


namespace ifoot3d {
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructLeg(std::vector<std::vector<std::vector<cv::Mat>>>& inputData) {
        using namespace std;
        using namespace open3d;

        vector<shared_ptr<geometry::PointCloud>> rightLegs, leftLegs;
        vector<Plane> rightFloors, leftFloors;

        for (int i = 0; i < inputData[0].size(); i++) {
            auto pcd = generatePointCLoud(inputData[0][i][0], inputData[0][i][1], inputData[0][i][2]);

            auto legPCDFloor = segmentLeg(pcd, {0,0,0});
            auto leg = get<0>(legPCDFloor);
            leg->EstimateNormals();
            leg->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            rightLegs.push_back(leg);
            rightFloors.push_back(get<1>(legPCDFloor));
        }
        for (int i = 0; i < inputData[1].size(); i++) {
            auto pcd = generatePointCLoud(inputData[1][i][0], inputData[1][i][1], inputData[1][i][2]);

            auto legPCDFloor = segmentLeg(pcd, {0,0,0});
            auto leg = get<0>(legPCDFloor);
            leg->EstimateNormals();
            leg->OrientNormalsTowardsCameraLocation({ 0,0,0 });
            leftLegs.push_back(leg);
            leftFloors.push_back(get<1>(legPCDFloor));
        }
        stitchLegs(rightLegs, rightFloors, leftLegs, leftFloors);
        shared_ptr<geometry::PointCloud> finalLeg (new geometry::PointCloud());
        for (auto& segment : rightLegs) {
            *finalLeg += *segment;
        }
        // starting from 1 because the first pcd is the same for right and left
        for (int i = 1; i < leftLegs.size(); i++) {
            *finalLeg += *leftLegs[i];
        }
;

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

        stitchSoles(soles, referenceSole);

        shared_ptr<geometry::PointCloud> sole(new geometry::PointCloud());
        for (auto& segment : soles) {
            *sole += *segment;
        }

        alignSoleWithLeg(sole, finalLeg, rightFloors[0]);
        postprocess(sole, finalLeg, rightFloors[0], 0.015);

        *finalLeg += *sole;
        auto legMesh = reconstructSurfacePoisson(finalLeg, 6);

        return legMesh;
    }

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
        repairFloorNormals(rightLegs, rightFloors);

        stitchLegs(rightLegs, rightFloors, leftLegs, leftFloors);


        shared_ptr<geometry::PointCloud> finalLeg(new geometry::PointCloud());
        for (auto& segment : rightLegs) {
            *finalLeg += *segment;
        }
        // starting from 1 because the first pcd is the same for right and left
        for (int i = 1; i < leftLegs.size(); i++) {
            *finalLeg += *leftLegs[i];
        }


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


        stitchSoles(soles, referenceSole);

        shared_ptr<geometry::PointCloud> sole(new geometry::PointCloud());
        for (auto& segment : soles) {
            *sole += *segment;
        }

        alignSoleWithLeg(sole, finalLeg, rightFloors[0]);


        postprocess(sole, finalLeg, rightFloors[0], 0.015);

        auto legMesh = reconstructSurfacePoisson(finalLeg, 6);

        return legMesh;
    }

    bool reconstructAndSaveLeg(std::vector<std::vector<std::vector<cv::Mat>>>& inputData, const std::string& path) {
        using namespace std;
        try {
            auto legMesh = reconstructLegExtrinsics(inputData);
            open3d::io::WriteTriangleMesh(path, *legMesh);
            return true;
        }
        catch (StitchingException& e) {
            return false;
        }
    }
}