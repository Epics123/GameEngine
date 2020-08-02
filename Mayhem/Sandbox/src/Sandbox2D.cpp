#include "Sandbox2D.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


Sandbox2D::Sandbox2D()
	:Layer("Sandox2D"), mCameraController(1280.0f / 720.0f)
{

}

void Sandbox2D::onAttatch()
{
	MH_PROFILE_FUNCTION();

	mTailsTexture = Mayhem::Texture2D::create("assets/textures/Tails.png");
	mSpriteSheet = Mayhem::Texture2D::create("assets/game/textures/RPGpack_sheet_2X.png");

	mTextureStairs = Mayhem::SubTexture2D::createFromCoords(mSpriteSheet, { 7, 6 }, { 128, 128 });
	mTextureTree = Mayhem::SubTexture2D::createFromCoords(mSpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

	Mayhem::FrameBufferSpec fbSpec;
	fbSpec.Width = 1280;
	fbSpec.Height = 720;
	mFrameBuffer = Mayhem::FrameBuffer::create(fbSpec);
}

void Sandbox2D::onDetatch()
{
	MH_PROFILE_FUNCTION();
}

void Sandbox2D::onUpdate(Mayhem::Timestep ts)
{
	MH_PROFILE_FUNCTION();

	//Update
	mCameraController.onUpdate(ts);

	//Render
	Mayhem::Renderer2D::resetStats();
	{
		MH_PROFILE_SCOPE("Render Prep");
		mFrameBuffer->bind();
		Mayhem::RenderCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Mayhem::RenderCommand::clear();
	}
	
	{
		MH_PROFILE_SCOPE("Render Draw");

		static float rotation = 0.0f;
		static float texRotation = 0.0f;
		rotation += ts * 20.0f;
		texRotation += ts * -50.0f;

		Mayhem::Renderer2D::beginScene(mCameraController.getCamera());
		Mayhem::Renderer2D::drawRotatedQuad({ 1.2f, 0.0f }, { 0.8f, 0.8f }, rotation, { 0.8f, 0.2f, 0.3f, 1.0f });
		Mayhem::Renderer2D::drawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f },  { 0.8f, 0.2f, 0.3f, 1.0f });
		Mayhem::Renderer2D::drawQuad({ 0.5f, -1.0f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		Mayhem::Renderer2D::drawQuad({ 0.0f, 0.0f, -0.1f }, { 1.0f, 1.0f }, mTailsTexture);
		Mayhem::Renderer2D::drawRotatedQuad({ 2.5f, 0.0f, 0.0f }, { 1.0f, 1.0f },  texRotation, mTailsTexture);
		Mayhem::Renderer2D::endScene();

		Mayhem::Renderer2D::beginScene(mCameraController.getCamera());
		Mayhem::Renderer2D::drawQuad({ -2.5f, 0.0f, 0.5f }, { 1.0f, 1.0f }, mTextureStairs);
		Mayhem::Renderer2D::drawQuad({ -2.5f, -2.0f, 0.5f }, { 1.0f, 2.0f }, mTextureTree);
		Mayhem::Renderer2D::endScene();
		mFrameBuffer->unbind();
	}
}

void Sandbox2D::onEvent(Mayhem::Event& e)
{
	mCameraController.onEvent(e);
}

void Sandbox2D::onImGuiRender()
{
	MH_PROFILE_FUNCTION();

	static bool dockingEnabled = true;

	if (dockingEnabled)
	{
		static bool docspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->GetWorkPos());
			ImGui::SetNextWindowSize(viewport->GetWorkSize());
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background 
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &docspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("Exit")) Mayhem::Application::getInstance().close();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");

		auto stats = Mayhem::Renderer2D::getStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.getTotalVertexCount());
		ImGui::Text("Indices: %d", stats.getTotalIndexCount());

		ImGui::ColorEdit4("Square Color", glm::value_ptr(mSquareColor));

		uint32_t textureID = mFrameBuffer->getColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ 1280.0f, 720.0f });
		ImGui::End();

		//Dockspace End
		ImGui::End();
	}
}
