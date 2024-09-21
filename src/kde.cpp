#include "kde.h"

#include <QFile>
#include <QImage>
#include <QPointer>
#include <QtEndian>

#include <KCompressionDevice>
#include <KPluginFactory>

#include "common.h"

K_PLUGIN_CLASS_WITH_JSON(VTFCreator, "../installer/kde/generated/plugin.json")

KIO::ThumbnailResult VTFCreator::create(const KIO::ThumbnailRequest& request) {
	//request.url().toLocalFile()
	//return KIO::ThumbnailResult::fail();

    QImage thumbnail((const uchar*)imgBuffer.constData(), x, y, QImage::Format_ARGB32);
    if(request.targetSize().width() != KDE_THUMBNAIL_SIZE) {
        thumbnail = thumbnail.scaledToWidth(request.targetSize().width(), Qt::SmoothTransformation);
    }
    if(request.targetSize().height() != 128) {
        thumbnail = thumbnail.scaledToHeight(request.targetSize().height(), Qt::SmoothTransformation);
    }
    thumbnail = thumbnail.rgbSwapped();
    thumbnail = thumbnail.mirrored();
    QImage img = thumbnail.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    blendStream.device()->close();
    return !img.isNull() ? KIO::ThumbnailResult::pass(img) : KIO::ThumbnailResult::fail();
}

#include "kde.moc"
