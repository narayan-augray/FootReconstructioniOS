#include "stitching.h"

#include "open3d/Open3D.h"
#include <vector>
#include "alignment.h"
#include <Eigen/Dense>
#include "open3d/pipelines/registration/GlobalOptimization.h"
#include "open3d/pipelines/registration/GlobalOptimizationMethod.h"
#include "open3d/pipelines/registration/GlobalOptimizationConvergenceCriteria.h"

#include "logger.h"

namespace ifoot3d {

    auto pairwise_registration(
        const open3d::geometry::PointCloud& source,
        const open3d::geometry::PointCloud& target,
        const double max_correspondence_distance_coarse,
        const double max_correspondence_distance_fine)
    {

        if (source.IsEmpty() || target.IsEmpty())
        {
            LOG_ERROR("pairwise_registration : source.IsEmpty() || target.IsEmpty() ! ITS NOT HANDLED");
            // handle?
        }

        auto icp_coarse = open3d::pipelines::registration::RegistrationICP(source, target, max_correspondence_distance_coarse,
            Eigen::MatrixXd::Identity(4, 4), open3d::pipelines::registration::TransformationEstimationPointToPlane());
        auto icp_fine = open3d::pipelines::registration::RegistrationICP(source, target, max_correspondence_distance_fine,
            icp_coarse.transformation_, open3d::pipelines::registration::TransformationEstimationPointToPlane());
        
        auto transformationICP = icp_fine.transformation_;
        auto informationICP = open3d::pipelines::registration::GetInformationMatrixFromPointClouds(source, target,
            max_correspondence_distance_fine, icp_fine.transformation_);

        return std::make_tuple(transformationICP, informationICP);
    }

    auto full_registration(
        const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& PCDs,
        const double max_correspondence_distance_coarse,
        const double max_correspondence_distance_fine) 
    {
        if (PCDs.empty())
        {
            LOG_WARN("full_registration : PCDs.empty()");
            return open3d::pipelines::registration::PoseGraph();
        }

        LOG_DEBUG("full_registration: num pcd's =  %d", int(PCDs.size()));

        auto pose_graph = open3d::pipelines::registration::PoseGraph();
        auto odometry = Eigen::MatrixXd::Identity(4, 4);
        pose_graph.nodes_.push_back(open3d::pipelines::registration::PoseGraphNode(odometry));
        for (int sourceID = 0; sourceID < PCDs.size(); sourceID++) {
            for (int targetID = sourceID + 1; targetID < PCDs.size(); targetID++) {
                Eigen::Matrix4d transformationICP;
                Eigen::Matrix6d_u informationICP;
                std::tie(transformationICP, informationICP) = pairwise_registration(*PCDs[sourceID], *PCDs[targetID],
                    max_correspondence_distance_coarse, max_correspondence_distance_fine);
                if (targetID == sourceID + 1) 
                {
                    auto odomentry_temp = transformationICP * odometry;
                    pose_graph.nodes_.push_back(open3d::pipelines::registration::PoseGraphNode(odomentry_temp.inverse()));
                    pose_graph.edges_.push_back(open3d::pipelines::registration::PoseGraphEdge(sourceID, targetID,
                        transformationICP, informationICP, false));
                }
                else 
                {
                    pose_graph.edges_.push_back(open3d::pipelines::registration::PoseGraphEdge(sourceID, targetID,
                        transformationICP, informationICP, true));
                }
            }
        }
        return pose_graph;
    }

    std::shared_ptr<open3d::geometry::PointCloud> stitchLegs(
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& rightLegs, std::vector<Plane>& rightFloors,
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& leftLegs, std::vector<Plane>& leftFloors)
    {
        LOG_WARN("stitchLegs : function is not tested");

        using namespace std;
        using namespace open3d;
 
        double voxelSize = 0.003;
        double maxCorrespondenceDistanceCoarse = voxelSize * 5;
        double maxCorrespondenceDistanceFine = voxelSize * 3;

        initLegsPositions(rightLegs, rightFloors);
        
        auto poseGraph = full_registration(rightLegs, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        auto option = open3d::pipelines::registration::GlobalOptimizationOption(maxCorrespondenceDistanceFine, 0.25, 0);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < rightLegs.size(); i++) {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*rightLegs[i], *rightLegs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ < -0.3) {
                    throw StitchingException();
                }
            }
            rightLegs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }

        shared_ptr<geometry::PointCloud> rightSide(new geometry::PointCloud());
        for (auto& segment : rightLegs) {
            *rightSide += *segment;
        }
        rightSide = rightSide->VoxelDownSample(0.002);

        initLegsPositions(leftLegs, leftFloors);

        poseGraph = full_registration(leftLegs, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < leftLegs.size(); i++) {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*leftLegs[i], *leftLegs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ < -0.3) {
                    throw StitchingException();
                }
            }
            leftLegs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }
        shared_ptr<geometry::PointCloud> leftSide(new geometry::PointCloud());
        for (auto& segment : leftLegs) {
            *leftSide += *segment;
        }
        leftSide = leftSide->VoxelDownSample(0.002);

