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
	auto image = createThumbnail(
	        request.url().toLocalFile().toUtf8().data(),
	        request.targetSize().width(),
	        request.targetSize().height());
	if (image.empty()) {
		return KIO::ThumbnailResult::fail();
	}
	// convert to required format
	QImage thumb{
		reinterpret_cast<unsigned char*>(image.data()),
		request.targetSize().width(),
		request.targetSize().height(),
		QImage::Format_RGBA8888};
	thumb = thumb.rgbSwapped();
	thumb = thumb.mirrored();
	const QImage img = thumb.convertToFormat(QImage::Format_ARGB32_Premultiplied);

	// done!
	return !img.isNull() ? KIO::ThumbnailResult::pass(img) : KIO::ThumbnailResult::fail();
}

#include <kde.moc>
