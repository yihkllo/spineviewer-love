#pragma once
#include "spinelove/texture_loader.h"
#include <QQuickImageProvider>

namespace slqt {
class LegacyImageProvider final : public QQuickImageProvider {
public:
    LegacyImageProvider():QQuickImageProvider(QQuickImageProvider::Image){}
    QImage requestImage(const QString& id,QSize* size,const QSize&) override {
        const auto path=QString::fromUtf8(QByteArray::fromBase64(id.section('/',0,0).toLatin1(),QByteArray::Base64UrlEncoding));
        const auto image=loadTextureImage(path,false);if(size)*size=image.size();return image;
    }
};
}
