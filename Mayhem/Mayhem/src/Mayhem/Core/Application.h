#pragma once

#include "Mayhem/Core/Core.h"
#include "Mayhem/Core/Window.h"
#include "Mayhem/Core/LayerStack.h"
#include "Mayhem/Events/ApplicationEvent.h"

#include "Mayhem/Core/Timestep.h"

#include "Mayhem/ImGui/ImGuiLayer.h"

namespace Mayhem
{
	class Application
	{
	public:
		Application(const std::string& name = "Mayhem App");
		virtual ~Application();

		void run();

		void onEvent(Event& e);

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);

		Window& getWindow() { return *mWindow; }

		void close();

		ImGuiLayer* getImGuiLayer() { return mImGuiLayer; }

		static Application& getInstance() { return *sInstance; }

	private:
		std::unique_ptr<Window> mWindow;
		ImGuiLayer* mImGuiLayer;
		bool mRunning = true;
		bool mMinimized = false;
		LayerStack mLayerStack;
		float mLastFrameTime = 0.0f;

	private:
		bool onWindowClosed(WindowCloseEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

	private:
		static Application* sInstance;
	};

	//To be defined in client
	Application* createApplication();
}

