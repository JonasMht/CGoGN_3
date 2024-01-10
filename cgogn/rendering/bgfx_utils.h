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

#ifndef CGOGN_RENDERING_BGFX_UTILS_H_
#define CGOGN_RENDERING_BGFX_UTILS_H_

#include <cgogn/rendering/cgogn_rendering_export.h>

#include <bgfx/bgfx.h>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace cgogn
{
namespace rendering
{

	class CGOGN_RENDERING_EXPORT BGFXUtils
{
	public:
		static const bgfx::Memory* load_file(std::string _filePath, std::string parent = "")
		{
			// Using iostream and fstream
			namespace fs = std::filesystem;
			fs::path current_path = fs::current_path();

			if (current_path.filename() == "build")
				current_path += "/stage/bin";
			
			while (!current_path.empty() && current_path.filename() != "bin")
			{
				current_path = current_path.parent_path();
			}
			std::cout << current_path << std::endl;

			fs::path file_path(current_path);

			_filePath = "shaders/" + (parent == "" ? _filePath : parent + "/" + _filePath);
			_filePath = file_path.string() + "/" + _filePath;

			// Open file
			std::ifstream file(_filePath, std::ios::in | std::ios::binary | std::ios::ate);

			// Check if file is open
			if (!file.is_open())
			{
				std::cerr << "Failed to open file: " << _filePath << std::endl;
				return nullptr;
			}

			// Get file size
			std::streampos size = file.tellg();
			// Allocate memory
			const bgfx::Memory* mem = bgfx::alloc((uint32_t)size + 1);
			// Read file
			file.seekg(0, std::ios::beg);
			file.read((char*)mem->data, size);
			// Close file
			file.close();

			// Add null terminator
			((char*)mem->data)[size] = '\0';

			return mem;
		}
	};
}
}	

#endif


