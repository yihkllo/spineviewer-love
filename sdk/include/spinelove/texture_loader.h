#pragma once
#include <QImage>
#include <QString>
#include "spinelove/sdk_api.h"

namespace slqt {
SL_SDK_API QImage loadTextureImage(const QString& path,bool premultiplyAlpha,QString* error=nullptr);
}
