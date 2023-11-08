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

#include <cgogn/ui/app.h>
#include <cgogn/ui/view.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui_internal.h>

#include <map>
#include <thread>

namespace cgogn
{

namespace ui
{

float64 App::frame_time_ = 0;

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "Glfw Error " << error << ": " << description << std::endl;
}

#ifdef CGOGN_GL43_DEBUG_MODE
template <bool NOTIF>
static void APIENTRY cgogn_gl_debug_msg(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
										const GLchar* message, const void*)
{
	unused_parameters(id, length); // id not revelant

	if (!NOTIF && severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	static std::map<GLenum, std::string> gl_dbg_str = {
		{GL_DEBUG_SOURCE_API, "Calls to the OpenGL API"},
		{GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Calls to a window-system API"},
		{GL_DEBUG_SOURCE_SHADER_COMPILER, "A compiler for a shading language"},
		{GL_DEBUG_SOURCE_THIRD_PARTY, "An application associated with OpenGL"},
		{GL_DEBUG_SOURCE_APPLICATION, "Generated by the user of this application"},
		{GL_DEBUG_SOURCE_OTHER, "Other source"},
		{GL_DEBUG_TYPE_ERROR, "ERROR"},
		{GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEPRECATED_BEHAVIOR"},
		{GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "UNDEFINED_BEHAVIOR"},
		{GL_DEBUG_TYPE_PORTABILITY, "PORTABILITY"},
		{GL_DEBUG_TYPE_PERFORMANCE, "PERFORMANCE"},
		{GL_DEBUG_TYPE_MARKER, "MAKER"},
		{GL_DEBUG_TYPE_PUSH_GROUP, "PUSH_GROUP"},
		{GL_DEBUG_TYPE_POP_GROUP, "POP_GROUP"},
		{GL_DEBUG_TYPE_OTHER, "OTHER_ERROR"}};

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		std::cerr << "\033[91m";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cerr << "\033[93m";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		std::cerr << "\033[95m";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		std::cerr << "\033[92m";
		break;
	}

	std::cerr << "================== DEBUG OPENGL ==========================" << std::endl;
	std::cerr << gl_dbg_str[source] << std::endl;
	std::cerr << gl_dbg_str[type] << std::endl;
	std::cerr << message << std::endl;
	std::cerr << "==========================================================" << std::endl;
	std::cerr << "\033[m" << std::endl;

	assert(severity != GL_DEBUG_SEVERITY_HIGH);
}

inline void enable_gl43_debug_mode(bool show_notif = false)
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	if (show_notif)
		glDebugMessageCallback(cgogn_gl_debug_msg<true>, nullptr);
	else
		glDebugMessageCallback(cgogn_gl_debug_msg<false>, nullptr);
}
#endif

float64 App::fps_ = 0.0;

App::App()
	: window_(nullptr), context_(nullptr), window_name_("CGoGN"), window_width_(512), window_height_(512),
	  framebuffer_width_(0), framebuffer_height_(0), background_color_(0.35f, 0.35f, 0.35f, 1.0f),
	  interface_scaling_(1.0f), mouse_scroll_speed_(50.0f), show_imgui_(true), show_demo_(false), current_view_(nullptr)
{
#ifdef WIN32
	{
		bool ok = false;
		// Set output mode to handle virtual terminal sequences
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut != INVALID_HANDLE_VALUE)
		{
			DWORD dwMode = 0;
			if (GetConsoleMode(hOut, &dwMode))
			{
				dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				if (SetConsoleMode(hOut, dwMode))
					ok = true;
			}
		}
		if (ok)
		{
			// std::cout << "Windows \033[41m\033[37m console \033[42m color \033[42m mode " << std::endl;
			// std::cout << "\033[40m \033[91m YEEHHH \033[92m YEEHHH \033[93m YEEHHH \033[96m YEEHHH \033[95m YEEHHH "
			// 		  << "\033[m" << std::endl;
		}
	}
#endif
	// std::cout << "Windows \033[41m\033[37m console \033[42m color \033[42m mode " << std::endl;
	// std::cout << "\033[40m \033[91m YEEHHH \033[92m YEEHHH \033[93m YEEHHH \033[96m YEEHHH \033[95m YEEHHH "
	// 		  << "\033[m" << std::endl;


	tlq_ = boost::synapse::create_thread_local_queue();

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		std::cerr << "Failed to initialize GFLW!" << std::endl;

	/*
	// GL 3.3 + GLSL 150 + Core Profile
	const char* glsl_version = "#version 150";
#ifdef CGOGN_GL43_DEBUG_MODE
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
#endif
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	*/

	window_ = glfwCreateWindow(window_width_, window_height_, window_name_.c_str(), nullptr, nullptr);
	if (window_ == nullptr)
	{
		std::cerr << "Failed to create Window!" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window_);
	glfwSwapInterval(1); // Enable vsync

	//bool err = gl3wInit() != 0;
	//if (err)
	//	std::cerr << "Failed to initialize OpenGL loader!" << std::endl;

	
	
	// BGFX init
	// Initialize BGFX
    bgfx::Init init;
    init.type = bgfx::RendererType::OpenGL;


    // Platform specific data
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	init.platformData.ndt = glfwGetX11Display();
	init.platformData.nwh = (void*)(uintptr_t)glfwGetX11Window(window_);
#elif BX_PLATFORM_OSX
	init.platformData.nwh = glfwGetCocoaWindow(window_);
#elif BX_PLATFORM_WINDOWS
	init.platformData.nwh = glfwGetWin32Window(window_);
#endif

	int width, height;
	glfwGetWindowSize(window_, &width, &height);
	init.resolution.width = (uint32_t)width;
	init.resolution.height = (uint32_t)height;
	init.resolution.reset = BGFX_RESET_VSYNC;

	std::cout << "MILSESTONE 0" << std::endl;

	if (!bgfx::init(init))
		std::cerr << "Failed to initialize BGFX!" << std::endl;
	
	std::cout << "MILSESTONE 0.1" << std::endl;
	// Set view 0 to the same dimensions as the window and to clear the color buffer.
	const bgfx::ViewId kClearView = 0;
	bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);

