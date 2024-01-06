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

#ifndef CGOGN_UI_PICTURE_H_
#define CGOGN_UI_PICTURE_H_

#include <cgogn/ui/cgogn_ui_export.h>

#include <stb_image.h>

#include <vector>

namespace cgogn
{

namespace ui
{

class Picture
{
public:
	Picture(const char* filename, int dim_x, int dim_y);
	Picture(const char* filename, int dim);
	Picture(const char* filename);
	~Picture();

	void display();
	void displayPart(int d_min_x, int d_min_y, int d_size_x, int d_size_y);
	void setSize(int dim_x, int dim_y);

	int dim_x();
	int dim_y();

	void pushGlyph(int d_min_x, int d_min_y, int d_size_x, int d_size_y);
	void displayGlyph(int index);

private:
	GLuint text_;
	int dim_x_;
	int dim_y_;

	int size_x_;
	int size_y_;

	std::vector<std::vector<int>> glyph_list_;
};

} // namespace ui

} // namespace cgogn

#endif // CGOGN_UI_PICTURE_H_