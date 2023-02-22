#include "postprocessing.h"

#include "open3d/Open3D.h"
#include "util.h"

namespace ifoot3d {
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructSurfacePoisson(std::shared_ptr<open3d::geometry::PointCloud>& pcd, int depth) {
        using namespace std;
        using namespace open3d;
        
        auto mesh = get<0>(geometry::TriangleMesh::CreateFromPointCloudPoisson(*pcd, depth));
        //mesh->FilterSmoothSimple(2);
        return mesh;
    }

    void postprocess(std::shared_ptr<open3d::geometry::PointCloud>& sole, std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor, double shift) {
        using namespace std;
        using namespace open3d;
        
        auto dividingPlane = Plane(floor.getPoints()[0] - shift * floor.getNormal(), floor.getNormal());
        
        vector<size_t> indices;
        for (int i = 0; i < sole->points_.size(); ++i) {
            if (dividingPlane.signedDistanceFromPoint(sole->points_[i]) > 0)
                indices.push_back(i);
        }
        sole = sole->SelectByIndex(indices);
        sole = sole->VoxelDownSample(0.002);

        indices.clear();
        for (int i = 0; i < leg->points_.size(); ++i) {
            if (dividingPlane.signedDistanceFromPoint(leg->points_[i]) < 0)
                indices.push_back(i);
        }
        leg = leg->SelectByIndex(indices);
        leg = leg->VoxelDownSample(0.002);
        *leg += *sole;

        alignGeometryByPointAndVector(leg, { 0,0,0 }, leg->GetCenter(), { 0, 1, 0 }, -floor.getNormal());
        
        Line line = getLegAxis(leg, floor);
        
        Eigen::Vector3d toe = getLegToe(sole, line);
        toe[1] = 0;
        alignGeometryByPointAndVector(leg, { 0,0,0 }, { 0,0,0 }, { 1, 0, 0 }, toe);
    }
}
