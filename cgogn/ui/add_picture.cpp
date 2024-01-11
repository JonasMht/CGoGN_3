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

#include <cgogn/ui/add_picture.h>
#include <cgogn/ui/app.h>

namespace cgogn
{

namespace ui
{

Picture::Picture(const char* filename, int dim_x, int dim_y)
	: dim_x_(dim_x), dim_y_(dim_y), text_(0), size_x_(dim_x / 10.f), size_y_(dim_y / 10.f)
{
	int image_width = 0;
	int image_height = 0;

	// Charger l'image du fichier
	std::string file_path = std::string(CGOGN_STR(CGOGN_DATA_PATH)) + std::string(filename);
	unsigned char* image_data = stbi_load(file_path.c_str(), &image_width, &image_height, NULL, 4);
	if (image_data != NULL)
	{
		// Créer une texture OpenGL
		GLuint image_texture;
		glGenTextures(1, &image_texture);
		glBindTexture(GL_TEXTURE_2D, image_texture);

		// Paramètres d'affichage
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
						GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

		if (dim_x_ == 0 || dim_y_ == 0)
		{
			dim_x_ = image_width;
			dim_y_ = image_height;
			size_x_ = image_width / 10.f;
			size_y_ = image_height / 10.f;
		}

		// Upload pixels into texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim_x_, dim_y_, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		stbi_image_free(image_data);

		text_ = image_texture;
	}
}

Picture::Picture(const char* filename, int dim) : Picture(filename, dim, dim)
{
}

Picture::Picture(const char* filename) : Picture(filename, 0, 0)
{
}

Picture::~Picture()
{
}

void Picture::display()
{
	ImGui::Image((void*)(intptr_t)text_, ImVec2(dim_x_, dim_y_));
}

void Picture::setSize(int size_x, int size_y)
{
	size_x_ = size_x;
	size_y_ = size_y;
}

void Picture::displayPart(int d_min_x, int d_min_y, int d_size_x, int d_size_y)
{
	ImVec2 display_min = ImVec2(static_cast<float>(d_min_x), static_cast<float>(d_min_y));
	ImVec2 display_size = ImVec2(static_cast<float>(d_size_x), static_cast<float>(d_size_y));
	ImVec2 texture_size = ImVec2(static_cast<float>(dim_x_), static_cast<float>(dim_y_));

	// Point de départ du morceau de texture
	ImVec2 uv0 = ImVec2(display_min.x / texture_size.x, display_min.y / texture_size.y);

	// Taille de l'image par rapport au point de départ (en fonction de la taille de la texture)
	ImVec2 uv1 =
		ImVec2((display_min.x + display_size.x) / texture_size.x, (display_min.y + display_size.y) / texture_size.y);

	// Taille de l'image à afficher
	ImGui::Image((void*)(intptr_t)text_, ImVec2(size_x_, size_y_), uv0, uv1);
}

int Picture::dim_x()
{
	return dim_x_;
}

int Picture::dim_y()
{
	return dim_y_;
}

void Picture::pushGlyph(int d_min_x, int d_min_y, int d_size_x, int d_size_y)
{
	glyph_list_.push_back({d_min_x, d_min_y, d_size_x, d_size_y});
}

void Picture::displayGlyph(int index)
{
	if (glyph_list_.size() > index)
	{
		const std::vector<int> glyph = glyph_list_[index];
		displayPart(glyph[0], glyph[1], glyph[2], glyph[3]);
	}
}

} // namespace ui

} // namespace cgogn