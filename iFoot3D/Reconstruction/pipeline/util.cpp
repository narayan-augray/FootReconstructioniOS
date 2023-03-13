#include "util.h"

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <open3d/Open3D.h>

#include "logger.h"


namespace ifoot3d {

    bool splitToFloat(const std::string& line, std::vector<float>& data, const char& delimiter)
    {
        data.clear();
        if (line.empty())
        {
            LOG_WARN("splitToFloat: line.empty()");
            return false;
        }

        std::stringstream sstr(line);
        while (sstr.good())
        {
            std::string substr;
            std::getline(sstr, substr, char(delimiter));
            data.push_back(stof(substr));
        }
        return true;
    }

    bool parseFloatData(const std::vector<std::string>& lines, std::vector<float>& data, const char& delimiter)
    {
        data.clear();
        if (lines.empty())
        {
            LOG_ERROR("parseFloatData: lines.empty()");
            return false;
        }
        
        for (const auto& line : lines)
        {
            std::vector<float> numbers;
            splitToFloat(line, numbers, delimiter);
            data.insert(data.end(), numbers.begin(), numbers.end());
        }
        return true;
    }

    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> separateCloudForClusters(
        std::shared_ptr<open3d::geometry::PointCloud> pcd, 
        const std::vector<int> & labels) 
    {
        if (pcd->IsEmpty() )
        {
            LOG_ERROR("separateCloudForClusters : pcd->IsEmpty()");
            return { std::vector<std::shared_ptr<open3d::geometry::PointCloud>>() };
        }
        
        if (labels.empty())
        {
            LOG_ERROR("separateCloudForClusters : labels.empty()");
            return { std::vector<std::shared_ptr<open3d::geometry::PointCloud>>() };
        }

        auto points = pcd->points_;
        auto colors = pcd->colors_;
        auto num_clusters = *std::max_element(std::begin(labels), std::end(labels)) + 1;
        
        std::vector<std::vector<Eigen::Vector3d>> clusters (num_clusters);
        std::vector<std::vector<Eigen::Vector3d>> clusterColors(num_clusters);
        for (int i = 0; i < labels.size(); i++)
        {
            int label = labels[i];
            if (label == -1)
            {
                //std::cout << "ifoot3d::separateCloudForClusters exception  label == -1" << std::endl;
                continue;
            }

            clusters[label].push_back(points[i]);
            clusterColors[label].push_back(colors[i]);
        }

        // remove empty clusters
        int i, k;
        for (i = k = 0; i < clusters.size(); ++i)
        {
            if (!clusters[i].empty())
            {
                clusters[k] = clusters[i];
                clusterColors[k++] = clusterColors[i];
            }
        }

        clusters.resize(k);
        clusterColors.resize(k);

        std::vector<std::shared_ptr<open3d::geometry::PointCloud>> clusterClouds;
        for (int i = 0; i < num_clusters; i++) 
        {
            clusterClouds.push_back(std::make_shared<open3d::geometry::PointCloud>(clusters[i]));
            clusterClouds[clusterClouds.size() - 1]->colors_ = clusterColors[i];
        }
        return clusterClouds;
    }

    Line::Line(Eigen::Vector3d vector, Eigen::Vector3d point) {
        this->vector = vector;
        this->point = point;
    }

    Eigen::Vector3d Line::getPoint() {
        return this->point;
    }

    Eigen::Vector3d Line::getVector() {
        return this->vector;
    }

    float Line::distanceFromPoint(const Eigen::Vector3d& point) {
        return (this->point - point).cross(this->vector).norm() / this->vector.norm();
    }

    Eigen::Vector3d Line::pointProjection(const Eigen::Vector3d & point) {
        return this->point + this->vector * this->vector.dot(point - this->point);
    }

    Plane::Plane() {
        this->points = std::vector<Eigen::Vector3d>();
        this->normal = Eigen::Vector3d();
    }

    Plane::Plane(std::vector<Eigen::Vector3d> points) {
        this->points = points;
        this->normal = this->getNormal();
    }

    Plane::Plane(Eigen::Vector3d point, Eigen::Vector3d normal) {
        this->points = std::vector<Eigen::Vector3d>({ point });
        this->normal = normal;
        Eigen::Vector3d point1 = point + Eigen::Vector3d(normal.array() * Eigen::Vector3d({ 0.5,1,1 }).array()) + Eigen::Vector3d({1, 0, 0});
        point1 = point1 - this->signedDistanceFromPoint(point1) * normal;
        Eigen::Vector3d point2 = point + Eigen::Vector3d(normal.array() * Eigen::Vector3d({ 1,0.5,1 }).array()) + Eigen::Vector3d({ 0, 1, 0 });
        point2 = point2 - this->signedDistanceFromPoint(point2) * normal;
        this->points.push_back(point1);
        this->points.push_back(point2);
    }

