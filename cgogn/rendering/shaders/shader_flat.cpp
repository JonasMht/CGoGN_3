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
#include <bx/math.h>

namespace cgogn
{

namespace rendering
{

ShaderFlat* ShaderFlat::instance_ = nullptr;

ShaderFlat::ShaderFlat()
{

	load2bgfx("vs_flat.bin", "fs_flat.bin", "shader_flat");
	//load2bgfx("vs_cube.bin", "fs_cube.bin", "simple_cube");

	create_uniforms("front_color", "back_color", "ambiant_color", "light_position", "params");
	
}


void ShaderParamFlat::set_uniforms()
{
	// les attribues bool sont envoy� en block de 4
	// exemple: (vec3 vec3 bool vec3 bool bool bool) - > (vec3 vec3 vec4(bool 0 0 0) vec3 vec4(bool bool bool 0) 

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
	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	// bgfx::touch(0);
	bgfx::setVertexBuffer(0, *vbh_);
	//bgfx::setIndexBuffer(ibh);
	bgfx::setState(0 | BGFX_STATE_DEFAULT | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA);
	bgfx::submit(0, programHandle());
}

} // namespace rendering

} // namespace cgogn
