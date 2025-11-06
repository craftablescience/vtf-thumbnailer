#include "common.h"

#include <stdexcept>

#include <sourcepp/FS.h>
#include <vtfpp/vtfpp.h>

using namespace sourcepp;
using namespace vtfpp;

std::vector<std::byte> createThumbnail(const std::string& in, int& targetWidth, int& targetHeight) {
	try {
		const VTF vtf{in};
		if (!vtf) {
			return {};
		}
		auto data = vtf.getImageDataAsRGBA8888();
		if ((targetWidth > 0 && vtf.getWidthWithoutPadding() != targetWidth) || (targetHeight > 0 && vtf.getHeight() != targetHeight)) {
			if (targetWidth <= 0) {
				targetWidth = vtf.getWidthWithoutPadding();
			}
			if (targetHeight <= 0) {
				targetHeight = vtf.getHeightWithoutPadding();
			}
			return ImageConversion::resizeImageData(data, ImageFormat::RGBA8888, vtf.getWidthWithoutPadding(), targetWidth, vtf.getHeightWithoutPadding(), targetHeight, vtf.isSRGB(), ImageConversion::ResizeFilter::BILINEAR);
		}
		targetWidth = vtf.getWidthWithoutPadding();
		targetHeight = vtf.getHeightWithoutPadding();
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
		return 2;
	}
	if (out.ends_with(".jpg") || out.ends_with(".jpeg")) {
		return !fs::writeFileBuffer(out, ImageConversion::convertImageDataToFile(data, ImageFormat::RGBA8888, targetWidth, targetHeight, ImageConversion::FileFormat::JPG));
	}
	if (out.ends_with(".png")) {
		return !fs::writeFileBuffer(out, ImageConversion::convertImageDataToFile(data, ImageFormat::RGBA8888, targetWidth, targetHeight, ImageConversion::FileFormat::PNG));
	}
	if (out.ends_with(".tga")) {
		return !fs::writeFileBuffer(out, ImageConversion::convertImageDataToFile(data, ImageFormat::RGBA8888, targetWidth, targetHeight, ImageConversion::FileFormat::TGA));
	}
	return 3;
}
