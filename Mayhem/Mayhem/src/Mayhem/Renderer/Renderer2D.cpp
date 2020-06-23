#include "mpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Mayhem
{

	struct Renderer2DStorage
	{
		Ref<VertexArray> vertexArray;
		Ref<Shader> textureShader;
		Ref<Texture2D> whiteTexture;
	};

	static Renderer2DStorage* sData;

	void Renderer2D::init()
	{
		sData = new Renderer2DStorage();

		sData->vertexArray = VertexArray::create();

		float squareVerticies[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Ref<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::create(squareVerticies, sizeof(squareVerticies)));

		BufferLayout squareVBLayout = {
			{ ShaderDataType::Float3, "aPosition" },
			{ ShaderDataType::Float2, "aTexCoord" },
		};

		squareVB->setLayout(squareVBLayout);

		sData->vertexArray->addVertexBuffer(squareVB);

		uint32_t squareIndicies[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::create(squareIndicies, sizeof(squareIndicies) / sizeof(uint32_t)));
		sData->vertexArray->setIndexBuffer(squareIB);
		uint32_t whiteTextureData = 0xffffffff;
		sData->whiteTexture = Texture2D::create(1, 1);

		sData->whiteTexture->setData(&whiteTextureData, sizeof(uint32_t));

		sData->textureShader = Shader::create("assets/shaders/Texture.glsl");
		sData->textureShader->bind();
		sData->textureShader->setInt("uTexture", 0);
	}

	void Renderer2D::shutdown()
	{
		delete sData;
	}

	void Renderer2D::beginScene(const OrthographicCamera& camera)
	{
		sData->textureShader->bind();
		sData->textureShader->setMat4("uViewProj", camera.getViewProjMatrix());
	}

	void Renderer2D::endScene()
	{

	}

	void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		drawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		sData->textureShader->setFloat4("uColor", color);
		sData->whiteTexture->bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		sData->textureShader->setMat4("uTransform", transform);

		sData->vertexArray->bind();
		RenderCommand::drawIndexed(sData->vertexArray);
	}

	void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		drawQuad({ position.x, position.y, 0.0f }, size, texture);
	}

	void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		sData->textureShader->setFloat4("uColor", glm::vec4(1.0f));
		sData->textureShader->bind();

		texture->bind();
		
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		sData->textureShader->setMat4("uTransform", transform);

		sData->vertexArray->bind();
		RenderCommand::drawIndexed(sData->vertexArray);
	}
}