	std::cout << "MILSESTONE 1" << std::endl;

	IMGUI_CHECKVERSION();
	context_ = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	  // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	  // Enable Multi-Viewport / Platform Windows
	// io.ConfigDockingWithShift = false;
	// io.ConfigWindowsResizeFromEdges = true;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.Colors[ImGuiCol_WindowBg].w = 0.75f;

	std::string fontpath = std::string(CGOGN_STR(CGOGN_DATA_PATH)) + std::string("fonts/Roboto-Medium.ttf");
	/*ImFont* font = */ io.Fonts->AddFontFromFileTTF(fontpath.c_str(), 14);

	glfwSetWindowUserPointer(window_, this);

	std::cout << glGetString(GL_VENDOR) << std::endl;
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;
	std::cout << "MILSESTONE 1.1" << std::endl;

	glfwSetWindowSizeCallback(window_, [](GLFWwindow* wi, int width, int height) {
		App* that = static_cast<App*>(glfwGetWindowUserPointer(wi));

		that->window_width_ = width;
		that->window_height_ = height;
		glfwGetFramebufferSize(wi, &(that->framebuffer_width_), &(that->framebuffer_height_));

		for (const auto& v : that->views_)
			v->resize_event(that->window_width_, that->window_height_, that->framebuffer_width_,
							that->framebuffer_height_);
	});

