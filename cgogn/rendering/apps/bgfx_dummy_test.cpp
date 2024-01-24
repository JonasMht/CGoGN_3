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

#include <cgogn/ui/bgfx-imgui/imgui_impl_bgfx.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <GL/gl3w.h>
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
const int window_width_ = 1280;
const int window_height_ = 720;
GLFWwindow* window_;

bgfx::DynamicVertexBufferHandle vbh;
bgfx::DynamicIndexBufferHandle ibh;


int64_t m_timeOffset;

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
	uint32_t m_abgr;

	static void init()
	{
		Pos3.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	}

	static bgfx::VertexLayout Pos3;
};
bgfx::VertexLayout Pos3Vertex::Pos3;
ImGuiContext* context_;

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

	bool err = gl3wInit() != 0;
	if (err)
		std::cerr << "Failed to initialize OpenGL loader!" << std::endl;

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
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	bgfx::setViewRect(0, 0, 0, width, height);
	context_ = ImGui::CreateContext();

	ImGui_Implbgfx_Init(255);
	ImGui_ImplGlfw_InitForOther(window_, true);
}

inline void init_mesh()
{
	Pos3Vertex::init();

	Pos3Vertex vertices[] = {
		{-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},	 {-1.0f, -1.0f, 1.0f, 0xff00ff00},
		{1.0f, -1.0f, 1.0f, 0xff00ffff},   {-1.0f, 1.0f, -1.0f, 0xffff0000}, {1.0f, 1.0f, -1.0f, 0xffff00ff},
		{-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
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

	vbh = bgfx::createDynamicVertexBuffer(bgfx::makeRef(vertices, sizeof(vertices)), Pos3Vertex::Pos3);
	ibh = bgfx::createDynamicIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));


	// set uniforms
}

const inline bgfx::Memory* load_file(std::string _filePath, std::string parent = "")
{
	// Using iostream and fstream
	namespace fs = std::filesystem;
	fs::path current_path = fs::current_path();
	while (!current_path.empty() && current_path.filename() != "bin")
	{
		current_path = current_path.parent_path();
	}

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

	bgfx::ShaderHandle vs = bgfx::createShader(load_file("vs_cubes.bin", "simple_cube"));
	bgfx::ShaderHandle fs = bgfx::createShader(load_file("fs_cubes.bin", "simple_cube"));
	bgfx::ProgramHandle program = bgfx::createProgram(vs, fs, true);

	m_timeOffset = bx::getHPCounter();

	while (!glfwWindowShouldClose(window_))
	{
		glfwPollEvents();

		// Dynamic vbh update

		Pos3Vertex vertices[] = {
		{-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},	 {-1.0f, -1.0f, 1.0f, 0xff00ff00},
		{1.0f, -1.0f, 1.0f, 0xff00ffff},   {-1.0f, 1.0f, -1.0f, 0xffff0000}, {1.0f, 1.0f, -1.0f, 0xffff00ff},
		{-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
		};
		
		bgfx::update(vbh, 0, bgfx::makeRef(vertices, sizeof(vertices)));

		// set uniform values

		const bx::Vec3 at = {0.0f, 0.0f, 0.0f};
		const bx::Vec3 eye = {0.0f, 0.0f, -35.0f};

		// Set view and projection matrix for view 0.
		{
			float view[16];
			bx::mtxLookAt(view, eye, at);

			float proj[16];
			bx::mtxProj(proj, 60.0f, float(window_width_) / float(window_height_), 0.1f, 100.0f,
						bgfx::getCaps()->homogeneousDepth);
			bgfx::setViewTransform(0, view, proj);

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(window_width_), uint16_t(window_height_));
		}

		float time = (float)((bx::getHPCounter() - m_timeOffset) / double(bx::getHPFrequency()));

		bgfx::touch(0);
		uint64_t state = 0 | BGFX_STATE_WRITE_R | BGFX_STATE_WRITE_G | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
						 BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA;

		// Submit 11x11 cubes.
		for (uint32_t yy = 0; yy < 11; ++yy)
		{
			for (uint32_t xx = 0; xx < 11; ++xx)
			{
				float mtx[16];
				bx::mtxRotateXY(mtx, time + xx * 0.21f, time + yy * 0.37f);
				mtx[12] = -15.0f + float(xx) * 3.0f;
				mtx[13] = -15.0f + float(yy) * 3.0f;
				mtx[14] = 0.0f;

				// Set model matrix for rendering.
				bgfx::setTransform(mtx);

				bgfx::setVertexBuffer(0, vbh);
				bgfx::setIndexBuffer(ibh);
				bgfx::setState(state);
				bgfx::submit(0, program);
			}
		}
		ImGui::SetCurrentContext(context_);

		ImGui_Implbgfx_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
						ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::Begin("DockSpaceWindow", nullptr, window_flags);

		ImGui::PopStyleVar(3);

		ImGui::ShowDemoWindow();
		ImGui::End();
		ImGui::Render();

		ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(window_);
		glfwSwapBuffers(window_);
		bgfx::frame();
	}

	bgfx::destroy(vs);
	bgfx::destroy(fs);
	bgfx::destroy(program);
	bgfx::destroy(vbh);
	bgfx::destroy(ibh);
	bgfx::shutdown();

	glfwDestroyWindow(window_);
	glfwTerminate();

	return 0;
}