    std::shared_ptr<open3d::geometry::PointCloud> Plane::getPointCloud(
        float size, 
        int density, 
        Eigen::Vector3d& point) 
    {
        using namespace std;
        using namespace open3d;

        vector<Eigen::Vector3d> points;
        for (int i = 0; i <= density; i++) {
            for (int j = 0; j <= density; j++) {
                points.push_back({(double) i / density * size,(double) j / density * size, 0.0});
            }
        }
        auto pcd = make_shared<geometry::PointCloud>(points);

        pcd->EstimateNormals();
        pcd->OrientNormalsToAlignWithDirection({0,0,-1});
        alignGeometryByPointAndVector(pcd, point, { size / 2, size / 2, 0 }, this->normal, { 0,0,1 });
        return pcd;
    }

    std::vector<Eigen::Vector3d> Plane::getPoints() {
        return this->points;
    }

    Eigen::Vector3d Plane::getNormal() {
        if (&this->normal == NULL) {
            return (this->points[0] - this->points[1]).cross(this->points[0] - this->points[2]) / (this->points[0] - this->points[1]).cross(this->points[0] - this->points[2]).norm();
        }
        else {
            return this->normal;
        }
    }

    void Plane::setNormal(Eigen::Vector3d normal) {
        this->normal = normal;
    }

    double Plane::signedDistanceFromPoint(const Eigen::Vector3d& point) {
        double nx = this->normal[0];
        double ny = this->normal[1];
        double nz = this->normal[2];
        double x = this->points[0][0];
        double y = this->points[0][1];
        double z = this->points[0][2];

        if (nx == 0.0 && ny == 0.0 && nz == 0.0)
        {
            LOG_ERROR("signedDistanceFromPoint : ZERO normal");
            return 0;
        }

        return (nx * point[0] + ny * point[1] + nz * point[2] - nx * x - ny * y - nz * z) / std::sqrt(
            nx * nx + ny * ny + nz * nz);
    }

    double Plane::distanceFromPoint(const Eigen::Vector3d& point) {
        double nx = this->normal[0];
        double ny = this->normal[1];
        double nz = this->normal[2];
        double x = this->points[0][0];
        double y = this->points[0][1];
        double z = this->points[0][2];
        return std::abs((nx * point[0] + ny * point[1] + nz * point[2] - nx * x - ny * y - nz * z) / std::sqrt(
            nx * nx + ny * ny + nz * nz));
    }

    Line getLegAxis(const std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor)
    {
        if (leg->IsEmpty())
        {
            LOG_ERROR("getLegAxis : leg->IsEmpty()");
            return Line();
        }

        using MyPair = std::pair<int, double>;
        std::vector <MyPair> indices;
        const auto& points = leg->points_;
        for (int i = 0; i < points.size(); ++i) 
        {
            indices.emplace_back(i, floor.distanceFromPoint(points[i]));
        }
        sort(indices.begin(), indices.end(), [](const MyPair& p1, const MyPair& p2)->bool {return p1.second > p2.second; });

        int numberOfTopPoints = points.size() > 300 ? 300 : std::floor(points.size() / 5);

        if (numberOfTopPoints == 0)
        {
            LOG_ERROR("getLegAxis : numberOfTopPoints == 0");
            return Line();
        }

        Eigen::Vector3d topAveragePoint({ 0,0,0 });
        for (int i = 0; i < numberOfTopPoints; i++)
        {
            topAveragePoint += points[indices[i].first];
        }
        topAveragePoint /= numberOfTopPoints;

        Eigen::Vector3d normal = floor.getNormal();
        LOG_DEBUG("getLegAxis : normal = %.3f  %.3f  %.3f ", normal[0], normal[1], normal[2]);
        LOG_DEBUG("getLegAxis : topAveragePoint = %.3f  %.3f  %.3f ", topAveragePoint[0], topAveragePoint[1], topAveragePoint[2]);
        return Line(floor.getNormal(), topAveragePoint);
    }

