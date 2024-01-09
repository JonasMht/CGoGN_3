
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

#ifndef CGOGN_RENDERING_VERTEX_LAYOUT_H_
#define CGOGN_RENDERING_VERTEX_LAYOUT_H_

#include <cgogn/rendering/cgogn_rendering_export.h>
#include <cgogn/core/utils/numerics.h>


#include <bgfx/bgfx.h>

namespace cgogn
{

namespace rendering
{

class CGOGN_RENDERING_EXPORT VL
{

public:
	static bool initialized_;

	static bgfx::VertexLayout position;

	static void init_vertex_layout()
	{
		position.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
	};

	inline static void init()
	{
		if (!VL::initialized_)
		{
			VL::init_vertex_layout();
			VL::initialized_ = true;
		}
	}

	inline static bgfx::VertexLayout& layout(int32 vector_dimension)
	{
		switch (vector_dimension)
		{
		case 3:
			return position;
		default:
			return position;
		}
	}

};

	inline bool VL::initialized_ = false;
	inline bgfx::VertexLayout VL::position;

} // namespace rendering

} // namespace cgogn

#endif // CGOGN_RENDERING_VERTEX_LAYOUT_H_
