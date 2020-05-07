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

#ifndef CGOGN_RENDERING_SHADERS_BOLD_LINE_H_
#define CGOGN_RENDERING_SHADERS_BOLD_LINE_H_

#include <cgogn/rendering/cgogn_rendering_export.h>
#include <cgogn/rendering/shaders/shader_program.h>

namespace cgogn
{

namespace rendering
{

DECLARE_SHADER_CLASS(BoldLine, false, CGOGN_STR(BoldLine))

class CGOGN_RENDERING_EXPORT ShaderParamBoldLine : public ShaderParam
{
	void set_uniforms() override;

public:
	GLColor color_;
	float32 width_;
	float32 lighted_;
	GLVec4 plane_clip_;
	GLVec4 plane_clip2_;

	using ShaderType = ShaderBoldLine;

	ShaderParamBoldLine(ShaderType* sh)
		: ShaderParam(sh), color_(1, 1, 0, 1), width_(2.0f), lighted_(0.25f), plane_clip_(0, 0, 0, 0),
		  plane_clip2_(0, 0, 0, 0)
	{
	}

	inline ~ShaderParamBoldLine() override
	{
	}
};

} // namespace rendering

} // namespace cgogn

#endif // CGOGN_RENDERING_SHADERS_BOLD_LINE_H_
