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

#ifndef CGOGN_MODULE_SURFACE_RENDER_BGFX_H_
#define CGOGN_MODULE_SURFACE_RENDER_BGFX_H_

#include <cgogn/core/ui_modules/mesh_provider.h>
#include <cgogn/ui/app.h>
#include <cgogn/ui/imgui_helpers.h>
#include <cgogn/ui/module.h>
#include <cgogn/ui/view.h>

// BGFX
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <cgogn/geometry/types/vector_traits.h>

#include <cgogn/rendering/shaders/outliner.h>
#include <cgogn/rendering/shaders/shader_bold_line.h>
#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_flat_color_per_face.h>
#include <cgogn/rendering/shaders/shader_flat_color_per_vertex.h>
#include <cgogn/rendering/shaders/shader_flat_scalar_per_face.h>
#include <cgogn/rendering/shaders/shader_flat_scalar_per_vertex.h>
#include <cgogn/rendering/shaders/shader_phong.h>
#include <cgogn/rendering/shaders/shader_phong_color_per_face.h>
#include <cgogn/rendering/shaders/shader_phong_color_per_vertex.h>
#include <cgogn/rendering/shaders/shader_phong_scalar_per_face.h>
#include <cgogn/rendering/shaders/shader_phong_scalar_per_vertex.h>
#include <cgogn/rendering/shaders/shader_point_sprite.h>

#include <cgogn/geometry/algos/area.h>
#include <cgogn/geometry/algos/length.h>

#include <boost/synapse/connect.hpp>

#include <unordered_map>

#include <thirdparty/common/bgfx_utils.h>


// For shader loading
#include <filesystem>


namespace cgogn
{

namespace ui
{

using geometry::Scalar;
using geometry::Vec3;

template <typename MESH>
class SurfaceRender : public ViewModule
{
	static_assert(mesh_traits<MESH>::dimension >= 2, "SurfaceRender can only be used with meshes of dimension >= 2");

	enum AttributePerCell
	{
		GLOBAL = 0,
		PER_VERTEX,
		PER_EDGE,
		PER_FACE
	};
	enum ColorType
	{
		SCALAR = 0,
		VECTOR
	};

	template <typename T>
	using Attribute = typename mesh_traits<MESH>::template Attribute<T>;

	using Vertex = typename mesh_traits<MESH>::Vertex;
	// using Edge = typename mesh_traits<MESH>::Edge;
	using Face = typename mesh_traits<MESH>::Face;

	struct Parameters
	{
		Parameters()
			: vertex_position_(nullptr), vertex_position_vbo_(nullptr), vertex_normal_(nullptr),
			  vertex_normal_vbo_(nullptr), vertex_scalar_(nullptr), vertex_scalar_vbo_(nullptr), vertex_color_(nullptr),
			  vertex_color_vbo_(nullptr), edge_color_(nullptr), edge_color_vbo_(nullptr), face_scalar_(nullptr),
			  face_scalar_vbo_(nullptr), face_color_(nullptr), face_color_vbo_(nullptr), vertex_radius_(nullptr),
			  vertex_radius_vbo_(nullptr), vertex_point_color_(nullptr), vertex_point_color_vbo_(nullptr),
			  render_vertices_(true), render_edges_(true), render_faces_(true), ghost_mode_(false),
			  normal_per_cell_(PER_FACE), color_per_cell_(GLOBAL), color_type_(SCALAR), point_color_per_cell_(GLOBAL),
			  point_color_type_(VECTOR), edge_color_per_cell_(GLOBAL), edge_color_type_(VECTOR),
			  vertex_scale_factor_(1.0f), auto_update_vertex_scalar_min_max_(true),
			  auto_update_face_scalar_min_max_(true)
		{
			//TODO : Reimplement with BGFX
			param_point_sprite_ = rendering::ShaderPointSprite::generate_param();
			param_point_sprite_->color_ = {0.75f, 1.0f, 0.0f, 1.0f};

			
			param_point_sprite_size_ = rendering::ShaderPointSpriteSize::generate_param();
			param_point_sprite_size_->color_ = {0.75f, 1.0f, 0.0f, 1.0f};
			

			param_point_sprite_color_ = rendering::ShaderPointSpriteColor::generate_param();

			param_point_sprite_color_size_ = rendering::ShaderPointSpriteColorSize::generate_param();
			
			param_bold_line_ = rendering::ShaderBoldLine::generate_param();
			//param_bold_line_->color_ = {0.0f, 0.0f, 0.0f, 1.0f};

			param_bold_line_color_ = rendering::ShaderBoldLineColor::generate_param();

			param_flat_ = rendering::ShaderFlat::generate_param();
			param_flat_->front_color_ = {0.1f, 0.9f, 0.9f, 1.0f};
			param_flat_->back_color_ = {0.1f, 0.9f, 0.9f, 1.0f};

			param_flat_color_per_vertex_ = rendering::ShaderFlatColorPerVertex::generate_param();

			param_flat_scalar_per_vertex_ = rendering::ShaderFlatScalarPerVertex::generate_param();

			param_flat_color_per_face_ = rendering::ShaderFlatColorPerFace::generate_param();

			param_flat_scalar_per_face_ = rendering::ShaderFlatScalarPerFace::generate_param();

			param_phong_ = rendering::ShaderPhong::generate_param();
			// param_phong_->front_color_ = {0.1f, 0.9f, 0.9f, 1.0f};
			// param_phong_->back_color_ = {0.1f, 0.9f, 0.9f, 1.0f};

			param_phong_color_per_vertex_ = rendering::ShaderPhongColorPerVertex::generate_param();

			param_phong_scalar_per_vertex_ = rendering::ShaderPhongScalarPerVertex::generate_param();

			param_phong_color_per_face_ = rendering::ShaderPhongColorPerFace::generate_param();

			param_phong_scalar_per_face_ = rendering::ShaderPhongScalarPerFace::generate_param();
			
			
		}

		CGOGN_NOT_COPYABLE_NOR_MOVABLE(Parameters);

