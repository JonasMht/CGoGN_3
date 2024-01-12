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

#include <cgogn/geometry/types/vector_traits.h>
#include <cgogn/rendering/shaders/shader_flat.h>

#include <bx/math.h>
#include <bx/timer.h>

namespace cgogn
{

namespace rendering
{

ShaderFlat* ShaderFlat::instance_ = nullptr;

ShaderFlat::ShaderFlat()
{

	load2bgfx("vs_flat.bin", "fs_flat.bin", "shader_flat");
	// load2bgfx("vs_cube.bin", "fs_cube.bin", "simple_cube");

	create_uniforms("front_color", "back_color", "ambiant_color", "light_position", "params");
}

std::shared_ptr<bgfx::IndexBufferHandle> ShaderParamFlat::ibh()
{
	if (ibh_ == nullptr)
		ibh_ = std::make_shared<bgfx::IndexBufferHandle>();
	return ibh_;
}

void ShaderParamFlat::set_uniforms()
{
	// les attribues bool sont envoyé en block de 4
	// exemple: (vec3 vec3 bool vec3 bool bool bool) - > (vec3 vec3 vec4(bool 0 0 0) vec3 vec4(bool bool bool 0)

	shader_->set_uniforms_values_bgfx(front_color_, back_color_, ambiant_color_, light_position_, double_side_,
									  ghost_mode_);
}

void ShaderParamFlat::set_vbo(std::shared_ptr<std::vector<bx::Vec3>> vbo)
{
	attributes_initialized_ = true;
	if (vbh_ == nullptr)
	{
		vbh_ = std::make_unique<bgfx::VertexBufferHandle>(bgfx::createVertexBuffer(
			bgfx::makeRef(vbo->data(), uint32_t(vbo->size() * sizeof(bx::Vec3))), VL::position));
	}
	else
		*vbh_ = bgfx::createVertexBuffer(bgfx::makeRef(vbo->data(), uint32_t(vbo->size() * sizeof(bx::Vec3))),
										 VL::position);
}

void ShaderParamFlat::draw()
{
	set_uniforms();
	bgfx::setVertexBuffer(0, *vbh_);
	bgfx::setIndexBuffer(*ibh_);
	bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS |
				   BGFX_STATE_PT_TRISTRIP |
				   BGFX_STATE_MSAA | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
	bgfx::submit(0, programHandle());
}

} // namespace rendering

} // namespace cgogn
