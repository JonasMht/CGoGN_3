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

#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/geometry/types/vector_traits.h>

#include <bx/timer.h>

namespace cgogn
{

namespace rendering
{

ShaderFlat* ShaderFlat::instance_ = nullptr;

ShaderFlat::ShaderFlat()
{

	const char* vertex_shader_source = R"(
		#version 150
		uniform mat4 projection_matrix;
		uniform mat4 model_view_matrix;

		in vec3 vertex_position;
		
		out vec3 position;
		
		void main()
		{
			vec4 position4 = model_view_matrix * vec4(vertex_position, 1.0);
			position = position4.xyz;
			gl_Position = projection_matrix * position4;
		}
	)";

	const char* fragment_shader_source = R"(
		#version 150
		uniform vec4 front_color;
		uniform vec4 back_color;
		uniform vec4 ambiant_color;
		uniform vec3 light_position;
		uniform bool double_side;
		uniform bool ghost_mode;
		
		in vec3 position;

		out vec4 frag_out;

		void main()
		{
			vec3 N = normalize(cross(dFdx(position), dFdy(position)));
			vec3 L = normalize(light_position - position);
			float lambert = dot(N, L);
			if (ghost_mode)
				lambert = 0.4 * pow(1.0 - lambert, 2);
			if (gl_FrontFacing)
				frag_out = vec4(ambiant_color.rgb + lambert * front_color.rgb, front_color.a);
			else
				if (!double_side)
					discard;
				else frag_out = vec4(ambiant_color.rgb + lambert * back_color.rgb, back_color.a);
		}
	)";
	
	load2bgfx("vs_flat.bin", "fs_flat.bin", "shader_flat");
	//load2_bind(vertex_shader_source, fragment_shader_source, "vertex_position");
	// get_uniforms("front_color", "back_color", "ambiant_color", "light_position", "double_side", "ghost_mode");
	create_uniforms("front_color", "back_color", "ambiant_color", "light_position", "double_side", "ghost_mode");
	
}


void ShaderParamFlat::set_uniforms()
{
	shader_->set_uniforms_values_bgfx(front_color_, back_color_, ambiant_color_, light_position_, double_side_, ghost_mode_);
}

void ShaderParamFlat::set_vbo(std::shared_ptr<std::vector<bx::Vec3>> vbo)
{
	attributes_initialized_ = true;
	if (vbh_ == nullptr)
		vbh_ = std::make_unique<bgfx::VertexBufferHandle>(
			bgfx::createVertexBuffer(bgfx::makeRef(vbo->data(), uint32_t(vbo->size() * sizeof(bx::Vec3))),
											VL::position)
		);
	else
		*vbh_ = bgfx::createVertexBuffer(bgfx::makeRef(vbo->data(), uint32_t(vbo->size() * sizeof(bx::Vec3))), VL::position);
}


void ShaderParamFlat::draw(int w, int h)
{
	set_uniforms();
	const bx::Vec3 at = {0.0f, 0.0f, 0.0f};
	const bx::Vec3 eye = {0.0f, 0.0f, -4.0};

	// Set view and projection matrix for view 0.

	{
		float view[16];
		bx::mtxLookAt(view, eye, at);

		int m_height = h, m_width = w;
		
		float proj[16];
		bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(0, view, proj);

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
	}

	float time = (float)((bx::getHPCounter() - m_timeOffset) / double(bx::getHPFrequency()));
	float transform[16];
	bx::mtxScale(transform, 3.0f);
	bgfx::setTransform(transform);

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	// bgfx::touch(0);

	

	float vertices[] = {
		-1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
	};

	uint16_t indices[] = {
		0, 1, 2,		  // 0
		1, 3, 2, 4, 6, 5, // 2
		5, 6, 7, 0, 2, 4, // 4
		4, 2, 6, 1, 5, 3, // 6
		5, 7, 3, 0, 4, 1, // 8
		4, 5, 1, 2, 3, 6, // 10
		6, 3, 7,
	};

	bgfx::VertexLayout layout;
    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .end();


	bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(vertices)), layout);
	bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));
	

	bgfx::setVertexBuffer(0, vbh);//*vbh_);
	bgfx::setIndexBuffer(ibh);

	//bgfx::setIndexBuffer(ibh);
	bgfx::setState(0 | BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP | BGFX_STATE_WRITE_R | BGFX_STATE_WRITE_G | BGFX_STATE_WRITE_B | BGFX_STATE_WRITE_A |
				   BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA);
	bgfx::submit(0, programHandle());
}

} // namespace rendering

} // namespace cgogn