    Eigen::Vector3d getLegToe(const std::shared_ptr<open3d::geometry::PointCloud>& leg, Line& axis)
    {
        if (leg->IsEmpty())
        {
            LOG_ERROR("getLegToe : leg->IsEmpty()");
            return Eigen::Vector3d(NAN, NAN, NAN);
        }
        int idx = std::max_element(leg->points_.begin(), leg->points_.end(), [&axis](Eigen::Vector3d point1, Eigen::Vector3d point2) {return axis.distanceFromPoint(point1) < axis.distanceFromPoint(point2); }) - leg->points_.begin();
        
        Eigen::Vector3d pt = leg->points_[idx];
        LOG_DEBUG("getLegToe : point = %.3f  %.3f  %.3f ", pt[0], pt[1], pt[2]);

        return pt;
    }

    Eigen::Vector3d getLegHeel(const std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor)
    {
        if (leg->IsEmpty())
        {
            LOG_ERROR("getLegHeel : leg->IsEmpty()");
            return Eigen::Vector3d(NAN, NAN, NAN);
        }

        auto legAxis = getLegAxis(leg, floor);
        return legAxis.getPoint() + floor.getNormal() * floor.distanceFromPoint(legAxis.getPoint());
    }

    Eigen::Vector3d getSoleHeel(const std::vector<Eigen::Vector3d>& soleTriangle) 
    {
        using namespace std;

        if (soleTriangle.empty())
        {
            LOG_ERROR("getSoleHeel : soleTriangle.empty()");
            return Eigen::Vector3d(NAN, NAN, NAN);
        }

        vector<vector<int>> edges{ {0,1}, {1, 2}, {0,2} };
        sort(edges.begin(), edges.end(), [&soleTriangle](vector<int> edge1, vector<int> edge2) {return (soleTriangle[edge1[0]] - soleTriangle[edge1[1]]).norm() > (soleTriangle[edge2[0]] - soleTriangle[edge2[1]]).norm(); });
        if (edges[0][0] == edges[1][0] || edges[0][0] == edges[1][1]) {
            return soleTriangle[edges[0][0]];
        }
        else {
            return soleTriangle[edges[0][1]];
        }
    }

    float getAngleBetweenVectors(const Eigen::Vector3d& v1, const Eigen::Vector3d& v2) 
    {
        double len1 = v1.norm();
        double len2 = v2.norm();

        if (len1 == 0.0 || len2 == 0.0)
        {
            LOG_ERROR("getAngleBetweenVectors : len1 == 0.0 || len2 == 0.0");
            return 0;
        }

        return std::acos(v1.dot(v2) / v1.norm() / v2.norm());
    }

    void repairFloorNormals(
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legs,
        std::vector<Plane>& floors)
    {
        if (legs.empty() || floors.empty() || legs.size() != floors.size())
        {
            LOG_ERROR("repairFloorNormals : legs.empty() || floors.empty() || legs.size() != floors.size()");
            return;
        }

        for (int i = 0; i < legs.size(); i++)
        {
            if (floors[i].signedDistanceFromPoint(getLegAxis(legs[i], floors[i]).getPoint()) > 0)
            {
                LOG_TRACE("repairFloorNormals : normal inverted");
                floors[i].setNormal(-floors[i].getNormal());
            }
        }
    }

    void alignGeometryByPointAndVector(
        std::shared_ptr<open3d::geometry::PointCloud>& geometry, 
        const Eigen::Vector3d& targetPoint, 
        const Eigen::Vector3d& sourcePoint, 
        const Eigen::Vector3d& targetDirection, 
        const Eigen::Vector3d& sourceDirection)
    {
        if (geometry->IsEmpty())
        {
            LOG_ERROR("alignGeometryByPointAndVector : geometry->IsEmpty()");
            return;
        }

        if (targetDirection.norm() < 1e-8 || sourceDirection.norm() < 1e-8)
        {
            LOG_ERROR("alignGeometryByPointAndVector : targetDirection.norm() < 1e-8 || sourceDirection.norm() < 1e-8");
            return;
        }

        double angle = -getAngleBetweenVectors(targetDirection, sourceDirection);

        Eigen::Vector3d axis = targetDirection.cross(sourceDirection);

        axis /= axis.norm();

        auto R = open3d::geometry::Geometry3D::GetRotationMatrixFromAxisAngle(angle * axis);
        geometry->Translate(targetPoint - sourcePoint);
        geometry->Rotate(R, targetPoint);
        LOG_DEBUG("alignGeometryByPointAndVector : angle = %.3f ", angle);
        LOG_DEBUG("alignGeometryByPointAndVector : axis = %.3f  %.3f  %.3f ", axis[0], axis[1], axis[2]);
        LOG_DEBUG("alignGeometryByPointAndVector : R = %.3f  %.3f  %.3f %.3f  %.3f  %.3f %.3f  %.3f  %.3f ", 
            R(0, 0), R(0, 1), R(0, 2), R(1, 0), R(1, 1), R(1, 2), R(2, 0), R(2, 1), R(2, 2));
    }