//        visualization::DrawGeometries({ leftSide });
//        visualization::DrawGeometries({ rightSide });

        *rightSide += *leftSide;
//        visualization::DrawGeometries({ rightSide });
        return rightSide;
    }

    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> stitchLegsWithFloors(
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& rightLegs, 
        std::vector<Plane>& rightFloors,
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& leftLegs,
        std::vector<Plane>& leftFloors) 
    {
        LOG_TRACE("stitchLegsWithFloors:  start");

        if (rightLegs.empty() || leftLegs.empty())
        {
            LOG_ERROR("stitchLegsWithFloors : rightLegs.empty() || leftLegs.empty()");
            return std::vector<std::shared_ptr<open3d::geometry::PointCloud>>();
        }

        if (rightFloors.empty() || leftFloors.empty())
        {
            LOG_ERROR("stitchLegsWithFloors : rightFloors.empty() || leftFloors.empty()");
            return std::vector<std::shared_ptr<open3d::geometry::PointCloud>>();
        }

        LOG_TRACE("stitchLegsWithFloors:  num left pcd's = %d", int(leftLegs.size()));
        LOG_TRACE("stitchLegsWithFloors:  num right pcd's = %d", int(rightLegs.size()));

        using namespace std;
        using namespace open3d;

        double voxelSize = 0.003;
        double maxCorrespondenceDistanceCoarse = voxelSize * 10;
        double maxCorrespondenceDistanceFine = voxelSize * 4;

        LOG_TRACE("stitchLegsWithFloors:  initLegsPositions right");

        initLegsPositions(rightLegs, rightFloors);
        shared_ptr<geometry::PointCloud> rightSideBefore(new geometry::PointCloud());
        Eigen::Vector3d heel = getLegHeel(rightLegs[0], rightFloors[0]);

        LOG_DEBUG("stitchLegsWithFloors : heel point = %.3f  %.3f  %.3f ", heel[0], heel[1], heel[2]);

        auto floor = rightFloors[0].getPointCloud(0.5, 100, heel);
        
        if (floor->IsEmpty())
        {
            LOG_WARN("stitchLegsWithFloors:  floor->IsEmpty()");
        }

        vector<shared_ptr<geometry::PointCloud>> rightLegsWithFloors;
        for (int i = 0; i < rightLegs.size(); i++) 
        {
            auto leg = make_shared<geometry::PointCloud>(*rightLegs[i]);
            *rightSideBefore += *leg;
            *leg += *floor;
            rightLegsWithFloors.push_back(leg);
        }

        LOG_TRACE("stitchLegsWithFloors:  full_registration right");
        auto poseGraph = full_registration(rightLegsWithFloors, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        
        if (poseGraph.nodes_.empty() || poseGraph.edges_.empty())
        {
            LOG_WARN("stitchLegsWithFloors:  poseGraph.nodes_.empty() || poseGraph.edges_.empty()");
        }

        auto option = open3d::pipelines::registration::GlobalOptimizationOption(maxCorrespondenceDistanceFine, 0.25, 0);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        
        for (int i = 0; i < rightLegs.size(); i++)
        {
            if (i > 0)
            {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*rightLegs[i], *rightLegs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ == 0.0)
                {
                    LOG_WARN("stitchLegsWithFloors:  regRes.fitness_ == 0.0");
                    throw StitchingException();
                }
            }
            rightLegs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }

        shared_ptr<geometry::PointCloud> rightSide(new geometry::PointCloud());
        for (auto& segment : rightLegs)
        {
            *rightSide += *segment;
        }
        rightSide = rightSide->VoxelDownSample(0.002);

        if (rightSide->IsEmpty())
        {
            LOG_WARN("stitchLegsWithFloors:  rightSide->IsEmpty()");
        }

        LOG_TRACE("stitchLegsWithFloors:  initLegsPositions left");

        initLegsPositions(leftLegs, leftFloors);
        shared_ptr<geometry::PointCloud> leftSideBefore(new geometry::PointCloud());
        vector<shared_ptr<geometry::PointCloud>> leftLegsWithFloors;
        for (int i = 0; i < leftLegs.size(); i++)
        {
            auto leg = make_shared<geometry::PointCloud>(*leftLegs[i]);
            *leftSideBefore += *leg;
            *leg += *floor;
            leftLegsWithFloors.push_back(leg);      
        }

        LOG_TRACE("stitchLegsWithFloors:  full_registration left");
        poseGraph = full_registration(leftLegsWithFloors, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        
        if (poseGraph.nodes_.empty() || poseGraph.edges_.empty())
        {
            LOG_WARN("stitchLegsWithFloors:  poseGraph.nodes_.empty() || poseGraph.edges_.empty()");
        }

        
        pipelines::registration::GlobalOptimization(
            poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
       
        for (int i = 0; i < leftLegs.size(); i++) 
        {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*leftLegs[i], *leftLegs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ == 0.0) 
                {
                    LOG_WARN("stitchLegsWithFloors:  regRes.fitness_ == 0.0");
                    throw StitchingException();
                }
            }
            leftLegs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }

        shared_ptr<geometry::PointCloud> leftSide(new geometry::PointCloud());
        for (auto& segment : leftLegs) {
            *leftSide += *segment;
        }
        leftSide = leftSide->VoxelDownSample(0.002);

        if (leftSide->IsEmpty())
        {
            LOG_WARN("stitchLegsWithFloors:  leftSide->IsEmpty()");
        }

        /*visualization::DrawGeometries({ leftSideBefore });
        visualization::DrawGeometries({ rightSideBefore });
        visualization::DrawGeometries({ leftSide });
        visualization::DrawGeometries({ rightSide });*/

        LOG_TRACE("stitchLegsWithFloors:  end");

        return {rightSide, leftSide};
    }

    void stitchAllLegs(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legs)
    {
        LOG_WARN("stitchAllLegs : function is not tested");

        using namespace std;
        using namespace open3d;

        double voxelSize = 0.003;
        double maxCorrespondenceDistanceCoarse = voxelSize * 5;
        double maxCorrespondenceDistanceFine = voxelSize * 3;

        auto poseGraph = full_registration(legs, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        auto option = open3d::pipelines::registration::GlobalOptimizationOption(maxCorrespondenceDistanceCoarse, 0.25, 0);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < legs.size(); i++) {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*legs[i], *legs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ < -0.3) {
                    throw StitchingException();
                }
            }
            legs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }
    }

    std::shared_ptr<open3d::geometry::PointCloud> stitchLegsSeparate(
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& rightLegs,
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& leftLegs) 
    {
        LOG_WARN("stitchLegsSeparate : function is not tested");

        using namespace std;
        using namespace open3d;

        double voxelSize = 0.003;
        double maxCorrespondenceDistanceCoarse = voxelSize * 5;
        double maxCorrespondenceDistanceFine = voxelSize * 3;

        auto poseGraph = full_registration(rightLegs, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        auto option = open3d::pipelines::registration::GlobalOptimizationOption(maxCorrespondenceDistanceFine, 0.25, 0);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < rightLegs.size(); i++) {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*rightLegs[i], *rightLegs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ < -0.3) {
                    throw StitchingException();
                }
            }
            rightLegs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }

        shared_ptr<geometry::PointCloud> rightSide(new geometry::PointCloud());
        for (auto& segment : rightLegs) {
            *rightSide += *segment;
        }
        rightSide = rightSide->VoxelDownSample(0.002);

        poseGraph = full_registration(leftLegs, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < leftLegs.size(); i++) {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*leftLegs[i], *leftLegs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ < -0.3) {
                    throw StitchingException();
                }
            }
            leftLegs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }
        shared_ptr<geometry::PointCloud> leftSide(new geometry::PointCloud());
        for (auto& segment : leftLegs) {
            *leftSide += *segment;
        }
        leftSide = leftSide->VoxelDownSample(0.002);

