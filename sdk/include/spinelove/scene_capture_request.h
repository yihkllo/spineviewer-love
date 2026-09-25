#pragma once

#include <QImage>
#include <QString>
#include <atomic>

namespace slqt {

struct SceneCaptureRequest {
    std::atomic_bool submitted{false};
    std::atomic_bool completed{false};
    QImage image;
    QString error;
};

}
