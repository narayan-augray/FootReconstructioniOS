#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include <open3d/Open3D.h>


namespace ifoot3d {
	class Plane {
	private:
		std::vector<Eigen::Vector3d> points;
		Eigen::Vector3d normal;
	public:
		Plane();

		Plane(std::vector<Eigen::Vector3d> points);

		Plane(Eigen::Vector3d point, Eigen::Vector3d normal);
		
		std::vector<Eigen::Vector3d> getPoints();
		
		Eigen::Vector3d getNormal();

		void setNormal(Eigen::Vector3d);

		double signedDistanceFromPoint(const Eigen::Vector3d& point);

		double distanceFromPoint (const Eigen::Vector3d& point);
	};

	class Line {
	private:
		Eigen::Vector3d vector;
		Eigen::Vector3d point;
	public:
		Line(Eigen::Vector3d vector, Eigen::Vector3d point);

		Eigen::Vector3d getPoint();
		Eigen::Vector3d getVector();

		float distanceFromPoint(const Eigen::Vector3d& point);
		Eigen::Vector3d pointProjection(const Eigen::Vector3d& point);

	};

	std::vector<float> splitToFloat(std::string, const std::string);
	std::vector<float> parseFloatData(std::vector<std::string>& lines, const std::string delimiter);
	std::vector<std::shared_ptr<open3d::geometry::PointCloud>> separateCloudForClusters(std::shared_ptr<open3d::geometry::PointCloud> pcd, const std::vector<int>& labels);
	Line getLegAxis(const std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor);
	Eigen::Vector3d getLegToe(const std::shared_ptr<open3d::geometry::PointCloud>& leg, Line& axis);
	Eigen::Vector3d getLegHeel(const std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor);
	Eigen::Vector3d getSoleHeel(const std::vector<Eigen::Vector3d>& soleTriangle);
	float getAngleBetweenVectors(const Eigen::Vector3d&, const Eigen::Vector3d&);
	void repairFloorNormals(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legs, std::vector<Plane>& floors);
	Eigen::Vector3d rotateVector(const Eigen::Vector3d& vector, const Eigen::Vector3d& targetDirection, const Eigen::Vector3d& sourceDirection);
	void alignGeometriesByPointAndVector(std::vector<std::shared_ptr<open3d::geometry::Geometry3D>>& geometries,
                                         const Eigen::Vector3d& targetPoint,
                                         const Eigen::Vector3d& sourcePoint,
                                         const Eigen::Vector3d& targetDirection,
                                         const Eigen::Vector3d& sourceDirection);
	void leaveVisibleMesh(std::shared_ptr<open3d::geometry::TriangleMesh>&mesh);
	std::vector<std::tuple<std::vector<Eigen::Vector3d>, Eigen::Vector3d>> getBiggestTriangles(std::shared_ptr<open3d::geometry::TriangleMesh>& mesh, int num);
	std::tuple<std::vector<Eigen::Vector3d>, Eigen::Vector3d> getBiggestTriangle(std::shared_ptr<open3d::geometry::TriangleMesh>& mesh);
	std::vector<Eigen::Vector3d> getMeshBoundaries(std::shared_ptr<open3d::geometry::TriangleMesh> mesh, double stepLength);
	std::vector<Eigen::Vector3d> getCloseToFloorPoints(std::shared_ptr<open3d::geometry::PointCloud>& cloud, Plane& floor, double distance);
	std::vector<size_t> getCloseToFloorPointsIndexes(std::shared_ptr<open3d::geometry::PointCloud>& cloud, Plane& floor, double distance);
	std::vector<Eigen::Vector3d> getLegContour(std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor);
	Eigen::Matrix4d getReflectionMatrix(double a, double b, double c);
}