    void alignGeometryByPointAndVector(
        std::shared_ptr<open3d::geometry::TriangleMesh>& geometry, 
        const Eigen::Vector3d& targetPoint, 
        const Eigen::Vector3d& sourcePoint, 
        const Eigen::Vector3d& targetDirection,
        const Eigen::Vector3d& sourceDirection) {

        if (geometry->IsEmpty())
        {
            LOG_ERROR("alignGeometryByPointAndVector : geometry->IsEmpty()");
            return;
        }

        if (targetDirection.norm() < 1e-8 || sourceDirection.norm() < 1e-8)
        {
            LOG_ERROR("alignGeometryByPointAndVector : targetDirection.norm() < 1e-8 || sourceDirection.norm() < 1e-8");
            return;
        }
        double angle = -getAngleBetweenVectors(targetDirection, sourceDirection);

        Eigen::Vector3d axis = targetDirection.cross(sourceDirection);
        axis /= axis.norm();

        auto R = open3d::geometry::Geometry3D::GetRotationMatrixFromAxisAngle(angle * axis);
        geometry->Translate(targetPoint - sourcePoint);
        geometry->Rotate(R, targetPoint);

        LOG_DEBUG("alignGeometryByPointAndVector : angle = %.3f ", angle);
        LOG_DEBUG("alignGeometryByPointAndVector : axis = %.3f  %.3f  %.3f ", axis[0], axis[1], axis[2]);
        LOG_DEBUG("alignGeometryByPointAndVector : R = %.3f  %.3f  %.3f %.3f  %.3f  %.3f %.3f  %.3f  %.3f ",
            R(0, 0), R(0, 1), R(0, 2), R(1, 0), R(1, 1), R(1, 2), R(2, 0), R(2, 1), R(2, 2));
    }

    void alignGeometriesByPointAndVector(
        std::vector<std::shared_ptr<open3d::geometry::Geometry3D>>& geometries, 
        const Eigen::Vector3d& targetPoint, 
        const Eigen::Vector3d& sourcePoint, 
        const Eigen::Vector3d& targetDirection, 
        const Eigen::Vector3d& sourceDirection) 
    {
        if (geometries.empty())
        {
            LOG_ERROR("alignGeometriesByPointAndVector : geometries.empty()");
            return;
        }

        if (targetDirection.norm() < 1e-8 || sourceDirection.norm() < 1e-8)
        {
            LOG_ERROR("alignGeometryByPointAndVector : targetDirection.norm() < 1e-8 || sourceDirection.norm() < 1e-8");
            return;
        }

        double angle = -getAngleBetweenVectors(targetDirection, sourceDirection);
        
        Eigen::Vector3d axis = targetDirection.cross(sourceDirection);
        axis /= axis.norm();

        auto R = open3d::geometry::Geometry3D::GetRotationMatrixFromAxisAngle(angle * axis);
        for (auto geometry : geometries)
        {
            geometry->Translate(targetPoint - sourcePoint);
            geometry->Rotate(R, targetPoint);
        }

        LOG_DEBUG("alignGeometriesByPointAndVector : angle = %.3f ", angle);
        LOG_DEBUG("alignGeometriesByPointAndVector : axis = %.3f  %.3f  %.3f ", axis[0], axis[1], axis[2]);
        LOG_DEBUG("alignGeometriesByPointAndVector : R = %.3f  %.3f  %.3f %.3f  %.3f  %.3f %.3f  %.3f  %.3f ",
            R(0, 0), R(0, 1), R(0, 2), R(1, 0), R(1, 1), R(1, 2), R(2, 0), R(2, 1), R(2, 2));
    }

    Eigen::Vector3d rotateVector(
        const Eigen::Vector3d& vector, 
        const Eigen::Vector3d& targetDirection, 
        const Eigen::Vector3d& sourceDirection)
    {
        auto angle = -getAngleBetweenVectors(sourceDirection, targetDirection);
        auto axis = targetDirection.cross(sourceDirection) / targetDirection.norm() / sourceDirection.norm();
        auto R = open3d::geometry::PointCloud::GetRotationMatrixFromAxisAngle(angle * axis);
        return R * vector;
    }

