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

#include <cgogn/rendering/shaders/shader_phong.h>
#include <iostream>

namespace cgogn
{

namespace rendering
{

ShaderPhong* ShaderPhong::instance_ = nullptr;

ShaderPhong::ShaderPhong()
{
	load2bgfx("vs_phong.bin", "fs_phong.bin", "shader_phong");

	create_uniforms("front_color", "back_color", "ambiant_color", "light_position", "params");
	return;

	create_uniforms("light_position", "front_color", "back_color", "ambiant_color", "specular_color",
					"specular_coef", "double_side", "ghost_mode");
}

void ShaderParamPhong::set_uniforms()
{
	shader_->set_uniforms_values_bgfx(front_color_, back_color_, ambiant_color_, light_position_, double_side_,
									  ghost_mode_);
	return;
	shader_->set_uniforms_values(light_position_, front_color_, back_color_, ambiant_color_, specular_color_,
								 specular_coef_, double_side_, ghost_mode_);
}

} // namespace rendering

} // namespace cgogn
