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

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <cgogn/geometry/types/vector_traits.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <math.h>

#include <GLFW/glfw3.h>

#if BX_PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

const std::string window_name_ = "BGFX Test";
const int window_width_ = 480;
const int window_height_ = 720;
GLFWwindow* window_;

bgfx::VertexBufferHandle vbh;
bgfx::IndexBufferHandle ibh;

bgfx::UniformHandle front_color;
bgfx::UniformHandle back_color;
bgfx::UniformHandle ambient_color;
bgfx::UniformHandle light_position;
bgfx::UniformHandle params;

using Vec3_ = cgogn::geometry::Vec3;
using Vec4_ = cgogn::geometry::Vec4;
auto Vec4 = [](const float x, const float y, const float z, const float w) -> const void* {
	return static_cast<const void*>(Vec4_(x, y, z, w).data());
};
auto Vec3 = [](const float x, const float y, const float z) -> const void* {
	return static_cast<const void*>(Vec3_(x, y, z).data());
};

struct Pos3Vertex
{
	float x;
	float y;
	float z;

	static void init()
	{
		Pos3.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
	}

	static bgfx::VertexLayout Pos3;
};
bgfx::VertexLayout Pos3Vertex::Pos3;

inline void init()
{

	if (!glfwInit())
		std::cerr << "Failed to initialize GFLW!" << std::endl;

	window_ = glfwCreateWindow(window_width_, window_height_, window_name_.c_str(), nullptr, nullptr);
	if (window_ == nullptr)
	{
		std::cerr << "Failed to create Window!" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window_);
	glfwSwapInterval(1); // Enable vsync

	// BGFX init
	// Initialize BGFX
	bgfx::Init bgfx_init;
	bgfx_init.type = bgfx::RendererType::OpenGL;

	// Platform specific data
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	bgfx_init.platformData.ndt = glfwGetX11Display();
	bgfx_init.platformData.nwh = (void*)(uintptr_t)glfwGetX11Window(window_);
#elif BX_PLATFORM_OSX
	init.platformData.nwh = glfwGetCocoaWindow(window_);
#elif BX_PLATFORM_WINDOWS
	bgfx_init.platformData.nwh = glfwGetWin32Window(window_);
#endif

	int width, height;
	glfwGetWindowSize(window_, &width, &height);
	bgfx_init.resolution.width = (uint32_t)width;
	bgfx_init.resolution.height = (uint32_t)height;
	bgfx_init.resolution.reset = BGFX_RESET_VSYNC;

	if (!bgfx::init(bgfx_init))
		std::cerr << "Failed to initialize BGFX!" << std::endl;
	uint32_t background = 0xffffffff;
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, background, 1.0f, 0);
	bgfx::setViewRect(0, 0, 0, width, height);
}

inline void init_mesh()
{
	Pos3Vertex::init();

	Pos3Vertex vertices[] = {
		{-1.0f, 1.0f, 1.0f},  {1.0f, 1.0f, 1.0f},  {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, 1.0f},
		{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
	};

	uint16_t indices[] = {
		0, 1, 2,		  // 0
		1, 3, 2, 4, 6, 5, // 2
		5, 6, 7, 0, 2, 4, // 4
		4, 2, 6, 1, 5, 3, // 6
		5, 7, 3, 0, 4, 1, // 8
		4, 5, 1, 2, 3, 6, // 10
		6, 3, 7,
	};

	vbh = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(vertices)), Pos3Vertex::Pos3);
	ibh = bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));

	// set uniforms
	front_color = bgfx::createUniform("front_color", bgfx::UniformType::Vec4);
	back_color = bgfx::createUniform("back_color", bgfx::UniformType::Vec4);
	ambient_color = bgfx::createUniform("ambient_color", bgfx::UniformType::Vec4);
	light_position = bgfx::createUniform("light_position_", bgfx::UniformType::Vec4);
	params = bgfx::createUniform("params", bgfx::UniformType::Vec4);
}

const inline bgfx::Memory* load_file(std::string _filePath, std::string parent = "")
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

int main(int argc, char** argv)
{
	init();
	init_mesh();

	bgfx::ShaderHandle vs = bgfx::createShader(load_file("vs_flat.bin", "shader_flat"));
	bgfx::ShaderHandle fs = bgfx::createShader(load_file("fs_flat.bin", "shader_flat"));
	bgfx::ProgramHandle program = bgfx::createProgram(vs, fs, true);

	int64_t m_timeOffset = bx::getHPCounter();

	while (!glfwWindowShouldClose(window_))
	{
		glfwPollEvents();
		bgfx::touch(0);
		
		int m_height, m_width;
		glfwGetWindowSize(window_, &m_width, &m_height);

		float time = (float)((bx::getHPCounter() - m_timeOffset) / double(bx::getHPFrequency()));

		// Set view and projection matrix for view 0.
		float color1[4]{1.0f, 0.0f, 0.0f, 1.0f};
		float color2[4]{0.0f, 1.0f, 0.0f, 1.0f};
		float color3[4]{1.0f, 1.0f, 1.0f, 1.0f};
		float color4[4]{1.0f, 1.0f, 0.0f, 1.0f};
		bgfx::setUniform(front_color, color1);
		bgfx::setUniform(back_color, color2);
		bgfx::setUniform(ambient_color, color3);
		bgfx::setUniform(light_position, color4);
		bgfx::setUniform(params, color4);

		const bx::Vec3 at = {0.0f, 0.0f, 0.0f};
		const bx::Vec3 eye = {10*sin(time), 10*cos(time), -10.0f};

		// Set view and projection matrix for view 0.
		{
			float view[16];
			bx::mtxLookAt(view, eye, at);

			float proj[16];
			bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f,
						bgfx::getCaps()->homogeneousDepth);
			bgfx::setViewTransform(0, view, proj);

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
		}

		float transform[16];
		bx::mtxRotateXY(transform, sin(time), sin(time));
		bgfx::setTransform(transform);

		bgfx::setVertexBuffer(0, vbh);
		//bgfx::setIndexBuffer(ibh);
		bgfx::setState(0 | BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP);
		bgfx::submit(0, program);

		glfwSwapBuffers(window_);
		bgfx::frame();
	}

	bgfx::destroy(vs);
	bgfx::destroy(fs);
	bgfx::destroy(program);
	bgfx::destroy(vbh);
	bgfx::destroy(ibh);

	glfwDestroyWindow(window_);
	glfwTerminate();

	return 0;
}
