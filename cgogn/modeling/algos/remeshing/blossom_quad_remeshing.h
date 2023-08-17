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

#ifndef CGOGN_MODELING_ALGOS_REMESHING_BLOSSOM_QUAD_REMESHING_H_
#define CGOGN_MODELING_ALGOS_REMESHING_BLOSSOM_QUAD_REMESHING_H_

#include <cgogn/core/types/maps/cmap/cmap2.h>

#include <cgogn/geometry/algos/length.h>
#include <cgogn/geometry/types/vector_traits.h>

#include <vector>

#include <Blossom5/PerfectMatching.h>
#include <libacc/bvh_tree.h>

namespace cgogn
{

struct MapBase;

namespace modeling
{

using geometry::Vec3;
using Mat3 = geometry::Mat3;
using geometry::Scalar;

///////////////
// MapBase:2 //
///////////////

template <typename MESH, typename std::enable_if_t<std::is_convertible_v<MESH&, MapBase&> &&
												   (mesh_traits<MESH>::dimension == 2)>* = nullptr>
std::array<typename MESH::Vertex, 4> edge_quad_vertices(MESH& m, typename MESH::Edge e)
{
	using Vertex = typename mesh_traits<MESH>::Vertex;

	std::array<Vertex, 4> eqv;
	eqv[0] = Vertex(phi1(m, e.dart_));
	eqv[1] = Vertex(phi_1(m, e.dart_));
	eqv[2] = Vertex(e.dart_);
	eqv[3] = Vertex(phi<2, -1>(m, e.dart_));

	return eqv;
}

template <typename MESH, typename std::enable_if_t<std::is_convertible_v<MESH&, MapBase&> &&
												   (mesh_traits<MESH>::dimension == 2)>* = nullptr>
std::vector<typename mesh_traits<MESH>::Edge> butterfly_neighbour_edges(MESH& m, typename mesh_traits<MESH>::Edge e)
{
	using Vertex = typename mesh_traits<MESH>::Vertex;
	using Edge = typename mesh_traits<MESH>::Edge;

	std::vector<Edge> edges;

	Dart d = e.dart_;
	if (!is_boundary(m, d))
	{
		Dart d12 = phi<1, 2>(m, d);
		if (!is_boundary(m, d12))
		{
			Dart d1212 = phi<1, 2>(m, d12);
			if (!is_boundary(m, d1212))
				edges.push_back(Edge(d1212));
			Dart d12_12 = phi<-1, 2>(m, d12);
			if (!is_boundary(m, d12_12))
				edges.push_back(Edge(d12_12));
		}
		Dart d_12 = phi<-1, 2>(m, d);
		if (!is_boundary(m, d_12))
		{
			Dart d_1212 = phi<1, 2>(m, d_12);
			if (!is_boundary(m, d_1212))
				edges.push_back(Edge(d_1212));
			Dart d_12_12 = phi<-1, 2>(m, d_12);
			if (!is_boundary(m, d_12_12))
				edges.push_back(Edge(d_12_12));
		}
	}
	Dart d2 = phi2(m, e.dart_);
	if (!is_boundary(m, d2))
	{
		Dart d212 = phi<1, 2>(m, d2);
		if (!is_boundary(m, d212))
		{
			Dart d21212 = phi<1, 2>(m, d212);
			if (!is_boundary(m, d21212))
				edges.push_back(Edge(d21212));
			Dart d212_12 = phi<-1, 2>(m, d212);
			if (!is_boundary(m, d212_12))
				edges.push_back(Edge(d212_12));
		}
		Dart d2_12 = phi<-1, 2>(m, d2);
		if (!is_boundary(m, d2_12))
		{
			Dart d2_1212 = phi<1, 2>(m, d2_12);
			if (!is_boundary(m, d2_1212))
				edges.push_back(Edge(d2_1212));
			Dart d2_12_12 = phi<-1, 2>(m, d2_12);
			if (!is_boundary(m, d2_12_12))
				edges.push_back(Edge(d2_12_12));
		}
	}

	return edges;
}

template <typename MESH, typename std::enable_if_t<std::is_convertible_v<MESH&, MapBase&> &&
												   (mesh_traits<MESH>::dimension == 2)>* = nullptr>
void cut_boundary_triangle(MESH& m, typename mesh_traits<MESH>::Edge e,
						   typename MESH::template Attribute<Vec3>* vertex_position)
{
	using Vertex = typename mesh_traits<MESH>::Vertex;
	using Edge = typename mesh_traits<MESH>::Edge;
	using Face = typename mesh_traits<MESH>::Face;

	Dart d = e.dart_;
	Dart d1 = phi1(m, d);
	Dart d_1 = phi_1(m, d);
	Vertex opposite_v(d_1);
	Vertex new_v = cut_edge(m, e);
	cut_face(m, new_v, opposite_v);
	value<Vec3>(m, vertex_position, new_v) =
		(value<Vec3>(m, vertex_position, Vertex(d)) + value<Vec3>(m, vertex_position, Vertex(d1))) / 2.0;
}

template <typename MESH, typename std::enable_if_t<std::is_convertible_v<MESH&, MapBase&> &&
												   (mesh_traits<MESH>::dimension == 2)>* = nullptr>
typename mesh_traits<MESH>::Face next_boundary_face(MESH& m, typename mesh_traits<MESH>::Edge e)
{
	return typename mesh_traits<MESH>::Face(phi<2, 1, 2>(m, e.dart_));
}

template <typename MESH, typename std::enable_if_t<std::is_convertible_v<MESH&, MapBase&> &&
												   (mesh_traits<MESH>::dimension == 2)>* = nullptr>
inline typename mesh_traits<MESH>::Face previous_boundary_face(MESH& m, typename mesh_traits<MESH>::Edge e)
{
	return typename mesh_traits<MESH>::Face(phi<2, -1, 2>(m, e.dart_));
}

template <typename MESH, typename std::enable_if_t<std::is_convertible_v<MESH&, MapBase&> &&
												   (mesh_traits<MESH>::dimension == 2)>* = nullptr>
inline bool common_edge(MESH& m, typename MESH::Face f1, typename mesh_traits<MESH>::Face f2,
						typename mesh_traits<MESH>::Edge& res)
{
	using Edge = typename mesh_traits<MESH>::Edge;

	std::vector<Dart> f1darts;
	foreach_dart_of_orbit(m, f1, [&](Dart d) -> bool {
		f1darts.push_back(d);
		return true;
	});
	bool found = false;
	foreach_dart_of_orbit(m, f2, [&](Dart d) -> bool {
		Dart d2 = phi2(m, d);
		if (std::find(f1darts.begin(), f1darts.end(), d2) != f1darts.end())
		{
			res = Edge(d);
			found = true;
		}
		return !found;
	});
	return found;
}

template <typename MESH, typename std::enable_if_t<std::is_convertible_v<MESH&, MapBase&> &&
												   (mesh_traits<MESH>::dimension == 2)>* = nullptr>
inline typename mesh_traits<MESH>::Vertex open_boundary_vertex(MESH& m, typename mesh_traits<MESH>::Vertex v)
{
	using Vertex = typename mesh_traits<MESH>::Vertex;
	using HalfEdge = typename mesh_traits<MESH>::HalfEdge;
	using Edge = typename mesh_traits<MESH>::Edge;
	using Face = typename mesh_traits<MESH>::Face;
	using Volume = typename mesh_traits<MESH>::Volume;
	using ParentVertex = typename mesh_traits<typename mesh_traits<MESH>::Parent>::Vertex;
	using ParentEdge = typename mesh_traits<typename mesh_traits<MESH>::Parent>::Edge;

	Dart d = is_boundary(m, v.dart_) ? phi<-1, 2>(m, v.dart_) : v.dart_;
	ParentVertex v1 = cut_edge(static_cast<typename mesh_traits<MESH>::Parent&>(m), ParentEdge(phi_1(m, d)), false);
	ParentVertex v2 =
		cut_edge(static_cast<typename mesh_traits<MESH>::Parent&>(m), ParentEdge(phi<2, 1, 2>(m, d)), false);
	phi2_sew(m, v1.dart_, v2.dart_);

	if (is_indexed<Vertex>(m))
	{
		set_index(m, Vertex(v1.dart_), new_index<Vertex>(m));
		copy_index<Vertex>(m, v2.dart_, d);
	}
	if (is_indexed<HalfEdge>(m))
	{
		set_index(m, HalfEdge(v1.dart_), new_index<HalfEdge>(m));
		set_index(m, HalfEdge(v2.dart_), new_index<HalfEdge>(m));
	}
	if (is_indexed<Edge>(m))
	{
		set_index(m, Edge(v1.dart_), new_index<Edge>(m));
	}
	if (is_indexed<Face>(m))
	{
		copy_index<Face>(m, v1.dart_, d);
		copy_index<Face>(m, v2.dart_, phi1(m, v2.dart_));
	}
	if (is_indexed<Volume>(m))
	{
		copy_index<Volume>(m, v1.dart_, d);
		copy_index<Volume>(m, v2.dart_, d);
	}

	return Vertex(v1.dart_);
}

/////////////
// GENERIC //
/////////////

inline Scalar quad_quality(const std::array<Vec3, 4>& points)
{
	std::array<Vec3, 4> edges;
	std::array<Scalar, 4> edgeSqrNorm;

	for (uint32 i = 0; i < 4; ++i)
	{
		uint32 j = (i + 1) % 4;
		edges[i] = points[j] - points[i];
		edgeSqrNorm[i] = edges[i].squaredNorm();
	}

	Vec3 u = edges[0].normalized();
	Vec3 w = (u.cross(-edges[3])).normalized();
	Vec3 v = w.cross(u);
	Mat3 m;
	m.row(0) = u;
	m.row(1) = v;
	m.row(2) = w;

	std::array<Vec3, 4> edges_frame;
	for (uint32 i = 0; i < 4; ++i)
		edges_frame[i] = m * edges[i];

	auto determinant = [](const Vec3& p1, const Vec3& p2) { return p1[0] * p2[1] - p1[1] * p2[0]; };

	std::array<Scalar, 4> detJ;
	for (uint32 i = 0; i < 4; ++i)
	{
		uint32 il = (i - 1 + 4) % 4;
		detJ[i] = determinant(edges_frame[i], -edges_frame[il]);
	}

	std::array<Scalar, 4> c;

	for (uint32 i = 0; i < 4; ++i)
	{
		uint32 il = (i - 1 + 4) % 4;
		c[i] = detJ[i] / (edgeSqrNorm[i] + edgeSqrNorm[il]);
	}

	auto detMin = std::min_element(detJ.begin(), detJ.end());

	if (*detMin > 0)
	{
		auto cMin = std::min_element(c.begin(), c.end());
		return 2 * *cMin;
	}
	else
		return *detMin;
}

template <typename MESH>
void greedy_quad_remeshing(MESH& m,
						   std::shared_ptr<typename mesh_traits<MESH>::template Attribute<Vec3>>& vertex_position)
{
	using Vertex = typename mesh_traits<MESH>::Vertex;
	using Edge = typename mesh_traits<MESH>::Edge;
	using Face = typename mesh_traits<MESH>::Face;

	struct prio_edge
	{
		Scalar w;
		Edge e;
	};

	uint32 nbe = nb_cells<Edge>(m);

	std::vector<prio_edge> edges;
	edges.reserve(nbe);

	foreach_cell(m, [&](Edge e) -> bool {
		if (is_incident_to_boundary(m, e))
			return true;

		std::vector<Face> ifaces = incident_faces(m, e);
		if (codegree(m, ifaces[0]) > 3 || codegree(m, ifaces[1]) > 3)
			return true;

		std::array<Vertex, 4> eqv = edge_quad_vertices(m, e);
		std::array<Vec3, 4> eqv_pos;
		for (uint32 i = 0; i < 4; ++i)
			eqv_pos[i] = value<Vec3>(m, vertex_position, eqv[i]);
		Scalar weight = quad_quality(eqv_pos);

		edges.push_back({weight, e});

		return true;
	});

	auto deleted_faces = add_attribute<bool, Face>(m, "__deleted_faces");
	deleted_faces->fill(false);

	std::vector<Edge> edges_to_delete;
	edges_to_delete.reserve(nbe / 2);

	std::sort(edges.begin(), edges.end(), [](const prio_edge& a, const prio_edge& b) { return a.w > b.w; });

	for (const auto& [w, e] : edges)
	{
		std::vector<Face> ifaces = incident_faces(m, e);

		if (value<bool>(m, deleted_faces, ifaces[0]) || value<bool>(m, deleted_faces, ifaces[1]))
			continue;
		if (w < 0.1)
			continue;

		value<bool>(m, deleted_faces, ifaces[0]) = true;
		value<bool>(m, deleted_faces, ifaces[1]) = true;
		edges_to_delete.push_back(e);
	}

	remove_attribute<Face>(m, deleted_faces);

	for (Edge e : edges_to_delete)
		merge_incident_faces(m, e);
}

template <typename MESH>
void blossom_quad_remeshing(MESH& m,
							std::shared_ptr<typename mesh_traits<MESH>::template Attribute<Vec3>>& vertex_position)
{
	using Vertex = typename mesh_traits<MESH>::Vertex;
	using Edge = typename mesh_traits<MESH>::Edge;
	using Face = typename mesh_traits<MESH>::Face;

	auto face_index = add_attribute<uint32, Face>(m, "__face_index");
	uint32 nb_faces = 0;
	foreach_cell(m, [&](Face f) -> bool {
		value<uint32>(m, face_index, f) = nb_faces++;
		return true;
	});

	if (nb_faces % 2 > 0)
	{
		Edge max_e;
		Scalar max_length = std::numeric_limits<Scalar>::min();
		foreach_cell(m, [&](Edge e) -> bool {
			if (is_incident_to_boundary(m, e))
			{
				Scalar length = geometry::length(m, e, vertex_position.get());
				if (length > max_length)
				{
					max_e = e;
					max_length = length;
				}
			}
			return true;
		});
		cut_boundary_triangle(m, max_e, vertex_position.get());
	}

	auto edge_weight = add_attribute<int, Edge>(m, "__edge_weight");
	uint32 nb_graph_edges = 0;
	foreach_cell(m, [&](Edge e) -> bool {
		if (is_incident_to_boundary(m, e))
		{
			if (degree(m, incident_vertices(m, e)[0]) > 3)
			{
				value<int>(m, edge_weight, e) = 10;
				nb_graph_edges++;
			}
		}
		else
		{
			std::array<Vertex, 4> eqv = edge_quad_vertices(m, e);
			std::array<Vec3, 4> eqv_pos;
			for (uint32 i = 0; i < 4; ++i)
				eqv_pos[i] = value<Vec3>(m, vertex_position, eqv[i]);
			Scalar weight = 1.0 - quad_quality(eqv_pos);

			value<int>(m, edge_weight, e) = int(100 * weight);
			nb_graph_edges++;
		}

		return true;
	});

	std::vector<int> edges;
	edges.reserve(nb_graph_edges * 2);
	std::vector<int> weights;
	weights.reserve(nb_graph_edges);

	foreach_cell(m, [&](Edge e) -> bool {
		if (is_incident_to_boundary(m, e))
		{
			if (degree(m, incident_vertices(m, e)[0]) > 3)
			{
				Face f1 = incident_faces(m, e)[0];
				Face f2 = next_boundary_face(m, e);
				edges.push_back(value<uint32>(m, face_index, f1));
				edges.push_back(value<uint32>(m, face_index, f2));
				weights.push_back(value<int>(m, edge_weight, e));
			}
		}
		else
		{
			std::vector<Face> ifaces = incident_faces(m, e);
			edges.push_back(value<uint32>(m, face_index, ifaces[0]));
			edges.push_back(value<uint32>(m, face_index, ifaces[1]));
			weights.push_back(value<int>(m, edge_weight, e));
		}

		return true;
	});

	PerfectMatching pm(nb_faces, nb_graph_edges);
	for (uint32 i = 0; i < nb_graph_edges; ++i)
		pm.AddEdge(edges[2 * i], edges[2 * i + 1], weights[i]);
	pm.Solve();

	int res = CheckPerfectMatchingOptimality(nb_faces, nb_graph_edges, edges.data(), weights.data(), &pm);
	printf("check optimality: res=%d (%s)\n", res, (res == 0) ? "ok" : ((res == 1) ? "error" : "fatal error"));
	double cost = ComputePerfectMatchingCost(nb_faces, nb_graph_edges, edges.data(), weights.data(), &pm);
	printf("cost = %.1f\n", cost);

	std::vector<Edge> edges_to_delete;
	edges_to_delete.reserve(nb_graph_edges / 2);

	foreach_cell(m, [&](Face f) -> bool {
		uint32 idx = value<uint32>(m, face_index, f);
		uint32 match_idx = pm.GetMatch(idx);
		if (idx < match_idx)
		{
			Face match_f;
			bool found = false;
			foreach_adjacent_face_through_edge(m, f, [&](Face af) -> bool {
				if (value<uint32>(m, face_index, af) == match_idx)
				{
					match_f = af;
					found = true;
				}
				return !found;
			});
			if (found)
			{
				Edge e;
				if (common_edge(m, f, match_f, e))
					edges_to_delete.push_back(e);
				else
					std::cout << "common edge not found.." << std::endl;
			}
			else // boundary graph edge
			{
				std::cout << "boundary edge match" << std::endl;

				Edge be;
				bool be_found = false;
				foreach_incident_edge(m, f, [&](Edge e) -> bool {
					if (is_incident_to_boundary(m, e))
					{
						be_found = true;
						be = e;
					}
					return be_found;
				});

				if (be_found)
				{
					std::cout << " --> boundary edge found" << std::endl;

					Face fbn = next_boundary_face(m, be);
					Face fbp = previous_boundary_face(m, be);
					std::vector<Vertex> vertices = incident_vertices(m, be);
					Vertex nv;
					if (value<uint32>(m, face_index, fbn) == match_idx)
					{
						std::cout << " --> match with next boundary face" << std::endl;
						found = true;
						nv = open_boundary_vertex(m, vertices[0]);
					}
					else if (value<uint32>(m, face_index, fbp) == match_idx)
					{
						std::cout << " --> match with previous boundary face" << std::endl;
						found = true;
						nv = open_boundary_vertex(m, vertices[1]);
					}
					if (found)
					{
						std::cout << " --> compute position" << std::endl;

						Vec3 p{0, 0, 0};
						uint32 nbv = 0;
						foreach_adjacent_vertex_through_edge(m, nv, [&](Vertex av) -> bool {
							p += value<Vec3>(m, vertex_position, av);
							++nbv;
							return true;
						});
						p /= Scalar(nbv);
						value<Vec3>(m, vertex_position, nv) = p;
					}
				}
				else
					std::cout << " --> boundary edge not found" << std::endl;
			}
		}
		return true;
	});

	remove_attribute<Face>(m, face_index);
	remove_attribute<Edge>(m, edge_weight);

	for (Edge e : edges_to_delete)
		merge_incident_faces(m, e);
}

} // namespace modeling

} // namespace cgogn

#endif // CGOGN_MODELING_ALGOS_REMESHING_BLOSSOM_QUAD_REMESHING_H_