		std::shared_ptr<Attribute<Vec3>> vertex_position_;
		rendering::VBO* vertex_position_vbo_;
		std::shared_ptr<Attribute<Vec3>> vertex_normal_;
		rendering::VBO* vertex_normal_vbo_;
		std::shared_ptr<Attribute<Scalar>> vertex_scalar_;
		rendering::VBO* vertex_scalar_vbo_;
		std::shared_ptr<Attribute<Vec3>> vertex_color_;
		rendering::VBO* vertex_color_vbo_;
		std::shared_ptr<Attribute<Vec3>> edge_color_;
		rendering::VBO* edge_color_vbo_;
		std::shared_ptr<Attribute<Scalar>> face_scalar_;
		rendering::VBO* face_scalar_vbo_;
		std::shared_ptr<Attribute<Vec3>> face_color_;
		rendering::VBO* face_color_vbo_;
		std::shared_ptr<Attribute<Scalar>> vertex_radius_;
		rendering::VBO* vertex_radius_vbo_;
		std::shared_ptr<Attribute<Vec3>> vertex_point_color_;
		rendering::VBO* vertex_point_color_vbo_;
		std::shared_ptr<std::vector<bx::Vec3>> vertices;

		std::unique_ptr<rendering::ShaderPointSprite::Param> param_point_sprite_;
		std::unique_ptr<rendering::ShaderPointSpriteSize::Param> param_point_sprite_size_;
		std::unique_ptr<rendering::ShaderPointSpriteColor::Param> param_point_sprite_color_;
		std::unique_ptr<rendering::ShaderPointSpriteColorSize::Param> param_point_sprite_color_size_;
		std::unique_ptr<rendering::ShaderBoldLine::Param> param_bold_line_;
		std::unique_ptr<rendering::ShaderBoldLineColor::Param> param_bold_line_color_;
		std::unique_ptr<rendering::ShaderFlat::Param> param_flat_;
		std::unique_ptr<rendering::ShaderFlatColorPerVertex::Param> param_flat_color_per_vertex_;
		std::unique_ptr<rendering::ShaderFlatScalarPerVertex::Param> param_flat_scalar_per_vertex_;
		std::unique_ptr<rendering::ShaderFlatColorPerFace::Param> param_flat_color_per_face_;
		std::unique_ptr<rendering::ShaderFlatScalarPerFace::Param> param_flat_scalar_per_face_;
		std::unique_ptr<rendering::ShaderPhong::Param> param_phong_;
		std::unique_ptr<rendering::ShaderPhongColorPerVertex::Param> param_phong_color_per_vertex_;
		std::unique_ptr<rendering::ShaderPhongScalarPerVertex::Param> param_phong_scalar_per_vertex_;
		std::unique_ptr<rendering::ShaderPhongColorPerFace::Param> param_phong_color_per_face_;
		std::unique_ptr<rendering::ShaderPhongScalarPerFace::Param> param_phong_scalar_per_face_;

		bool render_vertices_;
		bool render_edges_;
		bool render_faces_;

		bool ghost_mode_;

		AttributePerCell normal_per_cell_;
		AttributePerCell color_per_cell_;
		ColorType color_type_;
		AttributePerCell point_color_per_cell_;
		ColorType point_color_type_; // TODO: manage scalar point color
		AttributePerCell edge_color_per_cell_;
		ColorType edge_color_type_; // TODO: manage scalar edge color

		float32 vertex_scale_factor_;
		float32 vertex_base_size_;

		bool auto_update_vertex_scalar_min_max_;
		bool auto_update_face_scalar_min_max_;
	};
	int m_width, m_height;

public:
	SurfaceRender(const App& app)
		: ViewModule(app, "SurfaceRender (" + std::string{mesh_traits<MESH>::name} + ")", "Rendering"),
		  selected_view_(app.current_view()), selected_mesh_(nullptr)
	{
		outline_engine_ = rendering::Outliner::instance();
		m_width = app.window_size().first;
		m_height = app.window_size().second;
	}


	~SurfaceRender()
	{
	}

private:
	void init_mesh(MESH* m)
	{
		for (View* v : linked_views_)
		{

			parameters_[v][m];

			
			std::shared_ptr<Attribute<Vec3>> vertex_position = cgogn::get_attribute<Vec3, Vertex>(*m, "position");
			if (vertex_position)
				set_vertex_position(*v, *m, vertex_position);

			mesh_connections_[m].push_back(
				boost::synapse::connect<typename MeshProvider<MESH>::connectivity_changed>(m, [this, v, m]() {
					Parameters& p = parameters_[v][m];
					MeshData<MESH>& md = mesh_provider_->mesh_data(*m);
					if (p.vertex_position_)
					{
						if constexpr (has_edge_v<MESH>)
							p.vertex_base_size_ =
								float32(geometry::mean_edge_length(*m, p.vertex_position_.get()) / 7.0);
						else
						{
							p.vertex_base_size_ = float32(geometry::mean_cell_area<Face>(*m, p.vertex_position_.get()) *
														  10.0 * (md.bb_max_ - md.bb_min_).norm());
						}
					}
					if (p.vertex_base_size_ == 0.0)
						p.vertex_base_size_ = float32((md.bb_max_ - md.bb_min_).norm() / 20.0);
					v->request_update();
				}));
			mesh_connections_[m].push_back(
				boost::synapse::connect<typename MeshProvider<MESH>::template attribute_changed_t<Vec3>>(
					m, [this, v, m](Attribute<Vec3>* attribute) {
						Parameters& p = parameters_[v][m];
						MeshData<MESH>& md = mesh_provider_->mesh_data(*m);
						if (p.vertex_position_.get() == attribute)
						{
							if constexpr (has_edge_v<MESH>)
								p.vertex_base_size_ =
									float32(geometry::mean_edge_length(*m, p.vertex_position_.get()) / 7.0);
							else
								p.vertex_base_size_ =
									float32(geometry::mean_cell_area<Face>(*m, p.vertex_position_.get()) * 10.0 *
											(md.bb_max_ - md.bb_min_).norm());
							if (p.vertex_base_size_ == 0.0)
								p.vertex_base_size_ = float32((md.bb_max_ - md.bb_min_).norm() / 20.0);
						}
						v->request_update();
					}));
			mesh_connections_[m].push_back(
				boost::synapse::connect<typename MeshProvider<MESH>::template attribute_changed_t<Scalar>>(
					m, [this, v, m](Attribute<Scalar>* attribute) {
						Parameters& p = parameters_[v][m];
						if (p.vertex_scalar_.get() == attribute && p.auto_update_vertex_scalar_min_max_)
							update_vertex_scalar_min_max_values(p);
						if (p.face_scalar_.get() == attribute && p.auto_update_face_scalar_min_max_)
							update_face_scalar_min_max_values(p);
						v->request_update();
					}));
		}
	}

public:
	void set_vertex_position(View& v, const MESH& m, const std::shared_ptr<Attribute<Vec3>>& vertex_position)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.vertex_position_ == vertex_position)
			return;

