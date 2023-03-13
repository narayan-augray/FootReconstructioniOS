#include "alignment.h"

#include "open3d/Open3D.h"
#include <Eigen/Dense>
#include "util.h"

#include "logger.h"

namespace ifoot3d {
    void initLegsPositions(
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legs, 
        std::vector<Plane>& floors) 
    {
        if (legs.empty() || floors.empty() || legs.size() != floors.size())
        {
            LOG_ERROR("initLegsPositions : legs.empty() || floors.empty() || legs.size() != floors.size()");
            return;
        }

        //repairFloorNormals(legs, floors);
        auto referenceLeg = legs[0];

        if (referenceLeg->IsEmpty())
        {
            LOG_ERROR("initLegsPositions : referenceLeg->IsEmpty()");
            return;
        }

        auto referenceFloor = floors[0];

        LOG_TRACE("initLegsPositions:  referenceLeg processing");
        auto referenceAxis = getLegAxis(referenceLeg, referenceFloor);

        Eigen::Vector3d referenceToe = getLegToe(referenceLeg, referenceAxis);
        Eigen::Vector3d ReferenceToeFloorProjection = referenceToe - referenceFloor.signedDistanceFromPoint(referenceToe) * referenceFloor.getNormal();
     
        LOG_DEBUG("ReferenceToeFloorProjection : point = %.3f  %.3f  %.3f ", ReferenceToeFloorProjection[0], ReferenceToeFloorProjection[1], ReferenceToeFloorProjection[2]);

        Eigen::Vector3d referenceHeel = getLegHeel(referenceLeg, referenceFloor);
        LOG_DEBUG("referenceHeel : point = %.3f  %.3f  %.3f ", referenceHeel[0], referenceHeel[1], referenceHeel[2]);

        for (int i = 1; i < legs.size(); i++) 
        {
            LOG_TRACE("initLegsPositions: process right leg # %d", i);

            auto currentLegAxis = getLegAxis(legs[i], floors[i]);
            alignGeometryByPointAndVector(legs[i], referenceAxis.getPoint(), currentLegAxis.getPoint(), referenceAxis.getVector(), currentLegAxis.getVector());
            
            Eigen::Vector3d currentToe = getLegToe(legs[i], referenceAxis);
            
            if (currentToe.hasNaN())
            {
                LOG_WARN("initLegsPositions : currentToe.hasNaN()");
                continue;
            }

            Eigen::Vector3d currentToeFloorProjection = currentToe - referenceFloor.signedDistanceFromPoint(currentToe) * referenceFloor.getNormal();
           
            LOG_DEBUG("currentToeFloorProjection : point = %.3f  %.3f  %.3f ", currentToeFloorProjection[0], currentToeFloorProjection[1], currentToeFloorProjection[2]);

            alignGeometryByPointAndVector(legs[i], referenceAxis.getPoint(), referenceAxis.getPoint(), ReferenceToeFloorProjection - referenceHeel, currentToeFloorProjection - referenceHeel);
            currentToe = getLegToe(legs[i], referenceAxis);
            //legs[i]->Translate(referenceToe - currentToe);
        }
    }

