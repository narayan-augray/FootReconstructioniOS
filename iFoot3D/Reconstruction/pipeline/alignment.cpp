#include "alignment.h"

#include "open3d/Open3D.h"
#include <Eigen/Dense>
#include "util.h"

namespace ifoot3d {
    void initLegsPositions(
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legs, 
        std::vector<Plane>& floors) 
    {
        if (legs.empty() || floors.empty() || legs.size() != floors.size())
        {
            std::cout << "ifoot3d::initLegsPositions: legs.empty() || floors.empty() || legs.size() != floors.size()" << std::endl;
            return;
        }

        repairFloorNormals(legs, floors);
        auto referenceLeg = legs[0];

        if (referenceLeg->IsEmpty())
        {
            std::cout << "ifoot3d::initLegsPositions: referenceLeg->IsEmpty()" << std::endl;
            return;
        }

        auto referenceFloor = floors[0];

        auto referenceAxis = getLegAxis(referenceLeg, referenceFloor);
        Eigen::Vector3d referenceToe = getLegToe(referenceLeg, referenceAxis);
        Eigen::Vector3d ReferenceToeFloorProjection = referenceToe - referenceFloor.signedDistanceFromPoint(referenceToe) * referenceFloor.getNormal();
        Eigen::Vector3d referenceHeel = getLegHeel(referenceLeg, referenceFloor);
        
        for (int i = 1; i < legs.size(); i++) 
        {
            auto currentLegAxis = getLegAxis(legs[i], floors[i]);
            alignGeometryByPointAndVector(legs[i], referenceAxis.getPoint(), currentLegAxis.getPoint(), referenceAxis.getVector(), currentLegAxis.getVector());
            
            Eigen::Vector3d currentToe = getLegToe(legs[i], referenceAxis);
            
            if (currentToe.hasNaN())
            {
                std::cout << "ifoot3d::initLegsPositions: currentToe.hasNaN()" << std::endl;
                continue;
            }

            Eigen::Vector3d currentToeFloorProjection = currentToe - referenceFloor.signedDistanceFromPoint(currentToe) * referenceFloor.getNormal();
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
            std::cout << "initSolesPositions :soles.empty()" << std::endl;
            return;
        }

        if (referenceSole->IsEmpty())
        {
            std::cout << "initSolesPositions :referenceSole->IsEmpty()" << std::endl;
            return;
        }

        bool save_logs = !logPath.empty();

        Eigen::Vector3d cameraPos{ 0, 0, 0 };

        auto hull = get<0>(referenceSole->ComputeConvexHull());
        
        if (hull->IsEmpty())
        {
            std::cout << "initSolesPositions : convex hull for refference sole is empty" << std::endl;
            return;
        }

        leaveVisibleMesh(hull);

        if (hull->IsEmpty())
        {
            std::cout << "initSolesPositions : convex hull is empty after leaveVisibleMesh" << std::endl;
            return;
        }
        
        auto biggestTriangleAndNormal = getBiggestTriangle(hull);

        auto referenceTriangle = get<0>(biggestTriangleAndNormal);
        auto referenceNormal = get<1>(biggestTriangleAndNormal);
        
        if (referenceTriangle.empty())
        {
            std::cout << "initSolesPositions : referenceTriangle.empty()" << std::endl;
            return;
        }

        Eigen::Vector3d referenceHeel = getSoleHeel(referenceTriangle);

        if (referenceHeel.hasNaN())
        {
            std::cout << "initSolesPositions :referenceHeel.hasNaN()" << std::endl;
            return;
        }

        Line legAxis = Line(referenceNormal, referenceHeel);

        Eigen::Vector3d referenceToe = getLegToe(referenceSole, legAxis);
        
        if (referenceToe.hasNaN())
        {
            std::cout << "initSolesPositions :referenceToe.hasNaN()" << std::endl;
            return;
        }

        int i = 0, j = 0;
        for (auto& sole : soles)
        {
            j = 0;
            hull = get<0>(sole->ComputeConvexHull());

            if (hull->IsEmpty())
            {
                std::cout << "initSolesPositions:sole: convex hull is empty" << std::endl;
                continue;
            }

            leaveVisibleMesh(hull);

            if (hull->IsEmpty())
            {
                std::cout << "initSolesPositions:sole: visible convex hull is empty" << std::endl;
                continue;
            }
            
            int correspondenceSetSize = 0;
            shared_ptr<geometry::PointCloud> fittedSole;

            for (auto& triangleData : getBiggestTriangles(hull, 3)) 
            {
                auto floorTriangle = get<0>(triangleData);
                auto normal = get<1>(triangleData);
                auto currentSole = make_shared<geometry::PointCloud>(geometry::PointCloud(*sole));
                Eigen::Vector3d soleHeel = getSoleHeel(floorTriangle);

                if (soleHeel.hasNaN())
                {
                    std::cout << "initSolesPositions :soleHeel.hasNaN()" << std::endl;
                    continue;
                }

                alignGeometryByPointAndVector(currentSole, referenceHeel, soleHeel, referenceNormal, normal);
                
                Line line = Line(referenceHeel, referenceHeel);
                Eigen::Vector3d soleToe = getLegToe(currentSole, line);

                if (soleToe.hasNaN())
                {
                    std::cout << "initSolesPositions :soleToe.hasNaN()" << std::endl;
                    continue;
                }


                alignGeometryByPointAndVector(currentSole, referenceHeel, referenceHeel, referenceToe - referenceHeel, soleToe - referenceHeel);
                float threshold = 0.02;
                auto registrationResult = pipelines::registration::RegistrationICP(
                    *currentSole, *referenceSole, threshold, Eigen::MatrixXd::Identity(4, 4),
                    pipelines::registration::TransformationEstimationPointToPlane());

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
                    io::WritePointCloud(logPath + "/logs_soles_" + to_string(i) + "_" + to_string(j) + ".pcd", *currentIterationCombinedSole);
                }

                j++;
            }
            i++;
            sole = fittedSole;
        }
    }


    void alignSoleWithLeg(
        std::shared_ptr<open3d::geometry::PointCloud>& sole,
        std::shared_ptr<open3d::geometry::PointCloud>& leg, 
        Plane& floor) 
    {
        using namespace std;
        using namespace open3d;

        if (sole->IsEmpty() || leg->IsEmpty())
        {
            std::cout << "ifoot3d::alignSoleWithLeg: sole->IsEmpty()|| leg->IsEmpty()" << std::endl;
            return;
        }


        Eigen::Vector3d cameraPos{ 0, 0, 0 };
        
        auto hull = get<0>(sole->ComputeConvexHull());
        leaveVisibleMesh(hull);

        if (hull->IsEmpty())
        {
            std::cout << "ifoot3d::alignSoleWithLeg: hull->IsEmpty()" << std::endl;
            return;
        }

        vector<Eigen::Vector3d> floorTriangle;
        Eigen::Vector3d floorNormal;
        tie(floorTriangle, floorNormal) = getBiggestTriangle(hull);
        auto solePlane = Plane(floorTriangle);
        solePlane.setNormal(floorNormal);

        Eigen::Vector3d soleHeel = getSoleHeel(floorTriangle);
        Eigen::Vector3d legHeel = getLegHeel(leg, floor);

        if (soleHeel.hasNaN() || legHeel.hasNaN())
        {
            std::cout << "ifoot3d::alignSoleWithLeg: soleHeel.hasNaN() || legHeel.hasNaN()" << std::endl;
            return;
        }

        vector<shared_ptr<geometry::Geometry3D>> hullAndSole{ hull, sole };

        alignGeometriesByPointAndVector(hullAndSole, legHeel, soleHeel, floor.getNormal(), solePlane.getNormal());
        auto axis = getLegAxis(leg, floor);
        Eigen::Vector3d soleToe = getLegToe(sole, axis);
        Eigen::Vector3d legToe = getLegToe(leg, axis);

        if (soleToe.hasNaN() || legToe.hasNaN())
        {
            std::cout << "ifoot3d::alignSoleWithLeg: soleToe.hasNaN() || legToe.hasNaN()" << std::endl;
            return;
        }
        alignGeometriesByPointAndVector(hullAndSole, legHeel, legHeel, legToe - legHeel, soleToe - legHeel);
        tie(floorTriangle, floorNormal) = getBiggestTriangle(hull);
        solePlane = Plane(floorTriangle);
        solePlane.setNormal(floorNormal);
        alignGeometriesByPointAndVector(hullAndSole, legHeel, legHeel, floor.getNormal(), solePlane.getNormal());
        auto soleSilhouettePoints = getMeshBoundaries(hull, 0.002);
        vector<Eigen::Vector3d> soleSilhouette;
        for (const auto& point : soleSilhouettePoints) 
        {
            soleSilhouette.push_back(point - floor.signedDistanceFromPoint(point) * floor.getNormal());
        }
        auto soleSilhouettePCD = make_shared<geometry::PointCloud>(soleSilhouette);
        auto legSilhouette = getLegContour(leg, floor, 0.015);
        auto legSilhouettePCD = make_shared<geometry::PointCloud>(legSilhouette);

        sole->Translate(legSilhouettePCD->GetCenter() - soleSilhouettePCD->GetCenter());
        soleSilhouettePCD->Translate(legSilhouettePCD->GetCenter() - soleSilhouettePCD->GetCenter());
        double threshold = 0.03;
        auto regP2P = pipelines::registration::RegistrationICP(
            *soleSilhouettePCD, *legSilhouettePCD, threshold, Eigen::MatrixXd::Identity(4, 4),
            pipelines::registration::TransformationEstimationPointToPoint()
        );
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
            std::cout << "alignSidesWithSole : sole->IsEmpty() || sides.empty()" << std::endl;
            return;
        }

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
            std::cout << "alignSidesWithSole : soleSilhouettePCD->IsEmpty()" << std::endl;
            return;
        }

        for (auto& side : sides)
        {
            auto Silhouette = getLegContour(side, floor, 0.005);
            auto SilhouettePCD = make_shared<geometry::PointCloud>(Silhouette);

            side->Transform(pipelines::registration::RegistrationICP(
                *SilhouettePCD, *soleSilhouettePCD, threshold, Eigen::MatrixXd::Identity(4, 4),
                pipelines::registration::TransformationEstimationPointToPoint()
            ).transformation_);
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