		p.vertex_position_ = vertex_position;
		if (p.vertex_position_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			*/
			p.vertices = md.update_vbo_bgfx(p.vertex_position_.get(), true);

			if constexpr (has_edge_v<MESH>)
				p.vertex_base_size_ = float32(geometry::mean_edge_length(m, p.vertex_position_.get()) / 7.0);
			else
				p.vertex_base_size_ = float32(geometry::mean_cell_area<Face>(m, p.vertex_position_.get()) * 10.0 *
											  (md.bb_max_ - md.bb_min_).norm());
			if (p.vertex_base_size_ == 0.0)
				p.vertex_base_size_ = float32((md.bb_max_ - md.bb_min_).norm() / 20.0);
		}
		else
			p.vertex_position_vbo_ = nullptr;

//		p.param_flat_->set_vbos({p.vertex_position_vbo_});
		p.param_flat_->set_vbo(p.vertices);		
		p.param_point_sprite_->set_vbos({p.vertex_position_vbo_});
		//p.param_point_sprite_size_->set_vbos({p.vertex_position_vbo_, p.vertex_radius_vbo_});
		//p.param_point_sprite_color_->set_vbos({p.vertex_position_vbo_, p.vertex_point_color_vbo_});
		//p.param_point_sprite_color_size_->set_vbos(
		//	{p.vertex_position_vbo_, p.vertex_point_color_vbo_, p.vertex_radius_vbo_});
		// p.param_bold_line_->set_vbos({p.vertex_position_vbo_});
		// p.param_bold_line_color_->set_vbos({p.vertex_position_vbo_, p.edge_color_vbo_});

		/* BGFX : TODO
		p.param_flat_->set_vbos({p.vertex_position_vbo_});
		*/

		// p.param_flat_color_per_vertex_->set_vbos({p.vertex_position_vbo_, p.vertex_color_vbo_});
		// p.param_flat_scalar_per_vertex_->set_vbos({p.vertex_position_vbo_, p.vertex_scalar_vbo_});
		// p.param_flat_color_per_face_->set_vbos({p.vertex_position_vbo_, p.face_color_vbo_});
		// p.param_flat_scalar_per_face_->set_vbos({p.vertex_position_vbo_, p.face_scalar_vbo_});
		// p.param_phong_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_});
		// p.param_phong_color_per_vertex_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_,
		// p.vertex_color_vbo_});
		// p.param_phong_scalar_per_vertex_->set_vbos(
		//	{p.vertex_position_vbo_, p.vertex_normal_vbo_, p.vertex_scalar_vbo_});
		// p.param_phong_color_per_face_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_, p.face_color_vbo_});
		// p.param_phong_scalar_per_face_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_, p.face_scalar_vbo_});

		v.request_update();
	}

	void set_vertex_normal(View& v, const MESH& m, const std::shared_ptr<Attribute<Vec3>>& vertex_normal)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.vertex_normal_ == vertex_normal)
			return;

		p.vertex_normal_ = vertex_normal;
		if (p.vertex_normal_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			p.vertex_normal_vbo_ = md.update_vbo(p.vertex_normal_.get(), true);
			*/
		}
		else
			p.vertex_normal_vbo_ = nullptr;

		// p.param_phong_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_});
		// p.param_phong_color_per_vertex_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_,
		// p.vertex_color_vbo_});
		// p.param_phong_scalar_per_vertex_->set_vbos(
		//	{p.vertex_position_vbo_, p.vertex_normal_vbo_, p.vertex_scalar_vbo_});
		// p.param_phong_color_per_face_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_, p.face_color_vbo_});
		// p.param_phong_scalar_per_face_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_, p.face_scalar_vbo_});

		v.request_update();
	}

	void set_vertex_color(View& v, const MESH& m, const std::shared_ptr<Attribute<Vec3>>& vertex_color)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.vertex_color_ == vertex_color)
			return;

		p.vertex_color_ = vertex_color;
		if (p.vertex_color_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			p.vertex_color_vbo_ = md.update_vbo(p.vertex_color_.get(), true);
			*/
		}
		else
			p.vertex_color_vbo_ = nullptr;

		// p.param_flat_color_per_vertex_->set_vbos({p.vertex_position_vbo_, p.vertex_color_vbo_});
		// p.param_phong_color_per_vertex_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_,
		// p.vertex_color_vbo_});

		v.request_update();
	}

	void set_vertex_scalar(View& v, const MESH& m, const std::shared_ptr<Attribute<Scalar>>& vertex_scalar)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.vertex_scalar_ == vertex_scalar)
			return;

		p.vertex_scalar_ = vertex_scalar;
		if (p.vertex_scalar_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			p.vertex_scalar_vbo_ = md.update_vbo(p.vertex_scalar_.get(), true);
			*/
			if (p.auto_update_vertex_scalar_min_max_)
				update_vertex_scalar_min_max_values(p);
		}
		else
		{
			p.vertex_scalar_vbo_ = nullptr;
			// p.param_flat_scalar_per_vertex_->color_map_.min_value_ = 0.0f;
			// p.param_flat_scalar_per_vertex_->color_map_.max_value_ = 1.0f;
			// p.param_phong_scalar_per_vertex_->color_map_.min_value_ = 0.0f;
			// p.param_phong_scalar_per_vertex_->color_map_.max_value_ = 1.0f;
		}

		// p.param_flat_scalar_per_vertex_->set_vbos({p.vertex_position_vbo_, p.vertex_scalar_vbo_});
		// p.param_phong_scalar_per_vertex_->set_vbos(
		//	{p.vertex_position_vbo_, p.vertex_normal_vbo_, p.vertex_scalar_vbo_});

		v.request_update();
	}

	void set_edge_color(View& v, const MESH& m, const std::shared_ptr<Attribute<Vec3>>& edge_color)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.edge_color_ == edge_color)
			return;

		p.edge_color_ = edge_color;
		if (p.edge_color_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			p.edge_color_vbo_ = md.update_vbo(p.edge_color_.get(), true);
			*/
		}
		else
			p.edge_color_vbo_ = nullptr;

		// p.param_bold_line_color_->set_vbos({p.vertex_position_vbo_, p.edge_color_vbo_});

		v.request_update();
	}

	void set_face_color(View& v, const MESH& m, const std::shared_ptr<Attribute<Vec3>>& face_color)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.face_color_ == face_color)
			return;

		p.face_color_ = face_color;
		if (p.face_color_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			p.face_color_vbo_ = md.update_vbo(p.face_color_.get(), true);
			*/
		}
		else
			p.face_color_vbo_ = nullptr;

		// p.param_flat_color_per_face_->set_vbos({p.vertex_position_vbo_, p.face_color_vbo_});
		// p.param_phong_color_per_face_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_, p.face_color_vbo_});

		v.request_update();
	}

	void set_face_scalar(View& v, const MESH& m, const std::shared_ptr<Attribute<Scalar>>& face_scalar)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.face_scalar_ == face_scalar)
			return;

		p.face_scalar_ = face_scalar;
		if (p.face_scalar_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			p.face_scalar_vbo_ = md.update_vbo(p.face_scalar_.get(), true);
			*/
			if (p.auto_update_face_scalar_min_max_)
				update_face_scalar_min_max_values(p);
		}
		else
		{
			p.face_scalar_vbo_ = nullptr;
			// p.param_flat_scalar_per_face_->color_map_.min_value_ = 0.0f;
			// p.param_flat_scalar_per_face_->color_map_.max_value_ = 1.0f;
			// p.param_phong_scalar_per_face_->color_map_.min_value_ = 0.0f;
			// p.param_phong_scalar_per_face_->color_map_.max_value_ = 1.0f;
		}

		// p.param_flat_scalar_per_face_->set_vbos({p.vertex_position_vbo_, p.face_scalar_vbo_});
		//p.param_phong_scalar_per_face_->set_vbos({p.vertex_position_vbo_, p.vertex_normal_vbo_, p.face_scalar_vbo_});

		v.request_update();
	}

	void set_vertex_radius(View& v, const MESH& m, const std::shared_ptr<Attribute<Scalar>>& vertex_radius)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.vertex_radius_ == vertex_radius)
			return;

		p.vertex_radius_ = vertex_radius;
		if (p.vertex_radius_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			p.vertex_radius_vbo_ = md.update_vbo(vertex_radius.get(), true);
			*/
		}
		else
			p.vertex_radius_vbo_ = nullptr;

		//p.param_point_sprite_size_->set_vbos({p.vertex_position_vbo_, p.vertex_radius_vbo_});
		//p.param_point_sprite_color_size_->set_vbos(
		//	{p.vertex_position_vbo_, p.vertex_point_color_vbo_, p.vertex_radius_vbo_});

		v.request_update();
	}

	void set_vertex_point_color(View& v, const MESH& m, const std::shared_ptr<Attribute<Vec3>>& vertex_point_color)
	{
		Parameters& p = parameters_[&v][&m];
		if (p.vertex_point_color_ == vertex_point_color)
			return;

		p.vertex_point_color_ = vertex_point_color;
		if (p.vertex_point_color_)
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(m);
			/* BGFX : TODO
			p.vertex_point_color_vbo_ = md.update_vbo(p.vertex_point_color_.get(), true);
			*/
		}
		else
			p.vertex_point_color_vbo_ = nullptr;

		//p.param_point_sprite_color_->set_vbos({p.vertex_position_vbo_, p.vertex_point_color_vbo_});
		//p.param_point_sprite_color_size_->set_vbos(
		//	{p.vertex_position_vbo_, p.vertex_point_color_vbo_, p.vertex_radius_vbo_});

		v.request_update();
	}

	void set_render_vertices(View& v, const MESH& m, bool b)
	{
		Parameters& p = parameters_[&v][&m];
		p.render_vertices_ = b;
		v.request_update();
	}

	void set_render_edges(View& v, const MESH& m, bool b)
	{
		Parameters& p = parameters_[&v][&m];
		p.render_edges_ = b;
		v.request_update();
	}

	void set_render_faces(View& v, const MESH& m, bool b)
	{
		Parameters& p = parameters_[&v][&m];
		p.render_faces_ = b;
		v.request_update();
	}

