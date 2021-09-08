/*******************************************************************************
 * CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
 * Copyright (C), IGG Group, ICube, University of Strasbourg, France            *
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU Lesser General Public License as published by the *
 * Free Software Foundation; either version 2.1 of the License, or (at your     *
 * option) any later version.                                                   *
 *                                                                              *
 * This library is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
 * for more details.                                                            *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this library; if not, write to the Free Software Foundation,      *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
 *                                                                              *
 * Web site: http://cgogn.unistra.fr/                                           *
 * Contact information: cgogn@unistra.fr                                        *
 *                                                                              *
 *******************************************************************************/

#ifndef CGOGN_MODELING_ALGOS_SKELETON_H_
#define CGOGN_MODELING_ALGOS_SKELETON_H_

#include <cgogn/core/functions/mesh_ops/edge.h>
#include <cgogn/core/functions/traversals/global.h>
#include <cgogn/core/functions/traversals/halfedge.h>
#include <cgogn/core/functions/traversals/vertex.h>
#include <cgogn/core/types/mesh_traits.h>

#include <cgogn/geometry/algos/angle.h>
#include <cgogn/geometry/algos/length.h>
#include <cgogn/geometry/algos/normal.h>
#include <cgogn/geometry/types/vector_traits.h>

#include <cgogn/modeling/algos/medial_axis.h>
#include <cgogn/modeling/algos/remeshing/pliant_remeshing.h>

#include <Eigen/Sparse>

#include <algorithm>
#include <array>
#include <vector>