    void initSolesPositions(
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& soles,
        const std::shared_ptr<open3d::geometry::PointCloud>& referenceSole,
        const std::string& logPath) 
    {
        using namespace std;
        using namespace open3d;
        
        if (soles.empty())
        {
            LOG_ERROR("initSolesPositions : soles.empty()");
            return;
        }

        if (referenceSole->IsEmpty())
        {
            std::cout << "initSolesPositions :referenceSole->IsEmpty()" << std::endl;
            return;
        }

        bool save_logs = !logPath.empty() && logger_getLevel() <= LogLevel::LogLevel_DEBUG;

        Eigen::Vector3d cameraPos{ 0, 0, 0 };

        auto hull = get<0>(referenceSole->ComputeConvexHull());
        
        if (hull->IsEmpty())
        {
            LOG_ERROR("initSolesPositions : hull->IsEmpty()");
            return;
        }

        LOG_TRACE("initLegsPositions: hull num points = %d", int(hull->vertices_.size()));

        leaveVisibleMesh(hull);

        if (hull->IsEmpty())
        {
            LOG_ERROR("initSolesPositions : convex hull is empty after leaveVisibleMesh");
            return;
        }

        LOG_TRACE("initLegsPositions: hull num points visible = %d", int(hull->vertices_.size()));
        
        auto biggestTriangleAndNormal = getBiggestTriangle(hull);

        auto referenceTriangle = get<0>(biggestTriangleAndNormal);
        auto referenceNormal = get<1>(biggestTriangleAndNormal);
        
        // for soles oZ axis is inverted, so we change basis 
//        if (referenceNormal.dot(Eigen::Vector3d(0, 0, 1)) > 0.0)
//        {
//            LOG_TRACE("initSolesPositions: referenceNormal inverted");
//            referenceNormal *= -1.f;
//        }

        if (referenceTriangle.empty())
        {
            LOG_ERROR("initSolesPositions : referenceTriangle.empty()");
            return;
        }

        Eigen::Vector3d referenceHeel = getSoleHeel(referenceTriangle);

        LOG_DEBUG("initSolesPositions : referenceTriangle v1 = %.3f  %.3f  %.3f ", referenceTriangle[0][0], referenceTriangle[0][1], referenceTriangle[0][2]);
        LOG_DEBUG("initSolesPositions : referenceTriangle v2 = %.3f  %.3f  %.3f ", referenceTriangle[1][0], referenceTriangle[1][1], referenceTriangle[1][2]);
        LOG_DEBUG("initSolesPositions : referenceTriangle v3 = %.3f  %.3f  %.3f ", referenceTriangle[2][0], referenceTriangle[2][1], referenceTriangle[2][2]);
        LOG_DEBUG("initSolesPositions : referenceNormal  = %.3f  %.3f  %.3f ", referenceNormal[0], referenceNormal[1], referenceNormal[2]);
        LOG_DEBUG("initSolesPositions : referenceHeel  = %.3f  %.3f  %.3f ", referenceHeel[0], referenceHeel[1], referenceHeel[2]);

        if (referenceHeel.hasNaN())
        {
            LOG_ERROR("initSolesPositions : referenceHeel.hasNaN()");
            return;
        }

        Line legAxis = Line(referenceNormal, referenceHeel);

        Eigen::Vector3d referenceToe = getLegToe(referenceSole, legAxis);
        
        if (referenceToe.hasNaN())
        {
            LOG_ERROR("initSolesPositions : referenceToe.hasNaN()");
            return;
        }

        int i = 0, j = 0;
        for (auto& sole : soles)
        {
            j = 0;
            hull = get<0>(sole->ComputeConvexHull());

            if (hull->IsEmpty())
            {
                LOG_WARN("initSolesPositions : sole: onvex hull is empty)");
                continue;
            }

            leaveVisibleMesh(hull);

            if (hull->IsEmpty())
            {
                LOG_WARN("initSolesPositions : visible convex hull is empty)");
                continue;
            }

            LOG_TRACE("initSolesPositions: sole hull num points visible = %d", int(hull->vertices_.size()));
         
            int correspondenceSetSize = 0;
            shared_ptr<geometry::PointCloud> fittedSole;

            for (auto& triangleData : getBiggestTriangles(hull, 3)) 
            {
                auto floorTriangle = get<0>(triangleData);
                auto normal = get<1>(triangleData);

                // fix normal direction
                if (normal.dot(referenceNormal) < 0.0)
                {
                    LOG_TRACE("initSolesPositions: sole normal inverted to the refference vector");
                    normal *= -1.0;
                }

                auto currentSole = make_shared<geometry::PointCloud>(geometry::PointCloud(*sole));
                Eigen::Vector3d soleHeel = getSoleHeel(floorTriangle);

                LOG_DEBUG("initSolesPositions : normal  = %.3f  %.3f  %.3f ", normal[0], normal[1], normal[2]);
                LOG_DEBUG("initSolesPositions : soleHeel  = %.3f  %.3f  %.3f ", soleHeel[0], soleHeel[1], soleHeel[2]);

                if (soleHeel.hasNaN())
                {
                    LOG_WARN("initSolesPositions :soleHeel.hasNaN()");
                    continue;
                }

                alignGeometryByPointAndVector(currentSole, referenceHeel, soleHeel, referenceNormal, normal);
                
                Line line = Line(referenceHeel, referenceHeel);
                Eigen::Vector3d soleToe = getLegToe(currentSole, line);

                if (soleToe.hasNaN())
                {
                    LOG_WARN("initSolesPositions :soleToe.hasNaN()");
                    continue;
                }

                alignGeometryByPointAndVector(currentSole, referenceHeel, referenceHeel, referenceToe - referenceHeel, soleToe - referenceHeel);
                float threshold = 0.02;
                auto registrationResult = pipelines::registration::RegistrationICP(
                    *currentSole, *referenceSole, threshold, Eigen::MatrixXd::Identity(4, 4),
                    pipelines::registration::TransformationEstimationPointToPlane());

                LOG_DEBUG("initSolesPositions : registration fittness  = %.3f  ", registrationResult.fitness_);
                LOG_DEBUG("initSolesPositions : registration set size  = %d  ", int(registrationResult.correspondence_set_.size()));

                if (registrationResult.correspondence_set_.size() > correspondenceSetSize)
                {
                    fittedSole = make_shared<geometry::PointCloud>(currentSole->Transform(registrationResult.transformation_));
                    correspondenceSetSize = registrationResult.correspondence_set_.size();
                }

                auto currentIterationCombinedSole = make_shared<geometry::PointCloud>(geometry::PointCloud(*referenceSole));
                for (int k = 0; k < i; k++) 
                {
                    *currentIterationCombinedSole += *soles[k];
                }
                *currentIterationCombinedSole += *currentSole;
                if (save_logs)
                {
                    LOG_TRACE("initSolesPositions : save sole registration results  %d - %d", i, j);
                    io::WritePointCloud(logPath + "/logs_soles_" + to_string(i) + "_" + to_string(j) + ".ply", *currentIterationCombinedSole);
                }

                j++;
            }
            i++;
            sole = fittedSole;
        }
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    void alignSoleWithLeg(
        std::shared_ptr<open3d::geometry::PointCloud>& sole,
        std::shared_ptr<open3d::geometry::PointCloud>& leg, 
        Plane& floor) 
    {
        using namespace std;
        using namespace open3d;

        if (sole->IsEmpty() || leg->IsEmpty())
        {
            LOG_ERROR("alignSoleWithLeg : sole->IsEmpty() || leg->IsEmpty()");
            return;
        }
        
        auto hull = get<0>(sole->ComputeConvexHull());
        hull->OrientTriangles();

        LOG_TRACE("alignSoleWithLeg: hull num points = %d", int(hull->vertices_.size()));
        leaveVisibleMesh(hull);

        LOG_TRACE("alignSoleWithLeg: hull num points visible = %d", int(hull->vertices_.size()));

        if (hull->IsEmpty())
        {
            LOG_ERROR("alignSoleWithLeg : hull->IsEmpty()");
            return;
        }

        Eigen::Vector3d floor_plane_normal = floor.getNormal();
        LOG_DEBUG("alignSoleWithLeg : floor_plane_normal  = %.3f  %.3f  %.3f ", floor_plane_normal[0], floor_plane_normal[1], floor_plane_normal[2]);

        vector<Eigen::Vector3d> floorTriangle;
        Eigen::Vector3d floorNormal;
        tie(floorTriangle, floorNormal) = getBiggestTriangle(hull);

        if (floor_plane_normal.dot(floorNormal) > 0.0) // Sole normal shoul be opposite
        {
            LOG_DEBUG("alignSoleWithLeg: sole normal vector invertion ");
            floorNormal *= -1.0;

            // invert hull normals
            for (auto& it : hull->triangle_normals_)
            {
                it *= -1;
            }
        }

        auto solePlane = Plane(floorTriangle);
        solePlane.setNormal(floorNormal);

        Eigen::Vector3d soleHeel = getSoleHeel(floorTriangle);
        Eigen::Vector3d legHeel = getLegHeel(leg, floor);

        LOG_DEBUG("alignSoleWithLeg : floorTriangle v1 = %.3f  %.3f  %.3f ", floorTriangle[0][0], floorTriangle[0][1], floorTriangle[0][2]);
        LOG_DEBUG("alignSoleWithLeg : floorTriangle v2 = %.3f  %.3f  %.3f ", floorTriangle[1][0], floorTriangle[1][1], floorTriangle[1][2]);
        LOG_DEBUG("alignSoleWithLeg : floorTriangle v3 = %.3f  %.3f  %.3f ", floorTriangle[2][0], floorTriangle[2][1], floorTriangle[2][2]);
        LOG_DEBUG("alignSoleWithLeg : referenceNormal  = %.3f  %.3f  %.3f ", floorNormal[0], floorNormal[1], floorNormal[2]);
        LOG_DEBUG("alignSoleWithLeg : soleHeel  = %.3f  %.3f  %.3f ", soleHeel[0], soleHeel[1], soleHeel[2]);
        LOG_DEBUG("alignSoleWithLeg : legHeel  = %.3f  %.3f  %.3f ", legHeel[0], legHeel[1], legHeel[2]);

        if (soleHeel.hasNaN() || legHeel.hasNaN())
        {
            LOG_ERROR("alignSoleWithLeg : soleHeel.hasNaN() || legHeel.hasNaN()");
            return;
        }

        vector<shared_ptr<geometry::Geometry3D>> hullAndSole{ hull, sole };

        alignGeometriesByPointAndVector(hullAndSole, legHeel, soleHeel, floor.getNormal(), solePlane.getNormal());
        auto axis = getLegAxis(leg, floor);

        LOG_TRACE("alignSoleWithLeg: compute LegToe for sole");
        Eigen::Vector3d soleToe = getLegToe(sole, axis);

        LOG_TRACE("alignSoleWithLeg: compute LegToe for leg");
        Eigen::Vector3d legToe = getLegToe(leg, axis);

        if (soleToe.hasNaN() || legToe.hasNaN())
        {
            LOG_ERROR("alignSoleWithLeg : soleToe.hasNaN() || legToe.hasNaN()");
            return;
        }

        alignGeometriesByPointAndVector(hullAndSole, legHeel, legHeel, legToe - legHeel, soleToe - legHeel);

        tie(floorTriangle, floorNormal) = getBiggestTriangle(hull);

        solePlane = Plane(floorTriangle);
        solePlane.setNormal(floorNormal);

        LOG_DEBUG("alignSoleWithLeg : floorTriangle v1 = %.3f  %.3f  %.3f ", floorTriangle[0][0], floorTriangle[0][1], floorTriangle[0][2]);
        LOG_DEBUG("alignSoleWithLeg : floorTriangle v2 = %.3f  %.3f  %.3f ", floorTriangle[1][0], floorTriangle[1][1], floorTriangle[1][2]);
        LOG_DEBUG("alignSoleWithLeg : floorTriangle v3 = %.3f  %.3f  %.3f ", floorTriangle[2][0], floorTriangle[2][1], floorTriangle[2][2]);
        LOG_DEBUG("alignSoleWithLeg : referenceNormal  = %.3f  %.3f  %.3f ", floorNormal[0], floorNormal[1], floorNormal[2]);

        alignGeometriesByPointAndVector(hullAndSole, legHeel, legHeel, floor.getNormal(), solePlane.getNormal());

        LOG_TRACE("alignSoleWithLeg : run getMeshBoundaries");
        //auto soleSilhouettePoints = getMeshBoundaries(hull, 0.002); //looks like there bug with edges direction
        auto soleSilhouettePoints = getMeshBoundariesAlt(hull, 0.002);
        
        LOG_DEBUG("alignSoleWithLeg : soleSilhouettePoints num points =  %d", int(soleSilhouettePoints.size()));

        vector<Eigen::Vector3d> soleSilhouette;
        for (const auto& point : soleSilhouettePoints) 
        {
            soleSilhouette.push_back(point - floor.signedDistanceFromPoint(point) * floor.getNormal());
        }

        auto soleSilhouettePCD = make_shared<geometry::PointCloud>(soleSilhouette);
        auto legSilhouette = getLegContour(leg, floor, 0.015);

        LOG_DEBUG("alignSoleWithLeg : legSilhouette num points =  %d", int(legSilhouette.size()));

        auto legSilhouettePCD = make_shared<geometry::PointCloud>(legSilhouette);

        sole->Translate(legSilhouettePCD->GetCenter() - soleSilhouettePCD->GetCenter());
        soleSilhouettePCD->Translate(legSilhouettePCD->GetCenter() - soleSilhouettePCD->GetCenter());

        double threshold = 0.03;
        auto regP2P = pipelines::registration::RegistrationICP(
            *soleSilhouettePCD, *legSilhouettePCD, threshold, Eigen::MatrixXd::Identity(4, 4),
            pipelines::registration::TransformationEstimationPointToPoint()
        );

        LOG_DEBUG("alignSoleWithLeg : registration ICP fittness  = %.5f  ", regP2P.fitness_);
        LOG_DEBUG("alignSoleWithLeg : registration ICP set size  = %d  ", int(regP2P.correspondence_set_.size()));
        
        Eigen::Matrix4d T = regP2P.transformation_;
        LOG_DEBUG("alignSoleWithLeg : T = %.3f  %.3f  %.3f %.3f  %.3f  %.3f %.3f  %.3f  %.3f  %.3f  %.3f %.3f  %.3f  %.3f %.3f  %.3f ",
            T(0, 0), T(0, 1), T(0, 2), T(0, 3), 
            T(1, 0), T(1, 1), T(1, 2), T(1, 3), 
            T(2, 0), T(2, 1), T(2, 2), T(2, 3), 
            T(3, 0), T(3, 1), T(3, 2), T(3, 3));

        sole->Transform(regP2P.transformation_);
        sole->Translate(floor.getNormal() * 0.02);
    }

    void alignSidesWithSole(
        std::shared_ptr<open3d::geometry::PointCloud>& sole, 
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& sides, 
        Plane& dividingPlane, Plane& floor, double shift)
    {
        using namespace std;
        using namespace open3d;

        if (sole->IsEmpty() || sides.size() < 2)
        {
            LOG_ERROR("alignSidesWithSole : sole->IsEmpty() || sides.size() < 2");
            return;
        }

        LOG_TRACE("alignSidesWithSole: num side : %d", int(sides.size()));

        Eigen::Vector3d floor_plane_normal = floor.getNormal();

        LOG_DEBUG("alignSidesWithSole : floor_plane_normal  = %.3f  %.3f  %.3f ", floor_plane_normal[0], floor_plane_normal[1], floor_plane_normal[2]);

        double threshold = 0.05;

        vector<Eigen::Vector3d> soleProjection;
        for (const auto& point : sole->points_)
        {
            if (floor.signedDistanceFromPoint(point) > 0)
                soleProjection.push_back(point - floor.signedDistanceFromPoint(point) * floor.getNormal());
        }

        auto soleProjectionPCD = make_shared<geometry::PointCloud>(soleProjection);
        soleProjectionPCD->PaintUniformColor({ 0.2, 0.7, 0.6 });

        auto soleSilhouettePCD = getFlatPointCloudContour(soleProjectionPCD);
        soleSilhouettePCD->PaintUniformColor({ 0.7, 0.2, 0.6 });

        if (soleSilhouettePCD->IsEmpty())
        {
            LOG_WARN("alignSidesWithSole : soleSilhouettePCD->IsEmpty()");
            return;
        }

        for (auto& side : sides)
        {
            auto Silhouette = getLegContour(side, floor, 0.005);

            if (Silhouette.empty())
            {
                LOG_WARN("alignSidesWithSole :Silhouette.empty()");
                continue;
            }

            auto SilhouettePCD = make_shared<geometry::PointCloud>(Silhouette);

            auto res = pipelines::registration::RegistrationICP(
                *SilhouettePCD, *soleSilhouettePCD, threshold, Eigen::MatrixXd::Identity(4, 4),
                pipelines::registration::TransformationEstimationPointToPoint());

            LOG_DEBUG("alignSidesWithSole : registration ICP fittness  = %.5f  ", res.fitness_);
            LOG_DEBUG("alignSidesWithSole : registration ICP set size  = %d  ", int(res.correspondence_set_.size()));

            side->Transform(res.transformation_);
        }

        /*auto hull = get<0>(sole->ComputeConvexHull());
        auto initialDirection = -floor.getNormal();
        auto initialViewPoint = sole->GetCenter() + floor.getNormal();
        auto initialCenter = sole->GetCenter();
        double initialDistance = 1;

        alignGeometryByPointAndVector(hull, { 0,0,1 }, initialCenter, { 0,0,1 }, initialDirection);
        
        leaveVisibleMesh(hull);

        alignGeometryByPointAndVector(hull, initialCenter, {0,0,1}, initialDirection, {0,0,1});*/
    }
}