    float sign(Eigen::Vector2d p1, Eigen::Vector2d p2, Eigen::Vector2d p3)
    {
        return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1]);
    }

    bool pointInTriangle(Eigen::Vector2d pt, Eigen::Vector2d v1, Eigen::Vector2d v2, Eigen::Vector2d v3)
    {
        float d1, d2, d3;
        bool has_neg, has_pos;

        d1 = sign(pt, v1, v2);
        d2 = sign(pt, v2, v3);
        d3 = sign(pt, v3, v1);

        has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(has_neg && has_pos);
    }

    Eigen::Vector3d planeLineIntersection(
        Eigen::Vector3d planePoint,
        Eigen::Vector3d planeNormal, 
        Eigen::Vector3d linePoint, 
        Eigen::Vector3d lineNormal) 
    {
        double t = planeNormal.dot(planePoint - linePoint) / planeNormal.dot(lineNormal);
        return Eigen::Vector3d({ lineNormal[0] * t + linePoint[0], lineNormal[1] * t + linePoint[1], lineNormal[2] * t + linePoint[2] });
    }

    void leaveVisibleMesh(std::shared_ptr<open3d::geometry::TriangleMesh>& mesh, Eigen::Vector3d cameraPos) 
    {
        using namespace std;

        if (mesh->IsEmpty())
        {
            LOG_ERROR("leaveVisibleMesh : mesh->IsEmpty()");
            return;
        }

        mesh->ComputeVertexNormals();
        mesh->ComputeTriangleNormals();
        
        const auto& triangles = mesh->triangles_;
        const auto& vertices = mesh->vertices_;

        if (triangles.empty() || vertices.empty())
        {
            LOG_ERROR("leaveVisibleMesh : triangles.empty() || vertices.empty()");
            return;
        }

        vector<Eigen::Vector2d> centersPlane, verticesPlane;
        
        Eigen::Vector3d center;
        // project centers of triangles on plane
        for (const auto& triangle : triangles)
        {
            center = (vertices[triangle[0]] + vertices[triangle[1]] + vertices[triangle[2]]) / 3;
            centersPlane.push_back({ center[0] / center[2], center[1] / center[2] });
        }
        // project vertices on plane
        for (const auto& vertice : vertices) 
        {
            verticesPlane.push_back({vertice[0]/vertice[2], vertice[1]/vertice[2]});
        }
        
        vector<Eigen::Vector3i> visibleTriangles;
        vector<Eigen::Vector3d> visibleNormals;
        for (int i = 0; i < centersPlane.size(); i++) 
        {     
            Eigen::Vector3d currentTriangleNormal = (vertices[triangles[i][0]] - vertices[triangles[i][1]]).cross(vertices[triangles[i][0]] - vertices[triangles[i][2]]);
            currentTriangleNormal /= currentTriangleNormal.norm();
            Eigen::Vector3d currentCenter = (vertices[triangles[i][0]] + vertices[triangles[i][1]] + vertices[triangles[i][2]]) / 3;
            // find the second triangle to which the center belongs
            for (int j = 0; j < triangles.size(); j++)
            {
                if (i == j)
                    continue;
                if (pointInTriangle(centersPlane[i], verticesPlane[triangles[j][0]], verticesPlane[triangles[j][1]], verticesPlane[triangles[j][2]])) {
                    Eigen::Vector3d secondTriangleNormal = (vertices[triangles[j][0]] - vertices[triangles[j][1]]).cross(vertices[triangles[j][0]] - vertices[triangles[j][2]]);
                    secondTriangleNormal /= secondTriangleNormal.norm();
                    auto currentTriangleIntersection = planeLineIntersection(vertices[triangles[i][0]], currentTriangleNormal, cameraPos, currentCenter);
                    auto secondTriangleIntersection = planeLineIntersection(vertices[triangles[j][0]], secondTriangleNormal, cameraPos, currentCenter);
                    // if the intersection with the second triangle is further - it is visible
                    if(secondTriangleIntersection.squaredNorm() > currentTriangleIntersection.squaredNorm()) {
                        visibleTriangles.push_back(triangles[i]);
                        visibleNormals.push_back(mesh->triangle_normals_[i]);
                        break;
                    }
                }
            }
        }
        mesh->triangles_ = visibleTriangles;
        mesh->triangle_normals_ = visibleNormals;
    }


    double triangleArea(const std::vector<Eigen::Vector3d>& vertices) {
        auto a = (vertices[0] - vertices[1]).norm();
        auto b = (vertices[1] - vertices[2]).norm();
        auto c = (vertices[0] - vertices[2]).norm();
        double s = (a + b + c) / 2;
        return std::sqrt(s * (s - a) * (s - b) * (s - c));
    }

    std::tuple<std::vector<Eigen::Vector3d>, Eigen::Vector3d> getBiggestTriangle(
        std::shared_ptr<open3d::geometry::TriangleMesh>& mesh) 
    {
        if (mesh->IsEmpty())
        {
            LOG_ERROR("getBiggestTriangle : mesh->IsEmpty()");
            return {std::vector<Eigen::Vector3d>(), Eigen::Vector3d()};
        }

        using MyPair = std::pair<int, double>;
        std::vector <MyPair> indices;
        const auto& triangles = mesh->triangles_;
        const auto& vertices = mesh->vertices_;

        for (int i = 0; i < triangles.size(); ++i) 
        {
            indices.emplace_back(i, triangleArea({vertices[triangles[i][0]], vertices[triangles[i][1]], vertices[triangles[i][2]] }));
        }
        auto biggest = *std::max_element(indices.begin(), indices.end(), [](const MyPair& p1, const MyPair& p2)->bool {return p1.second < p2.second; });
        int id = biggest.first;
        return { {vertices[triangles[id][0]], vertices[triangles[id][1]], vertices[triangles[id][2]]}, mesh->triangle_normals_[id] };
    }

    std::vector<std::tuple<std::vector<Eigen::Vector3d>, Eigen::Vector3d>> getBiggestTriangles(
        std::shared_ptr<open3d::geometry::TriangleMesh>& mesh,
        int num) 
    {
        if (mesh->IsEmpty())
        {
            LOG_ERROR("getBiggestTriangles : mesh->IsEmpty()");
            return std::vector<std::tuple<std::vector<Eigen::Vector3d>, Eigen::Vector3d>>();
        }

        using MyPair = std::pair<int, double>;
        std::vector <MyPair> indices;
        const auto& triangles = mesh->triangles_;
        const auto& vertices = mesh->vertices_;
        for (int i = 0; i < triangles.size(); ++i)
        {
            indices.emplace_back(i, triangleArea({ vertices[triangles[i][0]], vertices[triangles[i][1]], vertices[triangles[i][2]] }));
        }
        std::sort(indices.begin(), indices.end(), [](const MyPair& p1, const MyPair& p2)->bool {return p1.second > p2.second; });
        std::vector<std::tuple<std::vector<Eigen::Vector3d>, Eigen::Vector3d>> res;

        num = (std::min)(num, int(indices.size()));
        for (int i = 0; i < num; i++)
        {
            int id = indices[i].first;
            res.push_back({{ vertices[triangles[id][0]], vertices[triangles[id][1]], vertices[triangles[id][2]] }, mesh->triangle_normals_[id]});
        }
        return res;
    }

    int numberOfEdgeInstancesInMesh(const std::vector<int>& edge, const std::vector<Eigen::Vector3i>& triangles)
    {
        if (edge.empty() || triangles.empty())
        {
            LOG_ERROR("numberOfEdgeInstancesInMesh : edge.empty() || triangles.empty()");
            return 0;
        }

        using namespace std;
        using namespace open3d;
        
        int res = 0;
        for (const auto& triangle : triangles) 
        {
            vector<vector<int>> triangleEdges = {
                {triangle[0], triangle[1]},
                {triangle[0], triangle[2]},
                {triangle[1], triangle[2]}
                };
            for (const auto& triangleEdge : triangleEdges)
            {
                if (edge[0] == triangleEdge[0] && edge[1] == triangleEdge[1] ||
                    edge[0] == triangleEdge[1] && edge[1] == triangleEdge[0]) 
                {
                    res++;
                }
            }
        }
        return res;
    }

    std::vector<Eigen::Vector3d> getMeshBoundaries(
        std::shared_ptr<open3d::geometry::TriangleMesh> mesh, 
        double stepLength=0.002) 
    {
        using namespace std;
        using namespace open3d;

        if (mesh->IsEmpty())
        {
            LOG_ERROR("getMeshBoundaries : mesh->IsEmpty()");
            return std::vector<Eigen::Vector3d>();
        }

        const auto& points = mesh->vertices_;
        const auto& triangles = mesh->triangles_;

        vector<vector<int>> boundaryEdges;
        vector<Eigen::Vector3d> boundaryPoints;
        
        for (const auto& triangle : triangles)
        {
            vector<vector<int>> edges = {
                {triangle[0], triangle[1]},
                {triangle[0], triangle[2]},
                {triangle[1], triangle[2]}
            };
            for (const auto& edge : edges)
            {
                if (numberOfEdgeInstancesInMesh(edge, triangles) == 1)
                {
                    boundaryEdges.push_back(edge);
                    const auto p1 = points[edge[0]];
                    const auto p2 = points[edge[1]];
                    boundaryPoints.push_back(p1);
                    boundaryPoints.push_back(p2);
                    if ((p1-p2).norm() > 0.01)
                    {
                        auto step = (p2 - p1) / (p2 - p1).norm() * stepLength;
                        for (int i = 1; i < floor((p2 - p1).norm() / stepLength); i++) 
                        {
                            boundaryPoints.push_back(p1 + i * step);
                        }
                    }
                }
            }
        }
        return boundaryPoints;
    }

    //--------------------------------------------------------

    std::vector<Eigen::Vector3d> getMeshBoundariesAlt(
        std::shared_ptr<open3d::geometry::TriangleMesh> mesh,
        double stepLength = 0.002)
    {
        using namespace std;
        using namespace open3d;

        if (mesh->IsEmpty())
        {
            LOG_ERROR("getMeshBoundariesAlt : mesh->IsEmpty()");
            return std::vector<Eigen::Vector3d>();
        }

        struct comp_vec2i
        {
            bool operator () (Eigen::Vector2i const& a, Eigen::Vector2i const& b) const
            {
                return (a[0] < b[0]) || (a[0] == b[0] && a[1] < b[1]);
            }
        };

        const auto& points = mesh->vertices_;
        const auto& triangles = mesh->triangles_;

        int vi1, vi2, vi3;
        std::map<Eigen::Vector2i, std::vector<int>, comp_vec2i> face_edges_connectivity;

        for (int i = 0; i < triangles.size(); ++i)
        {
            vi1 = triangles[i][0];
            vi2 = triangles[i][1];
            vi3 = triangles[i][2];

            face_edges_connectivity[Eigen::Vector2i((std::min)(vi1, vi2), (std::max)(vi1, vi2))].push_back(i);
            face_edges_connectivity[Eigen::Vector2i((std::min)(vi2, vi3), (std::max)(vi2, vi3))].push_back(i);
            face_edges_connectivity[Eigen::Vector2i((std::min)(vi1, vi3), (std::max)(vi1, vi3))].push_back(i);
        }

        // get bound points
        vector<Eigen::Vector3d> boundaryPoints;

        for (const auto& it : face_edges_connectivity)
        {
            if (it.second.size() == 1)
            {
                const auto p1 = points[it.first[0]];
                const auto p2 = points[it.first[1]];
                boundaryPoints.push_back(p1);
                boundaryPoints.push_back(p2);

                if ((p1 - p2).norm() > 0.01)
                {
                    auto step = (p2 - p1) / (p2 - p1).norm() * stepLength;
                    for (int i = 1; i < floor((p2 - p1).norm() / stepLength); i++)
                    {
                        boundaryPoints.push_back(p1 + i * step);
                    }
                }
            }
        }

        return boundaryPoints;
    }

    //--------------------------------------------------------

    std::vector<Eigen::Vector3d> getCloseToFloorPoints(
        std::shared_ptr<open3d::geometry::PointCloud>& cloud, 
        Plane& floor, double distance)
    {
        using namespace std;
        using namespace open3d;

        const auto& points = cloud->points_;

        if (points.empty())
        {
            LOG_ERROR("getCloseToFloorPoints : points.empty()");
            return std::vector<Eigen::Vector3d>();
        }

        vector<double> dists;
        for (const Eigen::Vector3d& p : points)
            dists.push_back(floor.distanceFromPoint(p));
        int closestIdx = min_element(dists.cbegin(), dists.cend()) - dists.cbegin();
        double minDistance = floor.distanceFromPoint(points[closestIdx]);
        vector<Eigen::Vector3d> closePoints;
        for (const auto& point : points) 
        {
            if (floor.distanceFromPoint(point) < minDistance + distance) 
            {
                closePoints.push_back(point);
            }
        }

        LOG_DEBUG("getCloseToFloorPoints : num points =  %d", int(closePoints.size()));

        return closePoints;
    }

    std::vector<size_t> getCloseToFloorPointsIndexes(std::shared_ptr<open3d::geometry::PointCloud>& cloud, Plane& floor, double distance) {
        using namespace std;

        const auto& points = cloud->points_;
        vector<double> dists;
        for (const Eigen::Vector3d& p : points)
            dists.push_back(floor.distanceFromPoint(p));
        int closestIdx = min_element(dists.cbegin(), dists.cend()) - dists.cbegin();
        double minDistance = floor.distanceFromPoint(points[closestIdx]);
        std::vector<size_t> indexes;
        for (size_t i = 0; i < points.size(); i++) 
        {
            if (floor.distanceFromPoint(points[i]) < minDistance + distance) {
                indexes.push_back(i);
            }
        }
        return indexes;
    }

    std::vector<Eigen::Vector3d> getLegContour(
        std::shared_ptr<open3d::geometry::PointCloud>& leg, 
        Plane& floor, double threshold) 
    {
        if (leg->IsEmpty())
        {
            LOG_ERROR("getLegContour : leg->IsEmpty()");
            return std::vector<Eigen::Vector3d>();
        }

        //const auto& points = leg->points_;

        auto closePoints = getCloseToFloorPoints(leg, floor, threshold);
        std::vector<Eigen::Vector3d> legSilhouette;
        for (const auto& point : closePoints) 
        {
            legSilhouette.push_back(point - floor.signedDistanceFromPoint(point) * floor.getNormal());
        }
        return legSilhouette;
    }

    Eigen::Matrix4d getReflectionMatrix(double a, double b, double c) {
        Eigen::Matrix4d reflectionMatrix;
        reflectionMatrix << 1 - 2 * a * a, -2 * a * b, -2 * a * c, 0,
            -2 * a * b, 1 - 2 * b * b, -2 * b * c, 0,
            -2 * a * c, -2 * b * c, 1 - 2 * c * c, 0,
            0, 0, 0, 1;
        return reflectionMatrix;
    }

    std::shared_ptr<open3d::geometry::PointCloud> getFlatPointCloudContour(
        std::shared_ptr<open3d::geometry::PointCloud> pcd)
    {
        using namespace std;

        if (pcd->IsEmpty())
        {
            LOG_ERROR("getFlatPointCloudContour : pcd->IsEmpty()");
            return std::shared_ptr<open3d::geometry::PointCloud>();
        }

        int divisions = 100;
        auto points = pcd->points_;
        auto xlowest = *min_element(points.begin(), points.end(), [](const Eigen::Vector3d p1, const Eigen::Vector3d p2) { return p1[0] < p2[0]; });
        auto xbiggest = *max_element(points.begin(), points.end(), [](const Eigen::Vector3d p1, const Eigen::Vector3d p2) { return p1[0] < p2[0]; });
        auto ylowest = *min_element(points.begin(), points.end(), [](const Eigen::Vector3d p1, const Eigen::Vector3d p2) { return p1[1] < p2[1]; });
        auto ybiggest = *max_element(points.begin(), points.end(), [](const Eigen::Vector3d p1, const Eigen::Vector3d p2) { return p1[1] < p2[1]; });
        // let's slice the pcd and find end points
        vector<Eigen::Vector3d> startPointsY(divisions, ybiggest);
        vector<Eigen::Vector3d> endPointsY(divisions, ylowest);
        for (const auto& point : points) 
        {
            // todo : check original code out of bounds 
            int division = floor((point[0] - xlowest[0]) / (xbiggest[0] - xlowest[0]) * divisions);
            
            division = (std::max)((std::min)(divisions - 1, division), 0);

            if (point[1] > endPointsY[division][1])
            {
                endPointsY[division] = point;
            }
            if (point[1] < startPointsY[division][1])
            {
                startPointsY[division] = point;
            }
        }
        
        vector<Eigen::Vector3d> startPointsX(divisions, xbiggest);
        vector<Eigen::Vector3d> endPointsX(divisions, xlowest);
        for (const auto& point : points) {
            int division = floor((point[1] - ylowest[1]) / (ybiggest[1] - ylowest[1]) * divisions);
            division = (std::max)((std::min)(divisions - 1, division), 0);

            if (point[0] > endPointsX[division][0]) {
                endPointsX[division] = point;
            }
            if (point[0] < startPointsX[division][0]) {
                startPointsX[division] = point;
            }
        }
        startPointsY.insert(startPointsY.end(), endPointsY.begin(), endPointsY.end());
        startPointsY.insert(startPointsY.end(), endPointsX.begin(), endPointsX.end());
        startPointsY.insert(startPointsY.end(), startPointsX.begin(), startPointsX.end());
        auto contourPCD = make_shared<open3d::geometry::PointCloud>(startPointsY);
        return contourPCD;
    }
}
