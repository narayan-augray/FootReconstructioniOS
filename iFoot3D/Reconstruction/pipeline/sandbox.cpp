#include "sandbox.h"

#include "open3d/Open3D.h"
#include <string>
#include "io.h"
#include "segmentation.h"
#include "reconstruction.h"
#include <iostream>

namespace ifoot3d {

    void testPCDGeneration() {
        std::string imagePath = "../../data/input/leg/original_0.png";
        std::string depthPath = "../../data/input/leg/depth_logs_0.txt";
        std::string calibrationPath = "../../data/input/leg/depth_calibration_logs_0.txt";

        auto inputData = readInputData(imagePath, depthPath, calibrationPath);
        auto pcd = generatePointCLoud(inputData[0], inputData[1], inputData[2]);

        open3d::visualization::DrawGeometries({ pcd });
    }

    void showExampleMesh() {
        auto sphere = open3d::geometry::TriangleMesh::CreateSphere(1.0);
        sphere->ComputeVertexNormals();
        sphere->PaintUniformColor({ 1.0, .0, 0.0 });
        open3d::visualization::DrawGeometries({ sphere });
    }

    void testSegmentLeg() {
        using namespace std;

        string pcdPath = "../../data/input/test.pcd";
        auto pcd = open3d::io::CreatePointCloudFromFile(pcdPath);
        open3d::visualization::DrawGeometries({ pcd });
        auto leg = segmentLeg(pcd);
        open3d::visualization::DrawGeometries({get<0>(leg) });
    }

    void testAlgorithm() {
        using namespace std;
        using namespace open3d;

        string inputLegPath = "../../data/input/leg/", inputSolePath = "../../data/input/sole/";

        vector<int> rightIndexes { 0, 7, 6 };
        vector<int> leftIndexes { 0, 1, 2, 3 };
        vector<int> soleIndexes { 2,0,1,3,5 };
        
        auto inputData = readMultipleInputData(inputLegPath, inputSolePath, rightIndexes, leftIndexes, soleIndexes);
        auto finalLeg = reconstructLeg(inputData);
        
        visualization::DrawGeometries({ finalLeg });
    }
    void testMeshGeneration() {
        using namespace std;
        using namespace open3d;

        
        vector<Eigen::Vector3d> vertices;
        vertices.push_back({ 0.0, 0.0, 0.0 });
        vertices.push_back({ 0.0, 1.0, 0.0 });
        vertices.push_back({ 0.0, 1.0, 1.0 });
        
        vector<Eigen::Vector3i> triangles;
        triangles.push_back({ 0,1,2 });
        
        auto mesh = make_shared<geometry::TriangleMesh> (vertices, triangles);
        mesh->ComputeVertexNormals();
 
        visualization::DrawGeometries({mesh});
    }
}

