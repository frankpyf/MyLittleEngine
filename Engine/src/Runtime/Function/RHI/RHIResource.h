#pragma once

namespace rhi {
	enum class PixelFormat
	{
		Unknown			= 0,
		RGBA			= 1,
		RGBA32F			= 2,
		DEPTH			= 3
	};

	class RHITexture2D
	{
	public:
		RHITexture2D(uint32_t width, uint32_t height)
			:width_(width), height_(height) {};
		RHITexture2D(std::string_view path)
			:file_path_(path), width_(0), height_(0) {};
		virtual ~RHITexture2D() = default;
		uint32_t GetWidth() { return width_; };
		uint32_t GetHeight() { return height_; };

		virtual void* GetTextureID() const = 0;

		virtual void SetData(const void* data) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		
	protected:
		uint32_t width_ = 0;
		uint32_t height_ = 0;

		std::string file_path_{};
	};
}