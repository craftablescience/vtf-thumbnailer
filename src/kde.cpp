#include "kde.h"

#include <QFile>
#include <QImage>

#include <KPluginFactory>
#include <QPixmap>
#include <QPainter>

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

    int vtfWidth = 0;
    int vtfHeight = 0;

    auto image = createThumbnail(
            request.url().toLocalFile().toUtf8().data(),
            vtfWidth,
            vtfHeight);

    if (image.empty()) {
        return KIO::ThumbnailResult::fail();
    }
    // convert to required format
    const auto img = QImage{
            reinterpret_cast<unsigned char*>(image.data()),
            vtfWidth,
            vtfHeight,
            QImage::Format_RGBA8888,
    };

    auto imgPX = QPixmap(targetWidth,targetHeight);

    const auto scaledImage = img.scaled(targetWidth,targetHeight, Qt::KeepAspectRatio);
    imgPX.fill({0,0,0,0});
    auto painter = QPainter(&imgPX);
    painter.setBackground(QColor{0,0,0,0});
    auto heightResult = targetHeight - scaledImage.height();
    auto widthResult = targetWidth - scaledImage.width();

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(static_cast<int>(static_cast<float>(widthResult) * 0.5),static_cast<int>(static_cast<float>(heightResult) * 0.5) ,scaledImage);

    const auto img2 = imgPX.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

    // done!
    return !img2.isNull() ? KIO::ThumbnailResult::pass(img2) : KIO::ThumbnailResult::fail();
}

#include <kde.moc>
