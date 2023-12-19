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

#ifndef CGOGN_MODULE_MESH_PROVIDER_MESH_DATA_H_
#define CGOGN_MODULE_MESH_PROVIDER_MESH_DATA_H_

#include <cgogn/core/types/cells_set.h>

#include <cgogn/core/functions/mesh_info.h>

#include <cgogn/rendering/mesh_render.h>
#include <cgogn/rendering/vbo_update.h>

#include <cgogn/geometry/functions/bounding_box.h>
#include <cgogn/geometry/types/vector_traits.h>

#include <boost/synapse/emit.hpp>

#include <list>
#include <unordered_map>

namespace cgogn
{

namespace ui	
{

using geometry::Vec3;

template <typename MESH>
struct MeshData
{
	template <typename T>
	using Attribute = typename mesh_traits<MESH>::template Attribute<T>;
	using AttributeGen = typename mesh_traits<MESH>::AttributeGen;

	MeshData()
		: mesh_(nullptr), bb_vertex_position_(nullptr), bb_min_(0, 0, 0), bb_max_(0, 0, 0), outlined_until_(0.0),
		  translate_({0.f, 0.f, 0.f}), rotate_({0.f, 0.f, 0.f}), scale_(1.f), tr_for_rotate_({0.f, 0.f, 0.f})
	{
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MeshData);

	rendering::MeshRender* mesh_render()
	{
		return &render_;
	}

	void init(const MESH* m)
	{
		mesh_ = m;
		update_nb_cells();
	}

	void draw(rendering::DrawingType primitive, const typename std::shared_ptr<Attribute<Vec3>> position = nullptr)
	{
		if (!render_.is_primitive_uptodate(primitive))
			render_.init_primitives(*mesh_, primitive, position.get());
		render_.draw(primitive);
	}

	void init_primitives(rendering::DrawingType primitive,
						 const typename std::shared_ptr<Attribute<Vec3>> position = nullptr)
	{
		render_.init_primitives(*mesh_, primitive, position.get());
	}

	bool is_primitive_uptodate(rendering::DrawingType primitive)
	{
		return render_.is_primitive_uptodate(primitive);
	}

	void set_primitive_dirty(rendering::DrawingType primitive)
	{
		render_.set_primitive_dirty(primitive);
	}

	void set_all_primitives_dirty()
	{
		render_.set_all_primitives_dirty();
	}

private:
	template <class... T>
	void internal_update_nb_cells(const std::tuple<T...>&)
	{
		nb_cells_ = {cgogn::nb_cells<T>(*mesh_)...};
	}

public:
	void update_nb_cells()
	{
		internal_update_nb_cells(typename mesh_traits<MESH>::Cells{});
	}

	template <typename CELL>
	uint32 nb_cells()
	{
		static_assert(has_cell_type_v<MESH, CELL>, "CELL not supported in this MESH");
		static const uint32 cell_index = tuple_type_index<CELL, typename mesh_traits<MESH>::Cells>::value;
		return nb_cells_[cell_index];
	}

	template <typename CELL>
	std::vector<std::string> attributes_names()
	{
		std::vector<std::string> names;
		foreach_attribute<CELL>(
			*mesh_, [&](const std::shared_ptr<AttributeGen>& attribute) { names.push_back(attribute->name()); });
		return names;
	}

private:
	template <class... T>
	std::vector<std::vector<std::string>> attributes_names(const std::tuple<T...>&)
	{
		return {attributes_names<T>()...};
	}

public:
	std::vector<std::vector<std::string>> attributes_names()
	{
		return attributes_names(typename mesh_traits<MESH>::Cells{});
	}

	void update_bb()
	{
		if (!bb_vertex_position_)
		{
			bb_min_ = {0, 0, 0};
			bb_max_ = {0, 0, 0};
		}
		else
		{
			std::tie(bb_min_, bb_max_) = geometry::bounding_box(*bb_vertex_position_);
			tr_for_rotate_[0] = (bb_min_[0] + bb_max_[0]) / 2.f;
			tr_for_rotate_[1] = (bb_min_[1] + bb_max_[1]) / 2.f;
			tr_for_rotate_[2] = (bb_min_[2] + bb_max_[2]) / 2.f;
		}
	}

	rendering::VBO* vbo(AttributeGen* attribute)
	{
		if (auto it = vbos_.find(attribute); it != vbos_.end())
			return it->second.get();
		else
			return nullptr;
	}

	template <typename T>
	rendering::VBO* update_vbo(Attribute<T>* attribute, bool create_if_needed = false)
	{
		if (attribute == nullptr)
			return nullptr;

		rendering::VBO* v = vbo(attribute);
		if (!v && create_if_needed)
		{
			const auto [it, inserted] = vbos_.emplace(attribute, std::make_unique<rendering::VBO>());
			v = it->second.get();
		}
		if (v)
			rendering::update_vbo<T>(attribute, v);

		return v;
	}

