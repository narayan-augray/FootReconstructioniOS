#include "alignment.h"

#include "open3d/Open3D.h"
#include <Eigen/Dense>
#include "util.h"

namespace ifoot3d {
    void initLegsPositions(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legs, std::vector<Plane>& floors) {
        repairFloorNormals(legs, floors);
        auto referenceLeg = legs[0];
        auto referenceFloor = floors[0];
        auto referenceAxis = getLegAxis(referenceLeg, referenceFloor);
        Eigen::Vector3d referenceToe = getLegToe(referenceLeg, referenceAxis);
        Eigen::Vector3d ReferenceToeFloorProjection = referenceToe - referenceFloor.signedDistanceFromPoint(referenceToe) * referenceFloor.getNormal();
        Eigen::Vector3d referenceHeel = getLegHeel(referenceLeg, referenceFloor);
        for (int i = 1; i < legs.size(); i++) {
            auto currentLegAxis = getLegAxis(legs[i], floors[i]);
            alignGeometryByPointAndVector(legs[i], referenceAxis.getPoint(), currentLegAxis.getPoint(), referenceAxis.getVector(), currentLegAxis.getVector());
            
            Eigen::Vector3d currentToe = getLegToe(legs[i], referenceAxis);
            
            Eigen::Vector3d currentToeFloorProjection = currentToe - referenceFloor.signedDistanceFromPoint(currentToe) * referenceFloor.getNormal();
            alignGeometryByPointAndVector(legs[i], referenceAxis.getPoint(), referenceAxis.getPoint(), ReferenceToeFloorProjection - referenceHeel, currentToeFloorProjection - referenceHeel);
            currentToe = getLegToe(legs[i], referenceAxis);
            legs[i]->Translate(referenceToe - currentToe);
        }
    }

    void initSolesPositions(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& soles, std::shared_ptr<open3d::geometry::PointCloud>& referenceSole) {
        using namespace std;
        using namespace open3d;
        
        Eigen::Vector3d cameraPos{ 0, 0, 0 };

        auto hull = get<0>(referenceSole->ComputeConvexHull());
        
        leaveVisibleMesh(hull);
        
        auto biggestTriangleAndNormal = getBiggestTriangle(hull);
        auto referenceTriangle = get<0>(biggestTriangleAndNormal);
        auto referenceNormal = get<1>(biggestTriangleAndNormal);
        
        Eigen::Vector3d referenceHeel = getSoleHeel(referenceTriangle);
        Line legAxis = Line(referenceNormal, referenceHeel);
        Eigen::Vector3d referenceToe = getLegToe(referenceSole, legAxis);
        
        for (auto& sole : soles) {

            hull = get<0>(sole->ComputeConvexHull());
            leaveVisibleMesh(hull);
            
            int correspondenceSetSize = 0;
            shared_ptr<geometry::PointCloud> fittedSole;
            for (auto& triangleData : getBiggestTriangles(hull, 3)) {
                auto floorTriangle = get<0>(triangleData);
                auto normal = get<1>(triangleData);
                auto currentSole = make_shared<geometry::PointCloud>(geometry::PointCloud(*sole));
                Eigen::Vector3d soleHeel = getSoleHeel(floorTriangle);

                alignGeometryByPointAndVector(currentSole, referenceHeel, soleHeel, referenceNormal, normal);
                
                Line line = Line(referenceHeel, referenceHeel);
                Eigen::Vector3d soleToe = getLegToe(currentSole, line);
                alignGeometryByPointAndVector(currentSole, referenceHeel, referenceHeel, referenceToe - referenceHeel, soleToe - referenceHeel);
                float threshold = 0.02;
                auto registrationResult = pipelines::registration::RegistrationICP(
                    *currentSole, *referenceSole, threshold, Eigen::MatrixXd::Identity(4, 4),
                    pipelines::registration::TransformationEstimationPointToPlane());
                if (registrationResult.correspondence_set_.size() > correspondenceSetSize) {
                    fittedSole = make_shared<geometry::PointCloud>(currentSole->Transform(registrationResult.transformation_));
                    correspondenceSetSize = registrationResult.correspondence_set_.size();
                }
            }
            sole = fittedSole;
        }
    }


    void alignSoleWithLeg(std::shared_ptr<open3d::geometry::PointCloud>& sole, std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor) {
        using namespace std;
        using namespace open3d;

        Eigen::Vector3d cameraPos{ 0, 0, 0 };
        
        auto hull = get<0>(sole->ComputeConvexHull());
        leaveVisibleMesh(hull);

        vector<Eigen::Vector3d> floorTriangle;
        Eigen::Vector3d floorNormal;
        tie(floorTriangle, floorNormal) = getBiggestTriangle(hull);
        auto solePlane = Plane(floorTriangle);
        solePlane.setNormal(floorNormal);

        Eigen::Vector3d soleHeel = getSoleHeel(floorTriangle);
        Eigen::Vector3d legHeel = getLegHeel(leg, floor);

        vector<shared_ptr<geometry::Geometry3D>> hullAndSole{ hull, sole };

        alignGeometriesByPointAndVector(hullAndSole, legHeel, soleHeel, floor.getNormal(), solePlane.getNormal());

        Line legAxis = getLegAxis(leg, floor);
        Eigen::Vector3d soleToe = getLegToe(sole, legAxis);
        Eigen::Vector3d legToe = getLegToe(leg, legAxis);
        alignGeometriesByPointAndVector(hullAndSole, legHeel, legHeel, legToe - legHeel, soleToe - legHeel);
        tie(floorTriangle, floorNormal) = getBiggestTriangle(hull);
        solePlane = Plane(floorTriangle);
        solePlane.setNormal(floorNormal);
        alignGeometriesByPointAndVector(hullAndSole, legHeel, legHeel, floor.getNormal(), solePlane.getNormal());
        auto soleSilhouettePoints = getMeshBoundaries(hull, 0.002);
        vector<Eigen::Vector3d> soleSilhouette;
        for (const auto& point : soleSilhouettePoints) {
            soleSilhouette.push_back(point - floor.signedDistanceFromPoint(point) * floor.getNormal());
        }
        auto soleSilhouettePCD = make_shared<geometry::PointCloud>(soleSilhouette);
        auto legSilhouette = getLegContour(leg, floor);
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
}
