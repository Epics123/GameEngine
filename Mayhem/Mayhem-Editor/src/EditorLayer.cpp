#include "EditorLayer.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mayhem/Scene/SceneSerializer.h"

#include "Mayhem/Util/PlatformUtils.h"

#include "ImGuizmo.h"

#include "Mayhem/Math/Math.h"


namespace Mayhem
{
	EditorLayer::EditorLayer()
		:Layer("EditorLayer"), mCameraController(1280.0f / 720.0f)
	{

	}

	void EditorLayer::onAttatch()
	{
		mTailsTexture = Texture2D::create("assets/textures/Tails.png");
		mSpriteSheet = Texture2D::create("assets/game/textures/RPGpack_sheet_2X.png");

		mTextureStairs = SubTexture2D::createFromCoords(mSpriteSheet, { 7, 6 }, { 128, 128 });
		mTextureTree = SubTexture2D::createFromCoords(mSpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

		FrameBufferSpec fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		mFrameBuffer = FrameBuffer::create(fbSpec);

		mActiveScene = CreateRef<Scene>();

		mEditorCamera = EditorCamera(30.0f, 1.778, 0.1f, 1000.0f);

#if 0
		//Entity
		auto square = mActiveScene->createEntity("Green Square");
		square.addComponent<SpriteRendererComponent>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});

		auto redSquare = mActiveScene->createEntity("Red Square");
		redSquare.addComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

		mSquareEntity = square;

		mCameraEntity = mActiveScene->createEntity("Camera A");
		mCameraEntity.addComponent<CameraComponent>();

		mSecondCamera = mActiveScene->createEntity("Camera B");
		auto& cc = mSecondCamera.addComponent<CameraComponent>();
		cc.Primary = false;

		class CameraController : public ScriptableEntity
		{
		public:
			void onCreate()
			{		
				auto& translation = getComponent<TransformComponent>().Translation;
				translation.x = rand() % 10 - 5.0f;
			}

			void onDestroy()
			{

			}

			void onUpdate(Timestep ts)
			{
				auto& translation = getComponent<TransformComponent>().Translation;
				float speed = 5.0f;

				if (Input::isKeyPressed(KeyCode::A))
					translation.x -= speed * ts;
				if (Input::isKeyPressed(KeyCode::D))
					translation.x += speed * ts;
				if (Input::isKeyPressed(KeyCode::W))
					translation.y += speed * ts;
				if (Input::isKeyPressed(KeyCode::S))
					translation.y -= speed * ts;
			}
		};

