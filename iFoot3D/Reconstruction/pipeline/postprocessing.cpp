#include "postprocessing.h"

#include "open3d/Open3D.h"
#include "util.h"
#include "alignment.h"

#include "logger.h"

namespace ifoot3d {
    std::shared_ptr<open3d::geometry::TriangleMesh> reconstructSurfacePoisson(std::shared_ptr<open3d::geometry::PointCloud>& pcd, int depth) {
        using namespace std;
        using namespace open3d;
        
        auto mesh = get<0>(geometry::TriangleMesh::CreateFromPointCloudPoisson(*pcd, depth));
        mesh = mesh->SubdivideLoop(1);
        mesh = mesh->FilterSmoothTaubin(7);
        mesh->triangle_normals_.clear();
        mesh->vertex_normals_.clear();
        mesh->ComputeVertexNormals();
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
        auto axis = getLegAxis(leg, floor);
        Eigen::Vector3d toe = getLegToe(sole, axis);
        toe[1] = 0;
        alignGeometryByPointAndVector(leg, { 0,0,0 }, { 0,0,0 }, { 1, 0, 0 }, toe);
    }

    void postprocessSides(
        std::shared_ptr<open3d::geometry::PointCloud>& sole, 
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legSides,
        Plane& floor,
        double shift) 
    {
        LOG_TRACE("postprocessSides: start");

        using namespace std;
        using namespace open3d;

        if (sole->IsEmpty() || legSides.empty())
        {
            LOG_ERROR("postprocessSides : sole->IsEmpty() || legSides.empty()");
            return;
        }

        auto dividingPlane = Plane(floor.getPoints()[0] - shift * floor.getNormal(), floor.getNormal());
        double intersectingThresh = -0.005;
        auto soleIntersectingPlane = Plane(floor.getPoints()[0] - intersectingThresh * floor.getNormal(), floor.getNormal());

        LOG_TRACE("postprocessSides: run alignSidesWithSole");

        alignSidesWithSole(sole, legSides, soleIntersectingPlane, floor, intersectingThresh);

        vector<size_t> indices;
        for (int i = 0; i < sole->points_.size(); ++i)
        {
            if (dividingPlane.signedDistanceFromPoint(sole->points_[i]) > 0)
                indices.push_back(i);
        }

        sole = sole->SelectByIndex(indices);
        sole = sole->VoxelDownSample(0.002);

        LOG_DEBUG("postprocessSides : sole num points downsampled =  %d", int(sole->points_.size()));

        for (auto& leg : legSides) 
        {
            indices.clear();
            for (int i = 0; i < leg->points_.size(); ++i) {
                if (dividingPlane.signedDistanceFromPoint(leg->points_[i]) < 0)
                    indices.push_back(i);
            }
            leg = leg->SelectByIndex(indices);
            leg = leg->VoxelDownSample(0.002);

            LOG_DEBUG("postprocessSides : leg side num points downsampled =  %d", int(leg->points_.size()));

            *sole += *leg;
        }

        //*sole += *legSides[0] + *legSides[1];

        alignGeometryByPointAndVector(sole, { 0,0,0 }, sole->GetCenter(), { 0, 1, 0 }, -floor.getNormal());
        auto axis = getLegAxis(sole, floor);
        Eigen::Vector3d toe = getLegToe(sole, axis);
        toe[1] = 0;
        alignGeometryByPointAndVector(sole, { 0,0,0 }, { 0,0,0 }, { 1, 0, 0 }, toe);

        LOG_TRACE("postprocessSides: end");
    }
}
