/*******************************************************************************
 * CGoGN                                                                        *
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

#ifndef CGOGN_MODULE_SURFACE_DIFFERENTIAL_PROPERTIES_H_
#define CGOGN_MODULE_SURFACE_DIFFERENTIAL_PROPERTIES_H_

#include <cgogn/ui/app.h>
#include <cgogn/ui/imgui_helpers.h>
#include <cgogn/ui/module.h>

#include <cgogn/core/ui_modules/mesh_provider.h>

#include <cgogn/geometry/algos/angle.h>
#include <cgogn/geometry/algos/area.h>
#include <cgogn/geometry/algos/curvature.h>
#include <cgogn/geometry/algos/length.h>
#include <cgogn/geometry/algos/normal.h>
#include <cgogn/geometry/types/vector_traits.h>

namespace cgogn
{

namespace ui
{

using geometry::Scalar;
using geometry::Vec3;

template <typename MESH>
class SurfaceDifferentialProperties : public Module
{
	static_assert(mesh_traits<MESH>::dimension == 2,
				  "SurfaceDifferentialProperties can only be used with meshes of dimension 2");

	template <typename T>
	using Attribute = typename mesh_traits<MESH>::template Attribute<T>;

	using Vertex = typename mesh_traits<MESH>::Vertex;
	using Edge = typename mesh_traits<MESH>::Edge;

public:
	SurfaceDifferentialProperties(const App& app)
		: Module(app, "SurfaceDifferentialProperties (" + std::string{mesh_traits<MESH>::name} + ")", "Geometry"),
		  selected_mesh_(nullptr), selected_vertex_position_(nullptr), selected_vertex_normal_(nullptr),
		  selected_vertex_kmax_(nullptr), selected_vertex_kmin_(nullptr), selected_vertex_kgaussian_(nullptr),
		  selected_vertex_Kmax_(nullptr), selected_vertex_Kmin_(nullptr), selected_vertex_Knormal_(nullptr),
		  selected_area_policy_(geometry::VertexAreaPolicy::BARYCENTER)
	{
	}

	~SurfaceDifferentialProperties()
	{
	}

	void compute_normal(const MESH& m, const Attribute<Vec3>* vertex_position, Attribute<Vec3>* vertex_normal)
	{
		geometry::compute_normal<Vertex>(m, vertex_position, vertex_normal);
		mesh_provider_->emit_attribute_changed(m, vertex_normal);
	}

	void compute_curvature(const MESH& m, Scalar radius, const Attribute<Vec3>* vertex_position,
						   const Attribute<Vec3>* vertex_normal, const Attribute<Scalar>* edge_angle,
						   Attribute<Scalar>* vertex_kmax, Attribute<Scalar>* vertex_kmin,
						   Attribute<Scalar>* vertex_kgaussian, Attribute<Vec3>* vertex_Kmax,
						   Attribute<Vec3>* vertex_Kmin, Attribute<Vec3>* vertex_Knormal)
	{
		geometry::compute_curvature(m, radius, vertex_position, vertex_normal, edge_angle, vertex_kmax, vertex_kmin,
									vertex_Kmax, vertex_Kmin, vertex_Knormal);
		mesh_provider_->emit_attribute_changed(m, vertex_kmax);
		mesh_provider_->emit_attribute_changed(m, vertex_kmin);
		mesh_provider_->emit_attribute_changed(m, vertex_Kmax);
		mesh_provider_->emit_attribute_changed(m, vertex_Kmin);
		mesh_provider_->emit_attribute_changed(m, vertex_Knormal);

		// compute gaussian curvature using kmax * kmin
		parallel_foreach_cell(m, [&](Vertex v) -> bool {
			value<Scalar>(m, vertex_kgaussian, v) = value<Scalar>(m, vertex_kmax, v) * value<Scalar>(m, vertex_kmin, v);
			return true;
		});
		mesh_provider_->emit_attribute_changed(m, vertex_kgaussian);
	}

	// compute angular defect gaussian curvature
	void compute_gaussian_curvature(const MESH& m, const Attribute<Vec3>* vertex_position,
									geometry::VertexAreaPolicy area_policy, Attribute<Scalar>* vertex_kgaussian)
	{
		geometry::compute_gaussian_curvature(m, vertex_position, area_policy, vertex_kgaussian);
		mesh_provider_->emit_attribute_changed(m, vertex_kgaussian);
	}

protected:
	void init() override
	{
		mesh_provider_ = static_cast<MeshProvider<MESH>*>(
			app_.module("MeshProvider (" + std::string{mesh_traits<MESH>::name} + ")"));
	}

	void left_panel() override
	{

		selected_mesh_ = mesh_provider_->get_selected_mesh();
		selected_vertex_position_.reset();
		selected_vertex_normal_.reset();
		selected_vertex_kmax_.reset();
		selected_vertex_kmin_.reset();
		selected_vertex_kgaussian_.reset();
		selected_vertex_Kmax_.reset();
		selected_vertex_Kmin_.reset();
		selected_vertex_Knormal_.reset();


		if (selected_mesh_)
		{
			imgui_combo_attribute<Vertex, Vec3>(
				*selected_mesh_, selected_vertex_position_, "Position",
				[&](const decltype(selected_vertex_position_)& attribute) { selected_vertex_position_ = attribute; });

			imgui_combo_attribute<Vertex, Vec3>(
				*selected_mesh_, selected_vertex_normal_, "Normal",
				[&](const decltype(selected_vertex_normal_)& attribute) { selected_vertex_normal_ = attribute; });

			imgui_combo_attribute<Vertex, Scalar>(
				*selected_mesh_, selected_vertex_kmax_, "kmax",
				[&](const decltype(selected_vertex_kmax_)& attribute) { selected_vertex_kmax_ = attribute; });

			imgui_combo_attribute<Vertex, Scalar>(
				*selected_mesh_, selected_vertex_kmin_, "kmin",
				[&](const decltype(selected_vertex_kmin_)& attribute) { selected_vertex_kmin_ = attribute; });

			imgui_combo_attribute<Vertex, Scalar>(
				*selected_mesh_, selected_vertex_kgaussian_, "kgaussian",
				[&](const decltype(selected_vertex_kgaussian_)& attribute) { selected_vertex_kgaussian_ = attribute; });

			imgui_combo_attribute<Vertex, Vec3>(
				*selected_mesh_, selected_vertex_Kmax_, "Kmax",
				[&](const decltype(selected_vertex_Kmax_)& attribute) { selected_vertex_Kmax_ = attribute; });

			imgui_combo_attribute<Vertex, Vec3>(
				*selected_mesh_, selected_vertex_Kmin_, "Kmin",
				[&](const decltype(selected_vertex_Kmin_)& attribute) { selected_vertex_Kmin_ = attribute; });

			imgui_combo_attribute<Vertex, Vec3>(
				*selected_mesh_, selected_vertex_Knormal_, "Knormal",
				[&](const decltype(selected_vertex_Knormal_)& attribute) { selected_vertex_Knormal_ = attribute; });

			if (selected_vertex_position_)
			{
				if (ImGui::Button("Compute normal"))
				{
					if (!selected_vertex_normal_)
						selected_vertex_normal_ = get_or_add_attribute<Vec3, Vertex>(*selected_mesh_, "normal");
					compute_normal(*selected_mesh_, selected_vertex_position_.get(), selected_vertex_normal_.get());
				}

				ImGui::TextUnformatted("Vertex area"); // vertex area policy to compute gaussian curvature
				ImGui::BeginGroup();
				if (ImGui::RadioButton("Barycenter##VertexAreaPolicy",
									   selected_area_policy_ == geometry::VertexAreaPolicy::BARYCENTER))
				{
					selected_area_policy_ = geometry::VertexAreaPolicy::BARYCENTER;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Voronoï##VertexAreaPolicy",
									   selected_area_policy_ == geometry::VertexAreaPolicy::VORONOI))
				{
					selected_area_policy_ = geometry::VertexAreaPolicy::VORONOI;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Mixed##VertexAreaPolicy",
									   selected_area_policy_ == geometry::VertexAreaPolicy::MIXED))
				{
					selected_area_policy_ = geometry::VertexAreaPolicy::MIXED;
				}
				ImGui::EndGroup();

				if (ImGui::Button("Compute gaussian curvature"))
				{
					if (!selected_vertex_kgaussian_)
						selected_vertex_kgaussian_ = get_or_add_attribute<Scalar, Vertex>(*selected_mesh_, "kgaussian");
					compute_gaussian_curvature(*selected_mesh_, selected_vertex_position_.get(), selected_area_policy_,
											   selected_vertex_kgaussian_.get());
				}
			}

			if (selected_vertex_position_ && selected_vertex_normal_)
			{
				if (ImGui::Button("Compute curvature"))
				{
					if (!selected_vertex_kmax_)
						selected_vertex_kmax_ = get_or_add_attribute<Scalar, Vertex>(*selected_mesh_, "kmax");
					if (!selected_vertex_kmin_)
						selected_vertex_kmin_ = get_or_add_attribute<Scalar, Vertex>(*selected_mesh_, "kmin");
					if (!selected_vertex_kgaussian_)
						selected_vertex_kgaussian_ = get_or_add_attribute<Scalar, Vertex>(*selected_mesh_, "kgaussian");
					if (!selected_vertex_Kmax_)
						selected_vertex_Kmax_ = get_or_add_attribute<Vec3, Vertex>(*selected_mesh_, "Kmax");
					if (!selected_vertex_Kmin_)
						selected_vertex_Kmin_ = get_or_add_attribute<Vec3, Vertex>(*selected_mesh_, "Kmin");
					if (!selected_vertex_Knormal_)
						selected_vertex_Knormal_ = get_or_add_attribute<Vec3, Vertex>(*selected_mesh_, "Knormal");

					std::shared_ptr<Attribute<Scalar>> edge_angle =
						add_attribute<Scalar, Edge>(*selected_mesh_, "__edge_angle");
					geometry::compute_angle(*selected_mesh_, selected_vertex_position_.get(), edge_angle.get());

					Scalar mean_edge_length =
						geometry::mean_edge_length(*selected_mesh_, selected_vertex_position_.get());

					compute_curvature(*selected_mesh_, mean_edge_length * 2.5, selected_vertex_position_.get(),
									  selected_vertex_normal_.get(), edge_angle.get(), selected_vertex_kmax_.get(),
									  selected_vertex_kmin_.get(), selected_vertex_kgaussian_.get(),
									  selected_vertex_Kmax_.get(), selected_vertex_Kmin_.get(),
									  selected_vertex_Knormal_.get());

					remove_attribute<Edge>(*selected_mesh_, edge_angle);
				}
			}
		}
	}

private:
	MESH* selected_mesh_;
	std::shared_ptr<Attribute<Vec3>> selected_vertex_position_;
	std::shared_ptr<Attribute<Vec3>> selected_vertex_normal_;
	std::shared_ptr<Attribute<Scalar>> selected_vertex_kmax_;
	std::shared_ptr<Attribute<Scalar>> selected_vertex_kmin_;
	std::shared_ptr<Attribute<Scalar>> selected_vertex_kgaussian_;
	std::shared_ptr<Attribute<Vec3>> selected_vertex_Kmax_;
	std::shared_ptr<Attribute<Vec3>> selected_vertex_Kmin_;
	std::shared_ptr<Attribute<Vec3>> selected_vertex_Knormal_;
	geometry::VertexAreaPolicy selected_area_policy_;
	MeshProvider<MESH>* mesh_provider_;
};

} // namespace ui

} // namespace cgogn

#endif // CGOGN_MODULE_SURFACE_DIFFERENTIAL_PROPERTIES_H_