		mCameraEntity.addComponent<NativeScriptComponent>().bind<CameraController>();
		mSecondCamera.addComponent<NativeScriptComponent>().bind<CameraController>();
#endif
		mSceneHierarchyPanel.setContext(mActiveScene);
	}

	void EditorLayer::onDetatch()
	{

	}

	void EditorLayer::onUpdate(Timestep ts)
	{
		//Resize
		if (FrameBufferSpec spec = mFrameBuffer->getSpecification(); mViewportSize.x > 0.0f && mViewportSize.y > 0.0f &&
			(spec.Width != mViewportSize.x || spec.Height != mViewportSize.y))
		{
			mFrameBuffer->resize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
			mCameraController.onResize(mViewportSize.x, mViewportSize.y);
			mEditorCamera.setViewportSize(mViewportSize.x, mViewportSize.y);

			mActiveScene->onViewportResize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		}
		
		//Update
		if (mViewportFocused)
			mCameraController.onUpdate(ts);

		mEditorCamera.onUpdate(ts);

		//Render
		Renderer2D::resetStats();
		mFrameBuffer->bind();
		RenderCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::clear();
		
		//Update Scene
		mActiveScene->onUpdateEditor(ts, mEditorCamera);

		mFrameBuffer->unbind();
	}

	void EditorLayer::onEvent(Mayhem::Event& e)
	{
		mCameraController.onEvent(e);
		mEditorCamera.onEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.dispatchEvent<KeyPressedEvent>(MH_BIND_EVENT_FN(EditorLayer::onKeyPressed));
	}

	void EditorLayer::onImGuiRender()
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
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
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
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("New", "Ctrl+N"))
					newScene();
				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					openScene();
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					saveSceneAs();
				if (ImGui::MenuItem("Save", "Ctrl+S"))
					saveScene();

				if (ImGui::MenuItem("Exit")) Mayhem::Application::getInstance().close();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		mSceneHierarchyPanel.onImGuiRender();

		ImGui::Begin("Settings");

		auto stats = Mayhem::Renderer2D::getStats();
		ImGui::Text("Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.getTotalVertexCount());
		ImGui::Text("Indices: %d", stats.getTotalIndexCount());
		
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");
		
		mViewportFocused = ImGui::IsWindowFocused();
		mViewportHovered = ImGui::IsWindowHovered();
		Application::getInstance().getImGuiLayer()->blockEvents(!mViewportFocused && !mViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (mViewportSize != *((glm::vec2*)&viewportPanelSize))
		{
			mFrameBuffer->resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
			mViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			mCameraController.onResize(viewportPanelSize.x, viewportPanelSize.y);
		}

		//MH_WARN("Viewport Size: {0}, {1}", viewportPanelSize.x, viewportPanelSize.y);

		uint32_t textureID = mFrameBuffer->getColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ mViewportSize.x, mViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		//Gizmos
		Entity selectedEntity = mSceneHierarchyPanel.getSelectedEntity();
		if (selectedEntity && mGizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			//Camera

			//Runtime camera from entity
			//auto cameraEntity = mActiveScene->getPrimartyCameraEntity();
			//const auto& camera = cameraEntity.getComponent<CameraComponent>().Camera;
			//const glm::mat4& cameraProjection = camera.getProjection();
			//glm::mat4 cameraView = glm::inverse(cameraEntity.getComponent<TransformComponent>().getTransform());

			//Editor Camera
			const glm::mat4& cameraProjection = mEditorCamera.getProjection();
			glm::mat4 cameraView = mEditorCamera.getViewMatrix();

			//Entity Transform
			auto& tc = selectedEntity.getComponent<TransformComponent>();
			glm::mat4 transform = tc.getTransform();

			//Snapping
			bool snap = Input::isKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; //Snap to 0.5m for translation/scale
			//Snap to 45 degrees for rotation
			if (mGizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3]{ snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)mGizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), 
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, roation, scale;
				Math::decomposeTransform(transform, translation, roation, scale);

				glm::vec3 deltaRotation = roation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		//Dockspace End
		ImGui::End();

	}

	bool EditorLayer::onKeyPressed(KeyPressedEvent& e)
	{
		if (e.getRepeatCount() > 0)
			return false;

		bool control = Input::isKeyPressed(Key::LeftControl) || Input::isKeyPressed(Key::RightControl);
		bool shift = Input::isKeyPressed(Key::LeftShift) || Input::isKeyPressed(Key::RightShift);
		switch (e.getKeyCode())
		{
		case Key::N:
		{
			if (control)
				newScene();
			break;
		}
		case Key::O:
		{
			if (control)
				openScene();
			break;
		}
		case Key::S:
		{
			if (control && shift)
				saveSceneAs();
			else if (control)
				saveScene();
			break;
		}

		//Gizmos
		case Key::Q:
			mGizmoType = -1;
			break;
		case Key::W:
			mGizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case Key::E:
			mGizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case Key::R:
			mGizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}
		return true;
	}


	void EditorLayer::newScene()
	{
		mActiveScene = CreateRef<Scene>();
		mActiveScene->onViewportResize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		mSceneHierarchyPanel.setContext(mActiveScene);
	}

	void EditorLayer::openScene()
	{
		std::string filepath = FileDialogs::openFile("Mayhem Scene (*.mayhem)\0*.mayhem\0");
		mCurrentSceneFilePath = filepath;
		if (!filepath.empty())
		{
			mActiveScene = CreateRef<Scene>();
			mActiveScene->onViewportResize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
			mSceneHierarchyPanel.setContext(mActiveScene);

			SceneSerializer serializer(mActiveScene);
			serializer.deserialize(filepath);
		}
	}

	void EditorLayer::saveSceneAs()
	{
		std::string filepath = FileDialogs::saveFile("Mayhem Scene (*.mayhem)\0.*mayhem\0");
		mCurrentSceneFilePath = filepath;
		if (!filepath.empty())
		{
			SceneSerializer serializer(mActiveScene);
			serializer.serialize(filepath);
		}
	}

	void EditorLayer::saveScene()
	{
		if (!mCurrentSceneFilePath.empty())
		{
			SceneSerializer serializer(mActiveScene);
			serializer.serialize(mCurrentSceneFilePath);
		}
	}
}


