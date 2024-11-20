#include "kde.h"

#include <QFile>
#include <QImage>

#include <KPluginFactory>

#include "common.h"

K_PLUGIN_CLASS_WITH_JSON(VTFCreator, "../installer/kde/generated/plugin.json")

KIO::ThumbnailResult VTFCreator::create(const KIO::ThumbnailRequest& request) {
	// we only support local files
	if (!request.url().isLocalFile()) {
		return KIO::ThumbnailResult::fail();
	}
	// create the thumbnail data
	int targetWidth = request.targetSize().width();
	int targetHeight = request.targetSize().height();
	auto image = createThumbnail(
	        request.url().toLocalFile().toUtf8().data(),
	        targetWidth,
	        targetHeight);
	if (image.empty()) {
		return KIO::ThumbnailResult::fail();
	}
	// convert to required format
	const auto img = QImage{
		reinterpret_cast<unsigned char*>(image.data()),
		targetWidth,
		targetHeight,
		QImage::Format_RGBA8888,
	}.convertToFormat(QImage::Format_ARGB32_Premultiplied);

	// done!
	return !img.isNull() ? KIO::ThumbnailResult::pass(img) : KIO::ThumbnailResult::fail();
}

#include <kde.moc>
