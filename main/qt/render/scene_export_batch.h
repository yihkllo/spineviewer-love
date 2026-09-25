#pragma once
#include "spinelove/scene_snapshot.h"
#include <QImage>
#include <QString>
#include <atomic>
#include <memory>
#include <vector>

namespace slqt {

struct SceneExportBatch {
    struct Frame {
        std::shared_ptr<const SceneSnapshot> snapshot;
        int live2dMotion = -1;
        bool live2dStep = false;
        float live2dAdvance = 0;
    };
    std::vector<Frame> frames;
    std::vector<QImage> images;
    QString error;
    std::atomic_bool submitted{false};
    std::atomic_bool completed{false};
    std::atomic_int remaining{0};
};

}
