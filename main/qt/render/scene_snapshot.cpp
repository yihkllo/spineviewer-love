#include "spinelove/scene_snapshot.h"
#include "spinelove/texture_loader.h"
#include <QImageReader>
#include <QtMath>
#include <utility>
#include <atomic>

namespace slqt {
static std::atomic<SlTextureId> nextTextureId{1};
SceneRecorder::SceneRecorder(TextureLoader loader) : m_loader(std::move(loader)) { QImage white(1,1,QImage::Format_RGBA8888);white.fill(Qt::white);m_textures.insert(1,white);++m_revision; }
SlTextureId SceneRecorder::LoadTextureUtf8(const char* path, bool premultiply)
{
    auto img = m_loader ? m_loader(QString::fromUtf8(path),premultiply) : loadTextureImage(QString::fromUtf8(path),premultiply);
    return addTexture(std::move(img));
}
SlTextureId SceneRecorder::addTexture(QImage image)
{
    if (image.isNull()) return 0;
    const auto id = ++nextTextureId;
    m_textures.insert(id, std::move(image));
    ++m_revision;
    return id;
}
void SceneRecorder::ReleaseTexture(SlTextureId id) noexcept
{
    if (m_textures.remove(id)) ++m_revision;
}
void SceneRecorder::begin(QSize size, QColor clear)
{
    m_frame = std::make_shared<SceneSnapshot>();
    m_frame->size = size;
    m_frame->clearColor = clear;
}
void SceneRecorder::Submit(const SlDrawList& list, const std::unordered_map<std::uint64_t, SlTextureId>& map)
{
    if (!m_frame) return;
    for (auto cmd : list.commands) {
        const auto id = map.find(cmd.textureId);
        if (id == map.end()) continue;
        cmd.textureId = id->second;
        for (auto& mask : cmd.masks) {
            const auto m = map.find(mask.textureId);
            mask.textureId = m == map.end() ? 0 : m->second;
        }
        m_frame->draws.push_back(std::move(cmd));
    }
}
void SceneRecorder::sprite(SlTextureId id, const SlRect& b, bool pma)
{
    if (!m_frame || !id) return;
    SlDrawCommand c;
    c.textureId = id;
    c.premultipliedAlpha = pma;
    c.vertices.resize(4);
    c.vertices[0].pos = {b.x,b.y,0}; c.vertices[0].uv = {0,0};
    c.vertices[1].pos = {b.x+b.w,b.y,0}; c.vertices[1].uv = {1,0};
    c.vertices[2].pos = {b.x+b.w,b.y+b.h,0}; c.vertices[2].uv = {1,1};
    c.vertices[3].pos = {b.x,b.y+b.h,0}; c.vertices[3].uv = {0,1};
    c.indices = {0,1,2,2,3,0};
    m_frame->draws.push_back(std::move(c));
}
std::shared_ptr<const SceneSnapshot> SceneRecorder::finish()
{
    m_frame->textures = m_textures;
    m_frame->textureRevision = m_revision;
    return std::exchange(m_frame, {});
}
}
