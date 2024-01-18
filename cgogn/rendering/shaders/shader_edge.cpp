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

#include <cgogn/rendering/shaders/shader_edge.h>

namespace cgogn
{

namespace rendering
{

ShaderEdge* ShaderEdge::instance_ = nullptr;

ShaderEdge::ShaderEdge()
{
	load2bgfx("vs_edge.bin", "fs_edge.bin", "edge");
	create_uniforms("color_", "ambiant_color_", "light_position_", "param1_"); //, "point_size_");
}

std::shared_ptr<bgfx::IndexBufferHandle> ShaderParamEdge::ibh()
{
	if (ibh_ == nullptr)
		ibh_ = std::make_shared<bgfx::IndexBufferHandle>();
	return ibh_;
}

void ShaderParamEdge::set_uniforms()
{
	shader_->set_uniforms_values_bgfx(color_, ambiant_color_, light_position_,
									  GLColor(line_size_, 0.0,0.0,0.0));
}

void ShaderParamEdge::set_vbo(std::shared_ptr<std::vector<bx::Vec3>> vbo)
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

void ShaderParamEdge::draw()
{
	set_uniforms();
	bgfx::setVertexBuffer(0, *vbh_);
	bgfx::setIndexBuffer(*ibh_);
	bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS |
				     BGFX_STATE_MSAA | BGFX_STATE_PT_LINES );
	bgfx::submit(0, programHandle());
}





} // namespace rendering

} // namespace cgogn
