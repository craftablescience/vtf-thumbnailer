#include "common.h"

#include <stdexcept>

#include <sourcepp/FS.h>
#include <vtfpp/vtfpp.h>

using namespace sourcepp;
using namespace vtfpp;

std::vector<std::byte> createThumbnail(const std::string& in, int targetWidth, int targetHeight) {
	try {
		const VTF vtf{in};
		if (!vtf) {
			throw std::runtime_error{"Invalid VTF!"};
		}
		auto data = vtf.getImageDataAsRGBA8888();
		if ((targetWidth > 0 && vtf.getWidth() != targetWidth) || (targetHeight > 0 && vtf.getHeight() != targetHeight)) {
			return ImageConversion::resizeImageData(data, ImageFormat::RGBA8888, vtf.getWidth(), targetWidth, vtf.getHeight(), targetHeight, vtf.imageDataIsSRGB(), ImageConversion::ResizeFilter::BILINEAR);
		}
		return data;
	} catch (const std::overflow_error&) {
		return {};
	} catch (const std::runtime_error&) {
		return {};
	}
}

int createThumbnail(const std::string& in, const std::string& out, int targetWidth, int targetHeight) {
	auto data = ::createThumbnail(in, targetWidth, targetHeight);
	if (data.empty()) {
		return 1;
	}
	if (out.ends_with(".jpg") || out.ends_with(".jpeg")) {
		return !fs::writeFileBuffer(out, ImageConversion::convertImageDataToFile(data, ImageFormat::RGBA8888, targetWidth, targetHeight, ImageConversion::FileFormat::JPEG));
	}
	if (out.ends_with(".png")) {
		return !fs::writeFileBuffer(out, ImageConversion::convertImageDataToFile(data, ImageFormat::RGBA8888, targetWidth, targetHeight, ImageConversion::FileFormat::PNG));
	}
	if (out.ends_with(".tga")) {
		return !fs::writeFileBuffer(out, ImageConversion::convertImageDataToFile(data, ImageFormat::RGBA8888, targetWidth, targetHeight, ImageConversion::FileFormat::TGA));
	}
	return 1;
}