namespace cgogn
{

namespace modeling
{

using Vec3 = geometry::Vec3;
using Scalar = geometry::Scalar;

///////////
// CMap2 //
///////////

inline void cut_incident_faces(CMap2& m, CMap2::Vertex v)
{
	std::vector<CMap2::Face> ifaces = incident_faces(m, v);
	for (CMap2::Face f : ifaces)
		cut_face(m, CMap2::Vertex(f.dart), CMap2::Vertex(phi<11>(m, f.dart)));
}

inline CMap2::Vertex opposite_vertex(CMap2& m, CMap2::HalfEdge he)
{
	return CMap2::Vertex(phi_1(m, he.dart));
}

inline std::vector<CMap2::Vertex> opposite_vertices(CMap2& m, CMap2::Edge e)
{
	return {CMap2::Vertex(phi_1(m, e.dart)), CMap2::Vertex(phi_1(m, phi2(m, e.dart)))};
}

// inline void compute_halfedges_opposite_angle(CMap2& m, CMap2::Face f, CMap2::Attribute<Vec3>* vertex_position,
// 											 CMap2::Attribute<Scalar>* halfedge_opposite_angle)
// {
// 	Scalar zero_threshold = 1e-5;

// 	Dart ha = f.dart;
// 	Dart hb = phi1(m, ha);
// 	Dart hc = phi1(m, hb);
// 	Scalar a = geometry::length(m, CMap2::Edge(ha), vertex_position);
// 	Scalar a2 = a * a;
// 	Scalar b = geometry::length(m, CMap2::Edge(hb), vertex_position);
// 	Scalar b2 = b * b;
// 	Scalar c = geometry::length(m, CMap2::Edge(hc), vertex_position);
// 	Scalar c2 = c * c;
// 	if (a < zero_threshold || b < zero_threshold || c < zero_threshold)
// 	{
// 		value<Scalar>(m, halfedge_opposite_angle, CMap2::HalfEdge(ha)) = -1;
// 		value<Scalar>(m, halfedge_opposite_angle, CMap2::HalfEdge(hb)) = -1;
// 		value<Scalar>(m, halfedge_opposite_angle, CMap2::HalfEdge(hc)) = -1;
// 	}
// 	else
// 	{
// 		/// Opposite angles (from law of cosines)
// 		value<Scalar>(m, halfedge_opposite_angle, CMap2::HalfEdge(ha)) =
// 			acos(std::clamp((-a2 + b2 + c2) / (2 * b * c), -1.0, 1.0));
// 		value<Scalar>(m, halfedge_opposite_angle, CMap2::HalfEdge(hb)) =
// 			acos(std::clamp((+a2 - b2 + c2) / (2 * a * c), -1.0, 1.0));
// 		value<Scalar>(m, halfedge_opposite_angle, CMap2::HalfEdge(hc)) =
// 			acos(std::clamp((+a2 + b2 - c2) / (2 * a * b), -1.0, 1.0));
// 	}
// }

/////////////
// GENERIC //
/////////////

template <typename MESH>
void mean_curvature_skeleton(MESH& m, typename mesh_traits<MESH>::template Attribute<Vec3>* vertex_position,
							 typename mesh_traits<MESH>::template Attribute<Vec3>* vertex_medial_point)
{
	static_assert(mesh_traits<MESH>::dimension == 2, "MESH dimension should be 2");

	using Vertex = typename mesh_traits<MESH>::Vertex;
	using Edge = typename mesh_traits<MESH>::Edge;
	using Face = typename mesh_traits<MESH>::Face;
	using HalfEdge = typename mesh_traits<MESH>::HalfEdge;

	// std::vector<Vec3> medial_points;
	// medial_points.reserve(nb_cells<Vertex>(m));
	// foreach_cell(m, [&](Vertex v) -> bool {
	// 	medial_points.push_back(value<Vec3>(m, vertex_medial_point, v));
	// 	return true;
	// });
	// acc::KDTree<3, uint32>* medial_points_kdt = new acc::KDTree<3, uint32>(medial_points);

	Vec3 bb_min, bb_max;
	for (uint32 i = 0; i < 2; ++i)
	{
		bb_min[i] = std::numeric_limits<float64>::max();
		bb_max[i] = std::numeric_limits<float64>::lowest();
	}
	for (const Vec3& p : *vertex_position)
	{
		for (uint32 i = 0; i < 3; ++i)
		{
			if (p[i] < bb_min[i])
				bb_min[i] = p[i];
			if (p[i] > bb_max[i])
				bb_max[i] = p[i];
		}
	}
	Scalar bb_diag = (bb_max - bb_min).norm();

	Scalar wL = 1.0;
	Scalar wH = 0.11;
	Scalar wM = 0.17;
	Scalar edge_collapse_threshold = 0.004 * bb_diag;
	Scalar zero_threshold = 1e-5;

	auto vertex_index = add_attribute<uint32, Vertex>(m, "__vertex_index");
	auto edge_weight = add_attribute<Scalar, Edge>(m, "__edge_weight");

	for (uint32 i = 0; i < 10; ++i)
	{
		uint32 nb_vertices = 0;
		foreach_cell(m, [&](Vertex v) -> bool {
			value<uint32>(m, vertex_index, v) = nb_vertices++;
			return true;
		});

		foreach_cell(m, [&](Edge e) -> bool {
			std::vector<Scalar> angles = geometry::opposite_angles(m, e, vertex_position);
			Scalar weight = 0.0;
			for (Scalar a : angles)
				weight += 1.0 / std::tan(std::clamp(a, Scalar(-0.99), Scalar(0.99)));
			// if (weight < 0.0)
			// 	weight = 0.0;
			weight = std::clamp(weight, 0.0, 1.0);
			value<Scalar>(m, edge_weight, e) = weight;
			return true;
		});

		std::vector<Eigen::Triplet<Scalar>> Acoeffs;
		Acoeffs.reserve(nb_vertices * 10);

		// smooth
		foreach_cell(m, [&](Edge e) -> bool {
			Scalar w = value<Scalar>(m, edge_weight, e);
			auto iv = incident_vertices(m, e);
			uint32 vidx1 = value<uint32>(m, vertex_index, iv[0]);
			uint32 vidx2 = value<uint32>(m, vertex_index, iv[1]);
			Acoeffs.push_back(Eigen::Triplet<Scalar>(int(vidx1), int(vidx2), w* wL));
			Acoeffs.push_back(Eigen::Triplet<Scalar>(int(vidx1), int(vidx1), -w* wL));
			Acoeffs.push_back(Eigen::Triplet<Scalar>(int(vidx2), int(vidx1), w* wL));
			Acoeffs.push_back(Eigen::Triplet<Scalar>(int(vidx2), int(vidx2), -w* wL));
			return true;
		});

		// velocity
		foreach_cell(m, [&](Vertex v) -> bool {
			uint32 vidx = value<uint32>(m, vertex_index, v);
			Acoeffs.push_back(Eigen::Triplet<Scalar>(int(nb_vertices + vidx), int(vidx), wH));
			return true;
		});

		// medial
		foreach_cell(m, [&](Vertex v) -> bool {
			uint32 vidx = value<uint32>(m, vertex_index, v);
			Acoeffs.push_back(Eigen::Triplet<Scalar>(int(2 * nb_vertices + vidx), int(vidx), wM));
			return true;
		});

		Eigen::SparseMatrix<Scalar, Eigen::ColMajor> A(3 * nb_vertices, nb_vertices);
		A.setFromTriplets(Acoeffs.begin(), Acoeffs.end());

		Eigen::MatrixXd x(nb_vertices, 3);
		Eigen::MatrixXd b(3 * nb_vertices, 3);

		foreach_cell(m, [&](Vertex v) -> bool {
			uint32 vidx = value<uint32>(m, vertex_index, v);
			b(vidx, 0) = 0;
			b(vidx, 1) = 0;
			b(vidx, 2) = 0;
			const Vec3& pos = value<Vec3>(m, vertex_position, v);
			b(nb_vertices + vidx, 0) = wH * pos[0];
			b(nb_vertices + vidx, 1) = wH * pos[1];
			b(nb_vertices + vidx, 2) = wH * pos[2];
			const Vec3& medp = value<Vec3>(m, vertex_medial_point, v);
			b(2 * nb_vertices + vidx, 0) = wM * medp[0];
			b(2 * nb_vertices + vidx, 1) = wM * medp[1];
			b(2 * nb_vertices + vidx, 2) = wM * medp[2];
			return true;
		});

		Eigen::SparseMatrix<Scalar, Eigen::ColMajor> At = A.transpose();
		Eigen::SimplicialLDLT<Eigen::SparseMatrix<Scalar, Eigen::ColMajor>> solver(At * A);
		x = solver.solve(At * b);

		foreach_cell(m, [&](Vertex v) -> bool {
			uint32 vidx = value<uint32>(m, vertex_index, v);
			Vec3& pos = value<Vec3>(m, vertex_position, v);
			pos[0] = x(vidx, 0);
			pos[1] = x(vidx, 1);
			pos[2] = x(vidx, 2);
			return true;
		});

		// std::cout << i << " - detect & mark degeneracies" << std::endl;
		// uint32 nb_fixed_vertices = 0;
		// foreach_cell(m, [&](Vertex v) -> bool {
		// 	if (value<bool>(m, vertex_is_fixed, v))
		// 		return true;
		// 	uint32 count = 0;
		// 	foreach_incident_edge(m, v, [&](Edge ie) -> bool {
		// 		Scalar l = geometry::length(m, ie, vertex_position);
		// 		if (l < edge_collapse_threshold / 5.0 && !edge_can_collapse(m, ie))
		// 			++count;
		// 		return true;
		// 	});
		// 	bool is_fixed = count > 1;
		// 	if (is_fixed)
		// 	{
		// 		value<bool>(m, vertex_is_fixed, v) = true;
		// 		value<Vec3>(m, vertex_is_fixed_color, v) = {1.0, 0.0, 0.0};
		// 		++nb_fixed_vertices;
		// 	}
		// 	else
		// 	{
		// 		value<bool>(m, vertex_is_fixed, v) = false;
		// 		value<Vec3>(m, vertex_is_fixed_color, v) = {0.0, 0.0, 0.0};
		// 	}
		// 	return true;
		// });
		// std::cout << i << " -   nb fixed vertices: " << nb_fixed_vertices << std::endl;

		std::cout << i << " - flip flat edges" << std::endl;
		uint32 nb_flip_edges = 0;
		bool has_flat_edge = false;
		do
		{
			has_flat_edge = false;
			foreach_cell(m, [&](Edge e) -> bool {
				std::vector<Vertex> iv = incident_vertices(m, e);
				if (degree(m, iv[0]) < 5 || degree(m, iv[1]) < 5)
					return true;

				std::vector<Scalar> op_angles = geometry::opposite_angles(m, e, vertex_position);
				Scalar flip_threshold_low = 140.0 * M_PI / 180.0;
				if (op_angles[0] > flip_threshold_low && op_angles[1] > flip_threshold_low)
				{
					if (edge_can_flip(m, e))
					{
						if (flip_edge(m, e))
						{
							has_flat_edge = true;
							++nb_flip_edges;
						}
					}
				}
				return true;
			});
		} while (has_flat_edge);
		std::cout << i << " -   nb flip edges: " << nb_flip_edges << std::endl;

		// std::cout << i << " - cut long edges" << std::endl;
		// uint32 nb_cut_edges = 0;
		// bool has_long_edge = false;
		// do
		// {
		// 	cache.template build<Edge>();
		// 	has_long_edge = false;
		// 	foreach_cell(cache, [&](Edge e) -> bool {
		// 		std::vector<Vertex> op_vertices = opposite_vertices(m, e);
		// 		std::vector<Scalar> op_angles = geometry::opposite_angles(m, e, vertex_position);
		// 		Scalar alpha0 = op_angles[0];
		// 		Scalar alpha1 = op_angles[1];
		// 		// std::vector<HalfEdge> ihe = incident_halfedges(m, e);
		// 		// Scalar alpha0 = value<Scalar>(m, halfedge_opposite_angle, ihe[0]);
		// 		// Scalar alpha1 = value<Scalar>(m, halfedge_opposite_angle, ihe[1]);
		// 		Scalar cut_threshold_low = 120.0 * M_PI / 180.0;
		// 		Scalar cut_threshold_high = 178.0 * M_PI / 180.0;

		// 		if (alpha0 < cut_threshold_low || alpha1 < cut_threshold_low || alpha0 > cut_threshold_high ||
		// 			alpha1 > cut_threshold_high)
		// 			return true;

		// 		has_long_edge = true;
		// 		std::vector<Vertex> iv = incident_vertices(m, e);
		// 		const Vec3& p0 = value<Vec3>(m, vertex_position, iv[0]);
		// 		const Vec3& p1 = value<Vec3>(m, vertex_position, iv[1]);

		// 		// uint32 largest = alpha0 > alpha1 ? 0 : 1;
		// 		// Vertex op_vertex = op_vertices[largest]; // opposite_vertex(m, ihe[largest]);
		// 		// Vec3 vec = (p1 - p0).normalized();
		// 		// Vec3 proj = value<Vec3>(m, vertex_position, op_vertex) - p0;
		// 		// Scalar t = vec.dot(proj);
		// 		// Vec3 newp = p0 + t * vec;
		// 		Vec3 newp = (p0 + p1) * 0.5;

		// 		const Vec3& mp0 = value<Vec3>(m, vertex_medial_point, iv[0]);
		// 		const Vec3& mp1 = value<Vec3>(m, vertex_medial_point, iv[1]);
		// 		// vec = (mp1 - mp0).normalized();
		// 		// Vec3 newmp = mp0 + t * vec;
		// 		Vec3 newmp = (mp0 + mp1) * 0.5;

		// 		Vertex cv = cut_edge(m, e);
		// 		cut_incident_faces(m, cv);
		// 		value<Vec3>(m, vertex_position, cv) = newp;
		// 		value<Vec3>(m, vertex_medial_point, cv) = newmp;
		// 		// std::pair<uint32, Scalar> k_res;
		// 		// medial_points_kdt->find_nn(mp, &k_res);
		// 		// value<Vec3>(m, vertex_medial_point, cv) = medial_points_kdt->vertex(k_res.first);

		// 		// value<bool>(m, vertex_is_split, cv) = true;
		// 		// value<bool>(m, vertex_is_fixed, cv) = false;

		// 		++nb_cut_edges;
		// 		return true;
		// 	});
		// } while (has_long_edge);
		// std::cout << i << " -   nb cut edges: " << nb_cut_edges << std::endl;
		// std::cout << std::boolalpha << "integrity: " << check_integrity(m) << std::endl;
		// std::cout << std::boolalpha << "triangular: " << is_simplicial(m) << std::endl;

		std::cout << i << " - collapse short edges" << std::endl;
		uint32 nb_collapsed_edges = 0;
		bool has_short_edge = false;
		do
		{
			has_short_edge = false;
			foreach_cell(m, [&](Edge e) -> bool {
				std::vector<Vertex> iv = incident_vertices(m, e);
				if (geometry::length(m, e, vertex_position) < edge_collapse_threshold)
				{
					if (edge_can_collapse(m, e))
					{
						has_short_edge = true;
						Vec3 newp =
							(value<Vec3>(m, vertex_position, iv[0]) + value<Vec3>(m, vertex_position, iv[1])) * 0.5;
						const Vec3& mp0 = value<Vec3>(m, vertex_medial_point, iv[0]);
						const Vec3& mp1 = value<Vec3>(m, vertex_medial_point, iv[1]);
						Scalar d0 = (mp0 - newp).squaredNorm();
						Scalar d1 = (mp1 - newp).squaredNorm();
						Vec3 newmp = d0 < d1 ? mp0 : mp1;
						Vertex cv = collapse_edge(m, e);
						value<Vec3>(m, vertex_position, cv) = newp;
						value<Vec3>(m, vertex_medial_point, cv) = newmp;
						// std::pair<uint32, Scalar> k_res;
						// medial_points_kdt->find_nn(mp, &k_res);
						// value<Vec3>(m, vertex_medial_point, cv) = medial_points_kdt->vertex(k_res.first);
						++nb_collapsed_edges;
					}
				}
				return true;
			});
		} while (has_short_edge);
		std::cout << i << " -   nb collapsed edges: " << nb_collapsed_edges << std::endl;

		// std::cout << i << " - detect & mark degeneracies" << std::endl;
		// nb_fixed_vertices = 0;
		// foreach_cell(m, [&](Vertex v) -> bool {
		// 	if (value<bool>(m, vertex_is_fixed, v))
		// 		return true;
		// 	uint32 count = 0;
		// 	foreach_incident_edge(m, v, [&](Edge ie) -> bool {
		// 		Scalar l = geometry::length(m, ie, vertex_position);
		// 		if (l < edge_collapse_threshold / 5.0 && !edge_can_collapse(m, ie))
		// 			++count;
		// 		return true;
		// 	});
		// 	bool is_fixed = count > 1;
		// 	if (is_fixed)
		// 	{
		// 		value<bool>(m, vertex_is_fixed, v) = true;
		// 		value<Vec3>(m, vertex_is_fixed_color, v) = {1.0, 0.0, 0.0};
		// 		++nb_fixed_vertices;
		// 	}
		// 	else
		// 	{
		// 		value<bool>(m, vertex_is_fixed, v) = false;
		// 		value<Vec3>(m, vertex_is_fixed_color, v) = {0.0, 0.0, 0.0};
		// 	}
		// 	return true;
		// });
		// std::cout << i << " -   nb fixed vertices: " << nb_fixed_vertices << std::endl;
	}

	remove_attribute<Vertex>(m, vertex_index);
	remove_attribute<Edge>(m, edge_weight);

	// delete medial_points_kdt;
}

} // namespace modeling

} // namespace cgogn

#endif // CGOGN_MODELING_ALGOS_SKELETON_H_