//        visualization::DrawGeometries({ leftSide });
//        visualization::DrawGeometries({ rightSide });

        auto transform = get<0>(pairwise_registration(*rightSide, *leftSide, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine));
        rightSide->Transform(transform);
        *rightSide += *leftSide;
//        visualization::DrawGeometries({ rightSide });
        return rightSide;
    }

    void stitchSoles(
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& soles, 
        std::shared_ptr<open3d::geometry::PointCloud>& referenceSole,
        const std::string& logPath) 
    {
        LOG_TRACE("stitchSoles:  start");

        using namespace std;
        using namespace open3d;

        if (referenceSole->IsEmpty())
        {
            LOG_ERROR("stitchSoles : referenceSole->IsEmpty()");
            return;
        }

        LOG_DEBUG("stitchSoles : num soles = %d  ", int(soles.size()));

        initSolesPositions(soles, referenceSole, logPath);

        soles.insert(soles.begin(), referenceSole);
                
        double voxelSize = 0.003;
        double maxCorrespondenceDistanceCoarse = voxelSize * 5;
        double maxCorrespondenceDistanceFine = voxelSize * 1.5;

        LOG_TRACE("stitchSoles:  full_registration");
        auto poseGraph = full_registration(soles, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        auto option = open3d::pipelines::registration::GlobalOptimizationOption(maxCorrespondenceDistanceFine, 0.25, 0);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < soles.size(); i++)
        {
            if (i > 0) 
            {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*soles[i], *soles[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_);
                
                LOG_DEBUG("stitchSoles : registration : index = %d  ", i);
                LOG_DEBUG("stitchSoles : registration fittness  = %.5f  ", regRes.fitness_);
                LOG_DEBUG("stitchSoles : registration set size  = %d  ", int(regRes.correspondence_set_.size()));

                if (regRes.fitness_ == 0.0) 
                {
                    LOG_WARN("stitchSoles:  regRes.fitness_ == 0.0");
                    throw StitchingException();
                }
            }
            soles[i]->Transform(poseGraph.nodes_[i].pose_);
        }

        LOG_TRACE("stitchSoles:  end");
    }
}
