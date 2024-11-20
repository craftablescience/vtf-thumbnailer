#pragma once

#include <KIO/ThumbnailCreator>

class VTFCreator : public KIO::ThumbnailCreator {
public:
	using ThumbnailCreator::ThumbnailCreator;

	KIO::ThumbnailResult create(const KIO::ThumbnailRequest& request) override;
};