protected:
	void update_vertex_scalar_min_max_values(Parameters& p)
	{
		Scalar min = std::numeric_limits<float64>::max();
		Scalar max = std::numeric_limits<float64>::lowest();
		for (const Scalar& v : *p.vertex_scalar_)
		{
			if (v < min)
				min = v;
			if (v > max)
				max = v;
		}
		// p.param_flat_scalar_per_vertex_->color_map_.min_value_ = min;
		// p.param_flat_scalar_per_vertex_->color_map_.max_value_ = max;
		// p.param_phong_scalar_per_vertex_->color_map_.min_value_ = min;
		// p.param_phong_scalar_per_vertex_->color_map_.max_value_ = max;
	}

	void update_face_scalar_min_max_values(Parameters& p)
	{
		Scalar min = std::numeric_limits<float64>::max();
		Scalar max = std::numeric_limits<float64>::lowest();
		for (const Scalar& v : *p.face_scalar_)
		{
			if (v < min)
				min = v;
			if (v > max)
				max = v;
		}
		// p.param_flat_scalar_per_face_->color_map_.min_value_ = min;
		// p.param_flat_scalar_per_face_->color_map_.max_value_ = max;
		// p.param_phong_scalar_per_face_->color_map_.min_value_ = min;
		// p.param_phong_scalar_per_face_->color_map_.max_value_ = max;
	}


	// TODO : Rewrite this BGFX part somewhere else

	bgfx::ShaderHandle vertex_handler;
	bgfx::ShaderHandle fragment_shader;
	bgfx::ProgramHandle program;

	// TODO replace with dynamic buffers
	bgfx::DynamicVertexBufferHandle vb_cube;
	bgfx::DynamicIndexBufferHandle ib_cube;

	// Layout
	bgfx::VertexLayout layout;


	// Uniforms
	bgfx::UniformHandle u_transform;

	void init() override
	{
		mesh_provider_ = static_cast<ui::MeshProvider<MESH>*>(
			app_.module("MeshProvider (" + std::string{mesh_traits<MESH>::name} + ")"));
		mesh_provider_->foreach_mesh([this](MESH& m, const std::string&) { init_mesh(&m); });
		connections_.push_back(boost::synapse::connect<typename MeshProvider<MESH>::mesh_added>(
			mesh_provider_, this, &SurfaceRender<MESH>::init_mesh));

		
		/*
		// TODO:
		return;
		// Init BGFX shader
		// Create vertex stream declaration.

		layout.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			//.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
			.end();

		// TODO : Replace with dynamic buffers
		float cube_vertices[] = {
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
		};

		uint16_t cube_indices[] = {
			0, 1, 2, // 0
			1, 3, 2,
			4, 6, 5, // 2
			5, 6, 7,
			0, 2, 4, // 4
			4, 2, 6,
			1, 5, 3, // 6
			5, 7, 3,
			0, 4, 1, // 8
			4, 5, 1,
			2, 3, 6, // 10
			6, 3, 7,
		};
		

		vb_cube = bgfx::createDynamicVertexBuffer(
        bgfx::makeRef(cube_vertices, sizeof(cube_vertices)),
        layout);

		ib_cube = bgfx::createDynamicIndexBuffer(
			bgfx::makeRef(cube_indices, sizeof(cube_indices)));

		vertex_handler = bgfx::createShader(load_file("vs_cubes.bin", "simple_cube"));
		fragment_shader = bgfx::createShader(load_file("fs_cubes.bin", "simple_cube"));

		program = bgfx::createProgram(vertex_handler, fragment_shader, true);

		// Uniforms
    	u_transform = bgfx::createUniform("u_transform", bgfx::UniformType::Mat4);
		*/

	}

	void draw(View* view) override
	{
		/*
		for (auto& [m, p] : parameters_[view])
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(*m);

			const rendering::GLMat4& proj_matrix = view->projection_matrix();
			const rendering::GLMat4& view_matrix = view->modelview_matrix();

			
			// TODO : implement specific renderers
			// 1 - Bind shader
			// 2 - Draw
			//md.draw()
			// 3 - Release shader

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.	

			// Set rendering matrices through uniforms

			// Set vertex and index buffer.

			// Set render states.
			//bgfx::setTransform(mtx);
			float transform[16];
			// Identity
			bx::mtxIdentity(transform);
			bgfx::setUniform(u_transform, transform);

			// Update dynamic vertex buffer with p.vertex_position_
			

			// Update 
			// bgfx::setInstanceDataBuffer(vb_cube, 0, ...);

			// Destroy old vb_cube
			bgfx::destroy(vb_cube);

			// With p.vertex_position_ beeing std::shared_ptr<cgogn::ChunkArray<Eigen::Matrix<double, 3, 1> > >
			int nb_vertices = p.vertex_position_->size();

			// loop over vertices to create new array of floats
			float* vertices = new float[nb_vertices * 3];
			int i = 0;
			for (auto& v : *p.vertex_position_)
			{
				vertices[i] = v[0];
				vertices[i + 1] = v[1];
				vertices[i + 2] = v[2];
				i += 3;
			}

			float cube_vertices[] = {
				// Front face
				-0.5f, -0.5f,  0.5f, // Bottom-left
				0.5f, -0.5f,  0.5f, // Bottom-right    
				0.5f,  0.5f,  0.5f, // Top-right              
				0.5f,  0.5f,  0.5f, // Top-right
				-0.5f,  0.5f,  0.5f, // Top-left
				-0.5f, -0.5f,  0.5f, // Bottom-left                
				// Back face
				-0.5f, -0.5f, -0.5f, // Bottom-left
				0.5f,  0.5f, -0.5f, // Top-right
				0.5f, -0.5f, -0.5f, // Bottom-right        
				0.5f,  0.5f, -0.5f, // Top-right
				-0.5f, -0.5f, -0.5f, // Bottom-left
				-0.5f,  0.5f, -0.5f, // Top-left        
				// Left face
				-0.5f,  0.5f,  0.5f, // Top-right
				-0.5f,  0.5f, -0.5f, // Top-left
				-0.5f, -0.5f, -0.5f, // Bottom-left        
				-0.5f, -0.5f, -0.5f, // Bottom-left
				-0.5f, -0.5f,  0.5f, // Bottom-right
				-0.5f,  0.5f,  0.5f, // Top-right
				// Right face
				0.5f,  0.5f,  0.5f, // Top-left
				0.5f, -0.5f, -0.5f, // Bottom-right
				0.5f,  0.5f, -0.5f, // Top-right
				0.5f, -0.5f, -0.5f, // Bottom-right
				0.5f,  0.5f,  0.5f, // Top-left
				0.5f, -0.5f,  0.5f, // Bottom-left
				// Bottom face
				-0.5f, -0.5f, -0.5f, // Top-right
				0.5f, -0.5f, -0.5f, // Top-left
				0.5f, -0.5f,  0.5f, // Bottom-left
				0.5f, -0.5f,  0.5f, // Bottom-left
				-0.5f, -0.5f,  0.5f, // Bottom-right
				-0.5f, -0.5f, -0.5f, // Top-right
				// Top face
				-0.5f,  0.5f, -0.5f, // Top-left
				0.5f,  0.5f,  0.5f, // Bottom-right
				0.5f,  0.5f, -0.5f, // Top-right
				0.5f,  0.5f,  0.5f, // Bottom-right
				-0.5f,  0.5f, -0.5f, // Top-left
				-0.5f,  0.5f,  0.5f  // Bottom-left
			};



			// Create new vertex buffer
			vb_cube = bgfx::createDynamicVertexBuffer(
				bgfx::makeRef(cube_vertices, sizeof(cube_vertices)),
				layout);
			


			bgfx::setVertexBuffer(0, vb_cube);
			// Set model matrix for rendering.
	
			//bgfx::setIndexBuffer(ib_cube);

			// Set render states.
			bgfx::setState(0 | BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP);
					
        	bgfx::submit(0, program);

		}
		*/
		// TODO : Implement this
		
		for (auto& [m, p] : parameters_[view])
		{
			MeshData<MESH>& md = mesh_provider_->mesh_data(*m);

			const rendering::GLMat4& proj_matrix = view->projection_matrix();
			const rendering::GLMat4& view_matrix = view->modelview_matrix();

			if (p.render_faces_)
			{
				// DUMMY VIEW ID
				int id = 0;

				if (p.ghost_mode_)
				{
					//bgfx::addState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
						//			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE));

					//glDisable(GL_DEPTH_TEST);
					//glEnable(GL_BLEND);
					//glBlendFunc(GL_ONE, GL_ONE);
				}

				//glEnable(GL_POLYGON_OFFSET_FILL);
				//glPolygonOffset(1.0f, 1.5f);

				switch (p.normal_per_cell_)
				{
				case PER_VERTEX: {
					switch (p.color_per_cell_)
					{
					case GLOBAL: {
						if (p.param_phong_->attributes_initialized())
						{
							//p.param_phong_->bind(proj_matrix, view_matrix);
							//md.draw(rendering::TRIANGLES, p.vertex_position_);
							//p.param_phong_->release();
						}
					}
					break;
					case PER_VERTEX: {
						switch (p.color_type_)
						{
						case SCALAR: {
							if (p.param_phong_scalar_per_vertex_->attributes_initialized())
							{
								/*p.param_phong_scalar_per_vertex_->bind(proj_matrix, view_matrix);
								md.draw(rendering::TRIANGLES, p.vertex_position_);
								p.param_phong_scalar_per_vertex_->release();*/
							}
						}
						break;
						case VECTOR: {
							if (p.param_phong_color_per_vertex_->attributes_initialized())
							{
								/*p.param_phong_color_per_vertex_->bind(proj_matrix, view_matrix);
								md.draw(rendering::TRIANGLES, p.vertex_position_);
								p.param_phong_color_per_vertex_->release();*/
							}
						}
						break;
						}
					}
					break;
					case PER_FACE: {
						switch (p.color_type_)
						{
						case SCALAR: {
							if (p.param_phong_scalar_per_face_->attributes_initialized())
							{
								/*p.param_phong_scalar_per_face_->bind(proj_matrix, view_matrix);
								md.draw(rendering::TRIANGLES_TB, p.vertex_position_);
								p.param_phong_scalar_per_face_->release();*/
							}
						}
						break;
						case VECTOR: {
							if (p.param_phong_color_per_face_->attributes_initialized())
							{
								/*p.param_phong_color_per_face_->bind(proj_matrix, view_matrix);
								md.draw(rendering::TRIANGLES_TB, p.vertex_position_);
								p.param_phong_color_per_face_->release();*/
							}
						}
						break;
						}
					}
					break;
					default:
						cgogn_assert_not_reached("");
					}
				}
				break;
				case PER_FACE: {
					switch (p.color_per_cell_)
					{
					case GLOBAL: {
						if (p.param_flat_->attributes_initialized())
						{
							auto ibh = p.param_flat_->ibh();
							md.init_indices(rendering::TRIANGLES, ibh, p.vertex_position_);
							ibh;
							p.param_flat_->set_matrices(proj_matrix, view_matrix);
							// p.param_flat_->draw(m_width, m_height);
							p.param_flat_->draw();
							
							//p.param_flat_->bind(proj_matrix, view_matrix);
							//p.param_flat_->release();
						}
					}
					break;
					case PER_VERTEX: {
						switch (p.color_type_)
						{
						case SCALAR: {
							if (p.param_flat_scalar_per_vertex_->attributes_initialized())
							{
								/*p.param_flat_scalar_per_vertex_->bind(proj_matrix, view_matrix);
								md.draw(rendering::TRIANGLES, p.vertex_position_);
								p.param_flat_scalar_per_vertex_->release();*/
							}
						}
						break;
						case VECTOR: {
							if (p.param_flat_color_per_vertex_->attributes_initialized())
							{
								/*p.param_flat_color_per_vertex_->bind(proj_matrix, view_matrix);
								md.draw(rendering::TRIANGLES, p.vertex_position_);
								p.param_flat_color_per_vertex_->release();*/
							}
						}
						break;
						}
					}
					break;
					case PER_FACE: {
						switch (p.color_type_)
						{
						case SCALAR: {
							if (p.param_flat_scalar_per_face_->attributes_initialized())
							{
								/*p.param_flat_scalar_per_face_->bind(proj_matrix, view_matrix);
								md.draw(rendering::TRIANGLES_TB, p.vertex_position_);
								p.param_flat_scalar_per_face_->release();*/
							}
						}
						break;
						case VECTOR: {
							if (p.param_flat_color_per_face_->attributes_initialized())
							{
								/*p.param_flat_color_per_face_->bind(proj_matrix, view_matrix);
								md.draw(rendering::TRIANGLES_TB, p.vertex_position_);
								p.param_flat_color_per_face_->release();*/
							}
						}
						break;
						}
					}
					break;
					default:
						cgogn_assert_not_reached("");
					}
				}
				break;
				default:
					cgogn_assert_not_reached("");
				}

				/* BGFX : TODO
				glDisable(GL_POLYGON_OFFSET_FILL);
				if (p.ghost_mode_)
				{
					glDisable(GL_BLEND);
					glEnable(GL_DEPTH_TEST);
				}
				*/
			}

			if (false && p.render_edges_)
			{
				switch (p.edge_color_per_cell_)
				{
				case GLOBAL: {
					if (p.param_bold_line_->attributes_initialized())
					{
						p.param_bold_line_->bind(proj_matrix, view_matrix);
						md.draw(rendering::LINES);
						p.param_bold_line_->release();
					}
				}
				break;
				case PER_EDGE: {
					if (p.param_bold_line_color_->attributes_initialized())
					{
						p.param_bold_line_color_->bind(proj_matrix, view_matrix);
						md.draw(rendering::LINES_TB);
						p.param_bold_line_color_->release();
					}
				}
				break;
				default:
					cgogn_assert_not_reached("");
				}
			}

			if (p.render_vertices_)
			{
				if (p.vertex_radius_)
				{
					switch (p.point_color_per_cell_)
					{
					case GLOBAL: {
						if (p.param_point_sprite_size_->attributes_initialized())
						{
							p.param_point_sprite_size_->bind(proj_matrix, view_matrix);
							md.draw(rendering::POINTS);
							p.param_point_sprite_size_->release();
						}
					}
					break;
					case PER_VERTEX: {
						if (p.param_point_sprite_color_size_->attributes_initialized())
						{
							p.param_point_sprite_color_size_->bind(proj_matrix, view_matrix);
							md.draw(rendering::POINTS);
							p.param_point_sprite_color_size_->release();
						}
					}
					break;
					default:
						cgogn_assert_not_reached("");
					}
				}
				else
				{
					switch (p.point_color_per_cell_)
					{
					case GLOBAL: {
						if (p.param_point_sprite_->attributes_initialized())
						{
							p.param_point_sprite_->point_size_ = p.vertex_base_size_ * p.vertex_scale_factor_;
							p.param_point_sprite_->bind(proj_matrix, view_matrix);
							md.draw(rendering::POINTS);
							p.param_point_sprite_->release();
						}
					}
					break;
					case PER_VERTEX: {
						if (p.param_point_sprite_color_->attributes_initialized())
						{
							p.param_point_sprite_color_->point_size_ = p.vertex_base_size_ * p.vertex_scale_factor_;
							p.param_point_sprite_color_->bind(proj_matrix, view_matrix);
							md.draw(rendering::POINTS);
							p.param_point_sprite_color_->release();
						}
					}
					break;
					default:
						cgogn_assert_not_reached("");
					}
				}
			}

			float64 remain = md.outlined_until_ - App::frame_time_;
			if (remain > 0 && p.vertex_position_vbo_)
			{
				rendering::GLColor color{0.9f, 0.9f, 0.1f, 1};
				color *= float(remain * 2);
				if (!md.is_primitive_uptodate(rendering::TRIANGLES))
					md.init_primitives(rendering::TRIANGLES);
				//outline_engine_->draw(p.vertex_position_vbo_, md.mesh_render(), proj_matrix, view_matrix, color);
			}
		}
	}
	
	void left_panel() override
	{
		bool need_update = false;

		if (app_.nb_views() > 1)
			imgui_view_selector(this, selected_view_, [&](View* v) { selected_view_ = v; });

		imgui_mesh_selector(mesh_provider_, selected_mesh_, "Surface", [&](MESH& m) {
			selected_mesh_ = &m;
			mesh_provider_->mesh_data(m).outlined_until_ = App::frame_time_ + 1.0;
		});

		if (selected_view_ && selected_mesh_)
		{
			Parameters& p = parameters_[selected_view_][selected_mesh_];

			imgui_combo_attribute<Vertex, Vec3>(*selected_mesh_, p.vertex_position_, "Position",
												[&](const std::shared_ptr<Attribute<Vec3>>& attribute) {
													set_vertex_position(*selected_view_, *selected_mesh_, attribute);
												});

			ImGui::Separator();
			need_update |= ImGui::Checkbox("Vertices", &p.render_vertices_);
			if (p.render_vertices_)
			{
				imgui_combo_attribute<Vertex, Scalar>(*selected_mesh_, p.vertex_radius_, "Radius",
													  [&](const std::shared_ptr<Attribute<Scalar>>& attribute) {
														  set_vertex_radius(*selected_view_, *selected_mesh_,
																			attribute);
													  });

				if (!p.vertex_radius_)
					need_update |= ImGui::SliderFloat("Size##vertices", &(p.vertex_scale_factor_), 0.1f, 4.0f);

				ImGui::TextUnformatted("Colors");
				ImGui::BeginGroup();
				if (ImGui::RadioButton("Global##pointcolor", p.point_color_per_cell_ == GLOBAL))
				{
					p.point_color_per_cell_ = GLOBAL;
					need_update = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Per vertex##pointcolor", p.point_color_per_cell_ == PER_VERTEX))
				{
					p.point_color_per_cell_ = PER_VERTEX;
					need_update = true;
				}
				ImGui::EndGroup();

				if (p.point_color_per_cell_ == GLOBAL)
				{
					/* if (ImGui::ColorEdit3("Color##vertices", p.param_point_sprite_->color_.data(),
										  ImGuiColorEditFlags_NoInputs))
					{
						p.param_point_sprite_size_->color_ = p.param_point_sprite_->color_;
						need_update = true;
					}*/
				}
				else if (p.point_color_per_cell_ == PER_VERTEX)
				{
					imgui_combo_attribute<Vertex, Vec3>(
						*selected_mesh_, p.vertex_point_color_, "Attribute##vectorvertexpointcolor",
						[&](const std::shared_ptr<Attribute<Vec3>>& attribute) {
							set_vertex_point_color(*selected_view_, *selected_mesh_, attribute);
						});
				}
			}

			if constexpr (has_edge_v<MESH>)
			{
				using Edge = typename mesh_traits<MESH>::Edge;

				ImGui::Separator();
				need_update |= ImGui::Checkbox("Edges", &p.render_edges_);
				if (p.render_edges_)
				{
					ImGui::TextUnformatted("Colors");
					ImGui::BeginGroup();
					if (ImGui::RadioButton("Global##edgecolor", p.edge_color_per_cell_ == GLOBAL))
					{
						p.edge_color_per_cell_ = GLOBAL;
						need_update = true;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Per edge##edgecolor", p.edge_color_per_cell_ == PER_EDGE))
					{
						p.edge_color_per_cell_ = PER_EDGE;
						need_update = true;
					}
					ImGui::EndGroup();

					if (p.edge_color_per_cell_ == GLOBAL)
					{
						need_update |= ImGui::ColorEdit3("Color##edges", p.param_bold_line_->color_.data(),
														 ImGuiColorEditFlags_NoInputs);
					}
					else if (p.edge_color_per_cell_ == PER_EDGE)
					{
						imgui_combo_attribute<Edge, Vec3>(*selected_mesh_, p.edge_color_, "Attribute##vectoredgecolor",
														  [&](const std::shared_ptr<Attribute<Vec3>>& attribute) {
															  set_edge_color(*selected_view_, *selected_mesh_,
																			 attribute);
														  });
					}

					if (ImGui::SliderFloat("Width##edges", &p.param_bold_line_->width_, 1.0f, 10.0f))
					{
						p.param_bold_line_color_->width_ = p.param_bold_line_->width_;
						need_update = true;
					}
				}
			}

			ImGui::Separator();
			need_update |= ImGui::Checkbox("Faces", &p.render_faces_);
			if (p.render_faces_)
			{
				if (ImGui::Checkbox("Ghost mode", &p.ghost_mode_))
				{
					p.param_flat_->ghost_mode_ = p.ghost_mode_;
					p.param_phong_->ghost_mode_ = p.ghost_mode_;
					need_update = true;
				}
				ImGui::TextUnformatted("Normals");
				ImGui::BeginGroup();
				if (ImGui::RadioButton("Per vertex##normal", p.normal_per_cell_ == PER_VERTEX))
				{
					p.normal_per_cell_ = PER_VERTEX;
					need_update = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Per face##normal", p.normal_per_cell_ == PER_FACE))
				{
					p.normal_per_cell_ = PER_FACE;
					need_update = true;
				}
				ImGui::EndGroup();

				if (p.normal_per_cell_ == PER_VERTEX)
				{
					imgui_combo_attribute<Vertex, Vec3>(*selected_mesh_, p.vertex_normal_, "Attribute##normal",
														[&](const std::shared_ptr<Attribute<Vec3>>& attribute) {
															set_vertex_normal(*selected_view_, *selected_mesh_,
																			  attribute);
														});
				}
				else if (p.normal_per_cell_ == PER_FACE)
				{
				}

				ImGui::TextUnformatted("Colors");
				ImGui::BeginGroup();
				if (ImGui::RadioButton("Global##color", p.color_per_cell_ == GLOBAL))
				{
					p.color_per_cell_ = GLOBAL;
					need_update = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Per vertex##color", p.color_per_cell_ == PER_VERTEX))
				{
					p.color_per_cell_ = PER_VERTEX;
					need_update = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Per face##color", p.color_per_cell_ == PER_FACE))
				{
					p.color_per_cell_ = PER_FACE;
					need_update = true;
				}
				ImGui::EndGroup();

				if (p.color_per_cell_ == GLOBAL)
				{
					if (ImGui::ColorEdit3("Front color", p.param_flat_->front_color_.data(),
										  ImGuiColorEditFlags_NoInputs))
					{
						p.param_phong_->front_color_ = p.param_flat_->front_color_;
						need_update = true;
					}
					if (ImGui::ColorEdit3("Back color", p.param_flat_->back_color_.data(),
										  ImGuiColorEditFlags_NoInputs))
					{
						p.param_phong_->back_color_ = p.param_flat_->back_color_;
						need_update = true;
					}
				}
				else if (p.color_per_cell_ == PER_VERTEX)
				{
					ImGui::BeginGroup();
					if (ImGui::RadioButton("Scalar", p.color_type_ == SCALAR))
					{
						p.color_type_ = SCALAR;
						need_update = true;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Vector", p.color_type_ == VECTOR))
					{
						p.color_type_ = VECTOR;
						need_update = true;
					}
					ImGui::EndGroup();

					if (p.color_type_ == SCALAR)
					{
						imgui_combo_attribute<Vertex, Scalar>(
							*selected_mesh_, p.vertex_scalar_, "Attribute##scalarvertexcolor",
							[&](const std::shared_ptr<Attribute<Scalar>>& attribute) {
								set_vertex_scalar(*selected_view_, *selected_mesh_, attribute);
							});
						if (ImGui::InputFloat("Scalar min##vertexcolor",
											  &p.param_flat_scalar_per_vertex_->color_map_.min_value_, 0.01f, 1.0f,
											  "%.3f"))
						{
							p.param_phong_scalar_per_vertex_->color_map_.min_value_ =
								p.param_flat_scalar_per_vertex_->color_map_.min_value_;
							need_update = true;
						}
						if (ImGui::InputFloat("Scalar max##vertexcolor",
											  &p.param_flat_scalar_per_vertex_->color_map_.max_value_, 0.01f, 1.0f,
											  "%.3f"))
						{
							p.param_phong_scalar_per_vertex_->color_map_.max_value_ =
								p.param_flat_scalar_per_vertex_->color_map_.max_value_;
							need_update = true;
						}
						if (ImGui::Checkbox("Auto update min/max##vertexcolor", &p.auto_update_vertex_scalar_min_max_))
						{
							if (p.auto_update_vertex_scalar_min_max_)
							{
								update_vertex_scalar_min_max_values(p);
								need_update = true;
							}
						}
						if (ImGui::Checkbox("Show iso lines##vertexcolor",
											&p.param_flat_scalar_per_vertex_->show_iso_lines_))
						{
							p.param_phong_scalar_per_vertex_->show_iso_lines_ =
								p.param_flat_scalar_per_vertex_->show_iso_lines_;
							need_update = true;
						}
						if (ImGui::InputInt("Nb iso lines##vertexcolor",
											&p.param_flat_scalar_per_vertex_->nb_iso_lines_))
						{
							p.param_phong_scalar_per_vertex_->nb_iso_lines_ =
								p.param_flat_scalar_per_vertex_->nb_iso_lines_;
							need_update = true;
						}
					}
					else if (p.color_type_ == VECTOR)
					{
						imgui_combo_attribute<Vertex, Vec3>(
							*selected_mesh_, p.vertex_color_, "Attribute##vectorvertexcolor",
							[&](const std::shared_ptr<Attribute<Vec3>>& attribute) {
								set_vertex_color(*selected_view_, *selected_mesh_, attribute);
							});
					}
				}
				else if (p.color_per_cell_ == PER_FACE)
				{
					ImGui::BeginGroup();
					if (ImGui::RadioButton("Scalar", p.color_type_ == SCALAR))
					{
						p.color_type_ = SCALAR;
						need_update = true;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Vector", p.color_type_ == VECTOR))
					{
						p.color_type_ = VECTOR;
						need_update = true;
					}
					ImGui::EndGroup();

					if (p.color_type_ == SCALAR)
					{
						imgui_combo_attribute<Face, Scalar>(
							*selected_mesh_, p.face_scalar_, "Attribute##scalarfacecolor",
							[&](const std::shared_ptr<Attribute<Scalar>>& attribute) {
								set_face_scalar(*selected_view_, *selected_mesh_, attribute);
							});
						if (ImGui::InputFloat("Scalar min##facecolor",
											  &p.param_flat_scalar_per_face_->color_map_.min_value_, 0.01f, 1.0f,
											  "%.3f"))
						{
							p.param_phong_scalar_per_face_->color_map_.min_value_ =
								p.param_flat_scalar_per_face_->color_map_.min_value_;
							need_update = true;
						}
						if (ImGui::InputFloat("Scalar max##facecolor",
											  &p.param_flat_scalar_per_face_->color_map_.max_value_, 0.01f, 1.0f,
											  "%.3f"))
						{
							p.param_phong_scalar_per_face_->color_map_.max_value_ =
								p.param_flat_scalar_per_face_->color_map_.max_value_;
							need_update = true;
						}
						if (ImGui::Checkbox("Auto update min/max##facecolor", &p.auto_update_face_scalar_min_max_))
						{
							if (p.auto_update_face_scalar_min_max_)
							{
								update_face_scalar_min_max_values(p);
								need_update = true;
							}
						}
					}
					else if (p.color_type_ == VECTOR)
					{
						imgui_combo_attribute<Face, Vec3>(*selected_mesh_, p.face_color_, "Attribute##vectorfacecolor",
														  [&](const std::shared_ptr<Attribute<Vec3>>& attribute) {
															  set_face_color(*selected_view_, *selected_mesh_,
																			 attribute);
														  });
					}
				}
			}

			float64 remain = mesh_provider_->mesh_data(*selected_mesh_).outlined_until_ - App::frame_time_;
			if (remain > 0)
				need_update = true;

			if (need_update)
				for (View* v : linked_views_)
					v->request_update();
		}
	}

private:
	View* selected_view_;
	const MESH* selected_mesh_;
	std::unordered_map<View*, std::unordered_map<const MESH*, Parameters>> parameters_;
	std::vector<std::shared_ptr<boost::synapse::connection>> connections_;
	std::unordered_map<const MESH*, std::vector<std::shared_ptr<boost::synapse::connection>>> mesh_connections_;
	MeshProvider<MESH>* mesh_provider_;

	rendering::Outliner* outline_engine_;

	bgfx::ProgramHandle cube_program;
};

} // namespace ui

} // namespace cgogn

#endif // CGOGN_MODULE_SURFACE_RENDER_BGFX_H_