	glfwSetMouseButtonCallback(window_, [](GLFWwindow* wi, int b, int a, int m) {
		App* that = static_cast<App*>(glfwGetWindowUserPointer(wi));

		if (ImGui::GetIO().WantCaptureMouse || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		{
			that->inputs_.mouse_buttons_ = 0;
			return;
		}

		if (that->current_view_)
		{
			that->inputs_.shift_pressed_ = (m & GLFW_MOD_SHIFT);
			that->inputs_.control_pressed_ = (m & GLFW_MOD_CONTROL);
			that->inputs_.alt_pressed_ = (m & GLFW_MOD_ALT);
			that->inputs_.meta_pressed_ = (m & GLFW_MOD_SUPER);

			double now = glfwGetTime();

			switch (a)
			{
			case GLFW_PRESS:
				that->inputs_.mouse_buttons_ |= 1 << b;
				that->current_view_->mouse_press_event(b, that->inputs_.previous_mouse_x_,
													   that->inputs_.previous_mouse_y_);
				if (now - that->inputs_.previous_click_time_ < that->inputs_.double_click_timeout_)
					that->current_view_->mouse_dbl_click_event(b, that->inputs_.previous_mouse_x_,
															   that->inputs_.previous_mouse_y_);
				that->inputs_.previous_click_time_ = now;
				break;
			case GLFW_RELEASE:
				that->inputs_.mouse_buttons_ &= ~(1 << b);
				that->current_view_->mouse_release_event(b, that->inputs_.previous_mouse_x_,
														 that->inputs_.previous_mouse_y_);
				break;
			}
		}
	});

	glfwSetCursorPosCallback(window_, [](GLFWwindow* wi, double cx, double cy) {
		App* that = static_cast<App*>(glfwGetWindowUserPointer(wi));

		if (ImGui::GetIO().WantCaptureMouse || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		{
			that->inputs_.mouse_buttons_ = 0;
			return;
		}

		ImGui::GetIO().MousePos = ImVec2(float(cx), float(cy));

		int32 px = int32(std::floor(cx));
		int32 py = int32(std::floor(cy));

		for (const auto& v : that->views_)
		{
			if (v->contains(px, that->window_height_ - py))
			{
				if (v.get() != that->current_view_)
				{
					that->inputs_.mouse_buttons_ = 0;
					that->current_view_ = v.get();
				}

				v->mouse_move_event(px, py);
			}
		}

		that->inputs_.previous_mouse_x_ = px;
		that->inputs_.previous_mouse_y_ = py;
	});

	glfwSetScrollCallback(window_, [](GLFWwindow* wi, double dx, double dy) {
		App* that = static_cast<App*>(glfwGetWindowUserPointer(wi));

		if (ImGui::GetIO().WantCaptureMouse || ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
		{
			that->inputs_.mouse_buttons_ = 0;
			return;
		}

		if (that->current_view_)
			that->current_view_->mouse_wheel_event(dx, that->mouse_scroll_speed_ * dy);
	});

	glfwSetCursorEnterCallback(window_, [](GLFWwindow* wi, int enter) {
		App* that = static_cast<App*>(glfwGetWindowUserPointer(wi));

		if (ImGui::GetIO().WantCaptureMouse || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		{
			that->inputs_.mouse_buttons_ = 0;
			return;
		}

		if (!enter)
		{
			that->inputs_.mouse_buttons_ = 0;
			that->current_view_ = nullptr;
		}
	});

	glfwSetKeyCallback(window_, [](GLFWwindow* wi, int k, int s, int a, int m) {
		unused_parameters(s);
		App* that = static_cast<App*>(glfwGetWindowUserPointer(wi));

		that->inputs_.shift_pressed_ = (m & GLFW_MOD_SHIFT);
		that->inputs_.control_pressed_ = (m & GLFW_MOD_CONTROL);
		that->inputs_.alt_pressed_ = (m & GLFW_MOD_ALT);
		that->inputs_.meta_pressed_ = (m & GLFW_MOD_SUPER);

		if (k == GLFW_KEY_ESCAPE)
		{
			that->stop();
			return;
		}

		switch (a)
		{
		case GLFW_PRESS:
			if (k == GLFW_KEY_SPACE && that->inputs_.control_pressed_)
				that->show_imgui_ = !that->show_imgui_;
			if (k == GLFW_KEY_H && that->inputs_.control_pressed_)
				that->show_demo_ = !that->show_demo_;
			else if (k == GLFW_KEY_KP_ADD && that->inputs_.control_pressed_)
			{
				that->interface_scaling_ += 0.1f;
				ImGui::GetIO().FontGlobalScale = that->interface_scaling_;
			}
			else if (k == GLFW_KEY_KP_SUBTRACT && that->inputs_.control_pressed_)
			{
				that->interface_scaling_ -= 0.1f;
				ImGui::GetIO().FontGlobalScale = that->interface_scaling_;
			}
			break;
		case GLFW_RELEASE:
			break;
		}

		if (that->current_view_)
		{
			switch (a)
			{
			case GLFW_PRESS:
				if ((k == GLFW_KEY_F) && that->inputs_.control_pressed_ && !that->inputs_.shift_pressed_)
				{
					GLFWmonitor* monitor = glfwGetPrimaryMonitor();
					const GLFWvidmode* mode = glfwGetVideoMode(monitor);
					glfwSetWindowMonitor(wi, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
					glfwSetInputMode(wi, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					return;
				}
				if ((k == GLFW_KEY_F) && that->inputs_.control_pressed_ && that->inputs_.shift_pressed_)
				{
					int count;
					GLFWmonitor** monitors = glfwGetMonitors(&count);
					if (count > 1)
					{
						const GLFWvidmode* mode = glfwGetVideoMode(monitors[1]);
						glfwSetWindowMonitor(wi, monitors[1], 0, 0, mode->width, mode->height, mode->refreshRate);
						glfwSetInputMode(wi, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					}
					else
						std::cerr << "Only one monitor" << std::endl;
					return;
				}
				if ((k == GLFW_KEY_W) && that->inputs_.control_pressed_)
				{
					glfwSetWindowMonitor(wi, nullptr, 100, 100, 1024, 1024, 0);
					glfwSetInputMode(wi, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					return;
				}
				if ((k == GLFW_KEY_S) && that->inputs_.control_pressed_)
				{
					that->current_view_->save_screenshot();
					return;
				}
				that->current_view_->key_press_event(k);
				break;
			case GLFW_RELEASE:
				that->current_view_->key_release_event(k);
				break;
			}
		}
	});

	//ImGui_ImplGlfw_InitForOpenGL(window_, true);
	//ImGui_ImplOpenGL3_Init(glsl_version);

	current_view_ = add_view();
}

App::~App()
{
}

void App::set_window_size(int32 w, int32 h)
{
	glfwSetWindowSize(window_, w, h);
}

void App::set_window_title(const std::string& name)
{
	window_name_ = name;
	if (window_)
		glfwSetWindowTitle(window_, window_name_.c_str());
}

View* App::add_view()
{
	if (uint32(views_.size()) < 4)
	{
		glfwMakeContextCurrent(window_);
		views_.push_back(std::make_unique<View>(&inputs_, "view" + std::to_string(uint32(views_.size()))));
		adapt_views_geometry();
		return views_.back().get();
	}
	return nullptr;
}

Module* App::module(const std::string& name) const
{
	auto it = std::find_if(modules_.begin(), modules_.end(), [&](Module* m) { return m->name().compare(name) == 0; });
	if (it != modules_.end())
		return *it;
	return nullptr;
}

void App::close_event()
{
	for (const auto& v : views_)
		v->close_event();

	cgogn::rendering::ShaderProgram::clean_all();
}

void App::adapt_views_geometry()
{
	switch (uint32(views_.size()))
	{
	case 1:
		views_[0]->set_view_ratio(0, 0, 1, 1);
		break;
	case 2:
		views_[0]->set_view_ratio(0, 0, 0.5, 1);
		views_[1]->set_view_ratio(0.5, 0, 0.5, 1);
		break;
	case 3:
		views_[0]->set_view_ratio(0, 0, 0.5, 0.5);
		views_[1]->set_view_ratio(0.5, 0, 0.5, 0.5);
		views_[2]->set_view_ratio(0, 0, 1, 0.5);
		break;
	case 4:
		views_[0]->set_view_ratio(0, 0, 0.5, 0.5);
		views_[1]->set_view_ratio(0.5, 0, 0.5, 0.5);
		views_[2]->set_view_ratio(0, 0.5, 0.5, 0.5);
		views_[3]->set_view_ratio(0.5, 0.5, 0.5, 0.5);
		break;
	}

	for (const auto& v : views_)
		v->resize_event(window_width_, window_height_, framebuffer_width_, framebuffer_height_);
}

void App::init_modules()
{
	for (Module* m : modules_)
		m->init();
}

int App::launch()
{
	std::cout << "MILSESTONE 2" << std::endl;
	param_frame_ = rendering::ShaderFrame2d::generate_param();
	param_frame_->width_ = 5.0f;

	int32 frame_counter = 0;
	while (!glfwWindowShouldClose(window_))
	{
		boost::synapse::poll(*tlq_);

		glfwPollEvents();
		//glfwMakeContextCurrent(window_);

		frame_time_ = glfwGetTime();
		if (++frame_counter == 50)
		{
			double now = frame_time_;
			frame_counter = 0;
			fps_ = 50 / (now - time_last_50_frames_);
			time_last_50_frames_ = now;
		}


        // Render frame
        bgfx::frame();

		/*

		//glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (const auto& v : views_)
		{
			v->draw();
			if (uint32(views_.size()) > 1)
			{
				if (v.get() == current_view_)
					param_frame_->color_ = rendering::GLColor(0.25f, 0.75f, 0.25f, 1);
				else
					param_frame_->color_ = rendering::GLColor(0.25f, 0.25f, 0.25f, 1);
				param_frame_->w_ = float(v->viewport_width());
				param_frame_->h_ = float(v->viewport_height());
				param_frame_->draw();
			}
		}

		if (show_imgui_)
		{
			ImGui::SetCurrentContext(context_);
			ImGui_ImplOpenGL3_NewFrame();
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

			if (show_demo_)
				ImGui::ShowDemoWindow();

			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("Main menu"))
				{
					if (ImGui::BeginMenu("Preferences"))
					{
						if (ImGui::ColorEdit3("Background color", background_color_.data(),
											  ImGuiColorEditFlags_NoInputs))
						{
							for (const auto& v : views_)
								v->request_update();
						}
						ImGui::InputFloat("Scroll speed", &mouse_scroll_speed_, 0.1f, 1.0f);
						if (ImGui::InputFloat("Interface scale", &interface_scaling_, 0.1f, 1.0f))
							ImGui::GetIO().FontGlobalScale = interface_scaling_;
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Views"))
					{
						for (const auto& v : views_)
						{
							if (ImGui::BeginMenu(v->name().c_str()))
							{
								if (ImGui::MenuItem("Save camera"))
									v->save_camera();
								if (ImGui::MenuItem("Restore camera"))
									v->restore_camera();
								if (ImGui::Button("Show entire scene"))
									v->show_entire_scene();
								// ImGui::Checkbox("Lock view BB", &v->scene_bb_locked_);
								ImGui::EndMenu();
							}
						}
						ImGui::EndMenu();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Quit", "[ESC]"))
						this->stop();
					ImGui::EndMenu();
				}
				for (Module* m : modules_)
				{
					ImGui::PushID(m->name().c_str());
					m->main_menu();
					ImGui::PopID();
				}
				ImGui::EndMainMenuBar();
			}

			ImGuiID dockspace_id = ImGui::GetID("DockSpaceWindow");
			ImGuiDockNodeFlags dockspace_flags =
				ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode;
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			dockspace_flags |= ImGuiDockNodeFlags_DockSpace;

			ImGuiID dockIdLeft = 0;
			ImGuiID dockIdBottom = 0;
			static bool first_render = true;

			if (first_render)
			{
				ImGui::DockBuilderRemoveNode(dockspace_id);
				ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
				ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

				dockIdLeft = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.22f, nullptr, &dockspace_id);
				dockIdBottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.15f, nullptr, &dockspace_id);

				ImGui::DockBuilderFinish(dockspace_id);
			}

			ImGui::Begin("Modules", nullptr, ImGuiWindowFlags_NoSavedSettings);
			ImGui::SetWindowSize({0, 0});
			for (Module* m : modules_)
			{
				ImGui::PushID(m->name().c_str());
				ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(255, 128, 0, 200));
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(255, 128, 0, 255));
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(255, 128, 0, 128));
				if (ImGui::CollapsingHeader(m->name().c_str()))
				{
					ImGui::PopStyleColor(3);
					m->left_panel();
				}
				else
					ImGui::PopStyleColor(3);
				ImGui::PopID();
			}
			ImGui::End();

			for (Module* m : modules_)
			{
				ImGui::PushID(m->name().c_str());
				m->popups();
				ImGui::PopID();
			}

			if (first_render)
				ImGui::DockBuilderDockWindow("Modules", dockIdLeft);

			ImGui::End();

			first_render = false;

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(window_);
		}
		*/

		glfwSwapBuffers(window_);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window_);
	glfwTerminate();
	return EXIT_SUCCESS;
}

void App::stop()
{
	close_event();
	glfwSetWindowShouldClose(window_, GLFW_TRUE);
}

} // namespace ui

} // namespace cgogn
