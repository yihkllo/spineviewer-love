#pragma once
#include "spinelove/sl_scene_renderer.h"
#include "spinelove/sdk_api.h"
#include <QColor>
#include <QHash>
#include <QImage>
#include <QSize>
#include <memory>
#include <functional>
#include "spinelove/scene_capture_request.h"

class Live2DBridge;

namespace slqt {
struct SceneTransition {
    std::vector<SlDrawCommand> before,after;
    float progress=0;
};
struct SceneSnapshot {
    QSize size;
    QColor clearColor = Qt::black;
    std::vector<SlDrawCommand> draws;
    QHash<int,SceneTransition> transitions;
    QHash<SlTextureId, QImage> textures;
    quint64 textureRevision = 0;
    std::shared_ptr<Live2DBridge> live2d;
    double animationTime = 0;
    float titleHeight = 0;
    QString live2dShaderPath;
    std::shared_ptr<SceneCaptureRequest> capture;
};

class SL_SDK_API SceneRecorder final : public SlSceneRenderer {
public:
    using TextureLoader = std::function<QImage(const QString&,bool)>;
    explicit SceneRecorder(TextureLoader loader = {});
    SlTextureId LoadTextureUtf8(const char* path, bool premultiply) override;
    SlTextureId addTexture(QImage image);
    void ReleaseTexture(SlTextureId id) noexcept override;
    void Submit(const SlDrawList& list, const std::unordered_map<std::uint64_t, SlTextureId>& map) override;
    void begin(QSize size, QColor clear);
    void sprite(SlTextureId id, const SlRect& bounds, bool premultiplied = false);
    std::shared_ptr<const SceneSnapshot> finish();
    QImage texture(SlTextureId id) const { return m_textures.value(id); }
private:
    quint64 m_revision = 0;
    QHash<SlTextureId, QImage> m_textures;
    std::shared_ptr<SceneSnapshot> m_frame;
    TextureLoader m_loader;
};
}
