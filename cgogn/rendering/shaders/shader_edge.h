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

#ifndef CGOGN_RENDERING_SHADER_EDGE_H_
#define CGOGN_RENDERING_SHADER_EDGE_H_

#include <cgogn/rendering/cgogn_rendering_export.h>
#include <cgogn/rendering/shader_program.h>

namespace cgogn
{

namespace rendering
{

DECLARE_SHADER_CLASS(Edge, false, CGOGN_STR(Edge))

class CGOGN_RENDERING_EXPORT ShaderParamEdge : public ShaderParam
{
	void set_uniforms() override;

public:
	GLColor color_;
	GLColor ambiant_color_;
	GLVec3 light_position_;
	float32 line_size_;

	using ShaderType = ShaderEdge;

	ShaderParamEdge(ShaderType* sh)
		: ShaderParam(sh), color_(1.0, 0.0, 0.0, 1), ambiant_color_(0.05f, 0.05f, 0, 1), light_position_(10, 100, 1000),
		  line_size_(2)
	{
	}

	inline ~ShaderParamEdge() override
	{
	}
	void set_vbo(std::shared_ptr<std::vector<bx::Vec3>> vbo);
	void set_ibo(int size);
	std::shared_ptr<bgfx::IndexBufferHandle> ibh();
	void draw();
	void init();
};

} // namespace rendering

} // namespace cgogn

#endif // CGOGN_RENDERING_SHADER_POINT_SPRITE_H_
