#pragma once

#include "Mayhem/Renderer/Texture.h"

namespace Mayhem
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual uint32_t getWidth() const override { return mWidth; }
		virtual uint32_t getHeight() const override { return mHeight; }

		virtual void bind(uint32_t slot = 0) const override;

	private:
		std::string mPath;
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mRendererID;
	};
}