	template <typename CELL, typename FUNC>
	void foreach_cells_set(const FUNC& f)
	{
		static_assert(has_cell_type_v<MESH, CELL>, "CELL not supported in this MESH");
		static_assert(is_func_parameter_same<FUNC, CellsSet<MESH, CELL>&>::value, "Wrong function parameter type");
		for (CellsSet<MESH, CELL>& cs : cells_sets<CELL>())
			f(cs);
	}

	template <typename CELL>
	CellsSet<MESH, CELL>& add_cells_set()
	{
		static_assert(has_cell_type_v<MESH, CELL>, "CELL not supported in this MESH");
		static const uint32 cell_index = tuple_type_index<CELL, typename mesh_traits<MESH>::Cells>::value;
		return cells_sets<CELL>().emplace_back(*mesh_, mesh_traits<MESH>::cell_names[cell_index] +
														   std::to_string(cells_sets<CELL>().size()));
	}

	const rendering::GLMat4 getTransfoMatrix()
	{
		// Translate 
		Eigen::Affine3f tr(Eigen::Translation3f(translate_[0], translate_[1], translate_[2]));

		// Scale
		Eigen::DiagonalMatrix<float, 4> sc(scale_, scale_, scale_, 1.0);

		// Rotate
		float value = 60.f;

		Eigen::Matrix3f rotationX;
		rotationX << 
			1, 0, 0, 
			0, cos(rotate_[0] / value), -sin(rotate_[0] / value), 
			0, sin(rotate_[0] / value), cos(rotate_[0] / value);

		// Rotation autour de l'axe Y
		Eigen::Matrix3f rotationY;
		rotationY << 
			cos(rotate_[1] / value), 0, sin(rotate_[1] / value), 
			0, 1, 0, 
			-sin(rotate_[1] / value), 0, cos(rotate_[1] / value);

		// Rotation autour de l'axe Z
		Eigen::Matrix3f rotationZ;
		rotationZ << 
			cos(rotate_[2] / value), -sin(rotate_[2] / value), 0, 
			sin(rotate_[2] / value), cos(rotate_[2] / value), 0, 
			0, 0, 1;

		// Composition des rotations autour des trois axes
		Eigen::Matrix3f rotationMatrix = rotationZ * rotationY * rotationX;

		// Deplacement du centre de rotation
		Eigen::Affine3f tr1(Eigen::Translation3f(tr_for_rotate_[0], tr_for_rotate_[1], tr_for_rotate_[2]));
		Eigen::Affine3f tr_1(Eigen::Translation3f(-tr_for_rotate_[0], -tr_for_rotate_[1], -tr_for_rotate_[2]));

		// Création d'une matrice 4x4 en étendant la rotation 3x3 avec une matrice d'identité
		Eigen::Matrix4f ro = Eigen::Matrix4f::Identity();
		ro.block<3, 3>(0, 0) = rotationMatrix;

		return tr.matrix() * tr1.matrix() * ro * sc * tr_1.matrix();
	}

private:
	template <typename CELL>
	void internal_rebuild_cells_sets_of_type()
	{
		for (CellsSet<MESH, CELL>& cs : cells_sets<CELL>())
			cs.rebuild();
	}

	template <class... T>
	void internal_rebuild_cells_sets(const std::tuple<T...>&)
	{
		// std::initializer_list<int> (comma operator returns 0 for each call)
		auto a = {(internal_rebuild_cells_sets_of_type<T>(), 0)...};
		unused_parameters(a);
	}

public:
	void rebuild_cells_sets()
	{
		internal_rebuild_cells_sets(typename mesh_traits<MESH>::Cells{});
	}

	const MESH* mesh_;
	std::shared_ptr<Attribute<Vec3>> bb_vertex_position_;
	Vec3 bb_min_, bb_max_;
	std::array<uint32, std::tuple_size<typename mesh_traits<MESH>::Cells>::value> nb_cells_;
	float64 outlined_until_;
	std::vector<float> translate_;
	std::vector<float> rotate_;
	float scale_;
	std::vector<float> tr_for_rotate_;

private:
	template <class>
	struct tuple_of_lists_of_cells_set_of_T_from_tuple_of_T;
	template <template <typename... Args> class tuple, typename... T>
	struct tuple_of_lists_of_cells_set_of_T_from_tuple_of_T<tuple<T...>>
	{
		using type = std::tuple<std::list<CellsSet<MESH, T>>...>;
	};
	using CellsSets =
		typename tuple_of_lists_of_cells_set_of_T_from_tuple_of_T<typename mesh_traits<MESH>::Cells>::type;

	template <typename CELL>
	std::list<CellsSet<MESH, CELL>>& cells_sets()
	{
		return std::get<tuple_type_index<std::list<CellsSet<MESH, CELL>>, CellsSets>::value>(cells_sets_);
	}

	rendering::MeshRender render_;
	std::unordered_map<AttributeGen*, std::unique_ptr<rendering::VBO>> vbos_;
	CellsSets cells_sets_;
};

} // namespace ui

} // namespace cgogn

#endif // CGOGN_MODULE_MESH_PROVIDER_MESH_DATA_H_
