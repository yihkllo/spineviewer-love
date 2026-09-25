#include "spine_scene.h"
#include "../core/viewer_controller.h"
#include <QFile>
#include <QMatrix4x4>
#include <QQuickWindow>
#include <QMouseEvent>
#include <QWheelEvent>
#include <rhi/qrhi.h>
#include <rhi/qshader.h>
#include <rhi/qrhi_platform.h>
#include "spinelove/live2d_bridge.h"
#include "scene_export_batch.h"
#include <array>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <unordered_map>

namespace slqt {
namespace {
QShader shader(const char* resource) {
    QFile f(QString::fromUtf8(resource));
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QShader::fromSerialized(f.readAll());
}
struct Vertex { float x,y,r,g,b,a,u,v,z,rhw; };
struct DrawCall {
    QRhiGraphicsPipeline* pipeline=nullptr;
    QRhiShaderResourceBindings* bindings=nullptr;
    quint32 firstIndex=0,indexCount=0;
    qint32 baseVertex=0;
};
struct Surface {
    std::unique_ptr<QRhiTexture> texture;
    std::unique_ptr<QRhiRenderPassDescriptor> pass;
    std::unique_ptr<QRhiTextureRenderTarget> target;
    std::vector<DrawCall> draws;
};
struct SurfacePool {
    std::vector<std::unique_ptr<Surface>> items;
    size_t used=0;
    int idleFrames=0;
};
struct BindingKey {
    const void *uniform,*texture,*mask;
    bool operator==(const BindingKey& o)const{return uniform==o.uniform&&texture==o.texture&&mask==o.mask;}
};
struct BindingKeyHash {
    size_t operator()(const BindingKey& k)const noexcept{
        const auto h=[](const void* p){return std::hash<const void*>{}(p);};
        return h(k.uniform)^(h(k.texture)*31)^(h(k.mask)*1000003);
    }
};
bool sameMasks(const std::vector<SlMaskDrawCommand>& a,const std::vector<SlMaskDrawCommand>& b){
    if(&a==&b)return true;
    if(a.size()!=b.size())return false;
    for(size_t i=0;i<a.size();++i){
        const auto &x=a[i],&y=b[i];
        if(x.textureId!=y.textureId||x.premultipliedAlpha!=y.premultipliedAlpha||x.vertices.size()!=y.vertices.size()||x.indices!=y.indices)return false;
        if(!x.vertices.empty()&&std::memcmp(x.vertices.data(),y.vertices.data(),x.vertices.size()*sizeof(SlVertex2D))!=0)return false;
    }
    return true;
}
class SceneRenderer final : public QQuickRhiItemRenderer {
    static constexpr int IdleTrimFrames=180;
    QRhi* m_rhi = nullptr;
    std::shared_ptr<const SceneSnapshot> m_frame;
    std::unordered_map<SlTextureId,std::unique_ptr<QRhiTexture>> m_textures;
    std::unique_ptr<QRhiSampler> m_sampler;
    std::unique_ptr<QRhiTexture> m_white;
    QShader m_vs, m_fs,m_transitionFs;
    quint64 m_revision = ~quint64(0);
    quint64 m_sourceGeneration=~quint64(0);
    bool m_whiteUploaded = false;
    struct Stream {
        std::unique_ptr<QRhiBuffer> vertex,index;
        std::vector<std::unique_ptr<QRhiBuffer>> transitions;
    };
    std::vector<Vertex> m_vertexData;
    std::vector<unsigned short> m_indexData;
    std::vector<Stream> m_streams;
    size_t m_slot=0;
    bool m_baseVertex=false;
    std::array<std::unique_ptr<QRhiBuffer>,3> m_modeUniforms;
    std::array<std::array<float,20>,3> m_modeUniformValues{};
    std::unordered_map<BindingKey,std::unique_ptr<QRhiShaderResourceBindings>,BindingKeyHash> m_bindings;
    std::unique_ptr<QRhiShaderResourceBindings> m_layoutBindings;
    std::unordered_map<int,std::unique_ptr<QRhiGraphicsPipeline>> m_mainPipelines,m_offscreenPipelines;
    QRhiRenderPassDescriptor* m_mainPass=nullptr;
    int m_mainSamples=0;
    std::unique_ptr<QRhiTexture> m_probeTexture;
    std::unique_ptr<QRhiRenderPassDescriptor> m_probePass;
    std::unique_ptr<QRhiTextureRenderTarget> m_probeTarget;
    QSize m_surfaceSize;
    SurfacePool m_maskPool,m_groupPool;
    struct UniqueMask { const std::vector<SlMaskDrawCommand>* masks; Surface* surface; };
    std::vector<UniqueMask> m_uniqueMasks;
    std::vector<Surface*> m_groupOrder;
    std::vector<DrawCall> m_mainDraws;
    std::shared_ptr<Live2DBridge> m_live2d;
    std::unique_ptr<QRhiTexture> m_liveTexture;
    quint64 m_liveNative = 0;
    double m_lastLiveTime = 0;
    std::shared_ptr<const SceneSnapshot> m_lastLiveFrame;
    QRhiReadbackResult m_readback;
    std::shared_ptr<SceneCaptureRequest> m_pendingCapture;
    std::shared_ptr<SceneExportBatch> m_batch,m_activeBatch;
    std::vector<std::unique_ptr<QRhiReadbackResult>> m_batchReadbacks;

    bool batchPending()const{return m_activeBatch&&!m_activeBatch->completed.load(std::memory_order_acquire);}

public:
    ~SceneRenderer() override {
        const bool capturePending=m_pendingCapture&&!m_pendingCapture->completed.load(std::memory_order_acquire);
        if((capturePending||batchPending())&&m_rhi)m_rhi->finish();
        if(m_pendingCapture&&!m_pendingCapture->completed.load(std::memory_order_acquire)){
            m_pendingCapture->error=QStringLiteral("The render surface closed during capture.");
            m_pendingCapture->completed.store(true,std::memory_order_release);
        }
        if(batchPending()){
            m_activeBatch->error=QStringLiteral("The render surface closed during capture.");
            m_activeBatch->completed.store(true,std::memory_order_release);
        }
        releaseAll();
        if (m_live2d) m_live2d->shutdown();
    }
private:
    void releaseAll() {
        m_mainDraws.clear();m_groupOrder.clear();m_uniqueMasks.clear();
        m_bindings.clear();m_mainPipelines.clear();m_offscreenPipelines.clear();m_layoutBindings.reset();
        m_maskPool={};m_groupPool={};m_surfaceSize={};
        m_probeTarget.reset();m_probePass.reset();m_probeTexture.reset();
        m_textures.clear();m_liveTexture.reset();m_liveNative=0;
        m_streams.clear();
        for(auto& u:m_modeUniforms)u.reset();
        m_sampler.reset();m_white.reset();m_mainPass=nullptr;m_mainSamples=0;
    }
    void initialize(QRhiCommandBuffer*) override {
        if (m_rhi == rhi()) return;
        m_lastLiveFrame.reset();
        if(m_live2d)m_live2d->shutdown();
        releaseAll();
        m_rhi = rhi(); m_revision = ~quint64(0); m_whiteUploaded=false;
        m_baseVertex=rhi()->isFeatureSupported(QRhi::BaseVertex);
        m_sampler.reset(rhi()->newSampler(QRhiSampler::Linear,QRhiSampler::Linear,QRhiSampler::None,
            QRhiSampler::ClampToEdge,QRhiSampler::ClampToEdge));
        m_sampler->create();
        m_white.reset(rhi()->newTexture(QRhiTexture::RGBA8,QSize(1,1),1));
        m_white->create();
        m_vs=shader(":/shaders/spine.vert.qsb");
        m_fs=shader(":/shaders/spine.frag.qsb");
        m_transitionFs=shader(":/shaders/scene_transition.frag.qsb");
        for(auto& u:m_modeUniforms){u.reset(rhi()->newBuffer(QRhiBuffer::Dynamic,QRhiBuffer::UniformBuffer,80));u->create();}
        m_modeUniformValues={};
        m_layoutBindings.reset(rhi()->newShaderResourceBindings());
        m_layoutBindings->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::VertexStage|QRhiShaderResourceBinding::FragmentStage,m_modeUniforms[0].get()),
            QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,m_white.get(),m_sampler.get()),
            QRhiShaderResourceBinding::sampledTexture(2,QRhiShaderResourceBinding::FragmentStage,m_white.get(),m_sampler.get())});
        m_layoutBindings->create();
        m_probeTexture.reset(rhi()->newTexture(QRhiTexture::RGBA8,QSize(1,1),1,QRhiTexture::RenderTarget));
        m_probeTexture->create();
        m_probeTarget.reset(rhi()->newTextureRenderTarget(QRhiTextureRenderTargetDescription(m_probeTexture.get())));
        m_probePass.reset(m_probeTarget->newCompatibleRenderPassDescriptor());
        m_probeTarget->setRenderPassDescriptor(m_probePass.get());
        m_probeTarget->create();
    }
    void synchronize(QQuickRhiItem* item) override {
        auto* scene=static_cast<SpineScene*>(item);
        if(m_sourceGeneration!=scene->sourceGeneration()){m_sourceGeneration=scene->sourceGeneration();m_revision=~quint64(0);}
        m_frame=scene->snapshot();
        if(!m_batch&&!scene->frameSource()&&scene->controller())m_batch=scene->controller()->takeExportBatch();
    }
    static QRhiGraphicsPipeline::TargetBlend blend(SlBlendMode mode,bool pma) {
        using P=QRhiGraphicsPipeline;
        P::TargetBlend b;
        b.enable=true; b.srcColor=pma?P::One:P::SrcAlpha; b.dstColor=P::OneMinusSrcAlpha;
        b.srcAlpha=P::One; b.dstAlpha=P::OneMinusSrcAlpha;
        if(mode==SlBlendMode::Additive){b.dstColor=P::One;b.dstAlpha=P::One;}
        else if(mode==SlBlendMode::Multiply){b.srcColor=P::DstColor;}
        else if(mode==SlBlendMode::Screen){b.srcColor=P::One;b.dstColor=P::OneMinusSrcColor;}
        return b;
    }
    QRhiTexture* texture(SlTextureId id) {
        const auto it=m_textures.find(id);
        return it==m_textures.end()?nullptr:it->second.get();
    }
    QRhiGraphicsPipeline* pipeline(SlBlendMode mode,bool pma,bool transition,bool offscreen) {
        const int key=int(mode)*2+int(pma)+(transition?16:0);
        auto& cache=offscreen?m_offscreenPipelines:m_mainPipelines;
        if(const auto it=cache.find(key);it!=cache.end())return it->second.get();
        auto p=std::unique_ptr<QRhiGraphicsPipeline>(rhi()->newGraphicsPipeline());
        p->setShaderStages({{QRhiShaderStage::Vertex,m_vs},{QRhiShaderStage::Fragment,transition?m_transitionFs:m_fs}});
        QRhiVertexInputLayout layout;
        layout.setBindings({QRhiVertexInputBinding(sizeof(Vertex))});
        layout.setAttributes({{0,0,QRhiVertexInputAttribute::Float2,0},{0,1,QRhiVertexInputAttribute::Float4,8},{0,2,QRhiVertexInputAttribute::Float2,24},{0,3,QRhiVertexInputAttribute::Float2,32}});
        p->setVertexInputLayout(layout);
        p->setShaderResourceBindings(m_layoutBindings.get());
        p->setRenderPassDescriptor(offscreen?m_probePass.get():m_mainPass);
        p->setSampleCount(offscreen?1:m_mainSamples);
        p->setTargetBlends({blend(mode,pma)});
        if(!p->create())return nullptr;
        return cache.emplace(key,std::move(p)).first->second.get();
    }
    QRhiShaderResourceBindings* bindings(QRhiBuffer* uniform,QRhiTexture* tex,QRhiTexture* mask) {
        if(!mask)mask=m_white.get();
        const BindingKey key{uniform,tex,mask};
        if(const auto it=m_bindings.find(key);it!=m_bindings.end())return it->second.get();
        auto b=std::unique_ptr<QRhiShaderResourceBindings>(rhi()->newShaderResourceBindings());
        b->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::VertexStage|QRhiShaderResourceBinding::FragmentStage,uniform),
            QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,tex,m_sampler.get()),
            QRhiShaderResourceBinding::sampledTexture(2,QRhiShaderResourceBinding::FragmentStage,mask,m_sampler.get())});
        if(!b->create())return nullptr;
        return m_bindings.emplace(key,std::move(b)).first->second.get();
    }
    std::array<float,20> uniformValues(float slot) {
        QMatrix4x4 projection;projection.ortho(0.f,float(m_frame->size.width()),float(m_frame->size.height()),0.f,-1.f,1.f);
        const QMatrix4x4 corrected=rhi()->clipSpaceCorrMatrix()*projection;
        std::array<float,20> params{};
        std::memcpy(params.data(),corrected.constData(),64);
        params[16]=float(m_frame->size.width());params[17]=float(m_frame->size.height());
        params[18]=slot;params[19]=rhi()->isYUpInFramebuffer()?1.f:0.f;
        return params;
    }
    void append(std::vector<DrawCall>& out,const std::vector<SlVertex2D>& vertices,const std::vector<unsigned short>& indices,
        QRhiGraphicsPipeline* pipe,QRhiShaderResourceBindings* srb) {
        if(!pipe||!srb||vertices.empty()||indices.empty())return;
        out.push_back({pipe,srb,quint32(m_indexData.size()),quint32(indices.size()),qint32(m_vertexData.size())});
        for(const auto& v:vertices)m_vertexData.push_back({v.pos.x,v.pos.y,v.color.r,v.color.g,v.color.b,v.color.a,v.uv.x,v.uv.y,v.pos.z,v.rhw});
        m_indexData.insert(m_indexData.end(),indices.begin(),indices.end());
    }
    Surface* acquire(SurfacePool& pool) {
        if(pool.used==pool.items.size()){
            auto s=std::make_unique<Surface>();
            s->texture.reset(rhi()->newTexture(QRhiTexture::RGBA8,m_surfaceSize,1,QRhiTexture::RenderTarget));
            if(!s->texture->create())return nullptr;
            s->target.reset(rhi()->newTextureRenderTarget(QRhiTextureRenderTargetDescription(s->texture.get())));
            s->pass.reset(s->target->newCompatibleRenderPassDescriptor());
            s->target->setRenderPassDescriptor(s->pass.get());
            if(!s->target->create())return nullptr;
            pool.items.push_back(std::move(s));
        }
        auto* s=pool.items[pool.used++].get();
        s->draws.clear();
        return s;
    }
    Surface* maskSurface(const std::vector<SlMaskDrawCommand>& masks) {
        for(const auto& u:m_uniqueMasks)if(sameMasks(*u.masks,masks))return u.surface;
        auto* s=acquire(m_maskPool);
        if(!s)return nullptr;
        for(const auto& m:masks){
            auto* tex=texture(SlTextureId(m.textureId));
            if(!tex)continue;
            append(s->draws,m.vertices,m.indices,pipeline(SlBlendMode::Normal,m.premultipliedAlpha,false,true),
                bindings(m_modeUniforms[0].get(),tex,nullptr));
        }
        m_uniqueMasks.push_back({&masks,s});
        return s;
    }
    void appendCommand(std::vector<DrawCall>& out,const SlDrawCommand& c,bool offscreen) {
        auto* tex=texture(SlTextureId(c.textureId));
        if(!tex)return;
        Surface* mask=c.masks.empty()?nullptr:maskSurface(c.masks);
        const size_t mode=mask?(c.invertedMask?2:1):0;
        append(out,c.vertices,c.indices,pipeline(c.blendMode,c.premultipliedAlpha,false,offscreen),
            bindings(m_modeUniforms[mode].get(),tex,mask?mask->texture.get():nullptr));
    }
    void trim(SurfacePool& pool) {
        if(pool.used>=pool.items.size()){pool.idleFrames=0;return;}
        if(++pool.idleFrames<IdleTrimFrames)return;
        m_bindings.clear();
        pool.items.resize(pool.used);pool.idleFrames=0;
    }
    bool upload(std::unique_ptr<QRhiBuffer>& buffer,QRhiBuffer::UsageFlags usage,const void* data,quint32 bytes,QRhiResourceUpdateBatch* batch) {
        if(!bytes)return true;
        if(!buffer||buffer->size()<bytes){
            const quint32 size=((bytes+bytes/2+4095)/4096)*4096;
            buffer.reset(rhi()->newBuffer(QRhiBuffer::Dynamic,usage,size));
            if(!buffer->create()){buffer.reset();return false;}
        }
        batch->updateDynamicBuffer(buffer.get(),0,bytes,data);
        return true;
    }
    void draw(QRhiCommandBuffer* cb,const std::vector<DrawCall>& calls) {
        auto* vertices=m_streams[m_slot].vertex.get();auto* indices=m_streams[m_slot].index.get();
        for(const auto& d:calls){
            cb->setGraphicsPipeline(d.pipeline);
            cb->setShaderResources(d.bindings);
            if(m_baseVertex){
                const QRhiCommandBuffer::VertexInput input(vertices,0);
                cb->setVertexInput(0,1,&input,indices,0,QRhiCommandBuffer::IndexUInt16);
                cb->drawIndexed(d.indexCount,1,d.firstIndex,d.baseVertex);
            }else{
                const QRhiCommandBuffer::VertexInput input(vertices,quint32(d.baseVertex)*quint32(sizeof(Vertex)));
                cb->setVertexInput(0,1,&input,indices,0,QRhiCommandBuffer::IndexUInt16);
                cb->drawIndexed(d.indexCount,1,d.firstIndex);
            }
        }
    }
    void renderSurface(QRhiCommandBuffer* cb,Surface& surface) {
        cb->beginPass(surface.target.get(),Qt::transparent,{1,0});
        cb->setViewport({0,0,float(m_surfaceSize.width()),float(m_surfaceSize.height())});
        draw(cb,surface.draws);
        cb->endPass();
    }
    bool wrapLiveTexture() {
        const quint64 handle=quint64(reinterpret_cast<quintptr>(m_live2d->nativeTexture()));
        if(!handle)return false;
        if(m_liveTexture&&m_liveNative==handle&&m_liveTexture->pixelSize()==m_live2d->textureSize())return true;
        m_bindings.clear();m_liveTexture.reset();
        m_liveTexture.reset(rhi()->newTexture(QRhiTexture::RGBA8,m_live2d->textureSize(),1));
        if(!m_liveTexture->createFrom({handle,0})){m_liveTexture.reset();m_liveNative=0;return false;}
        m_liveNative=handle;
        return true;
    }
    bool prepare(QRhiResourceUpdateBatch* batch) {
        if(!m_whiteUploaded){QImage img(1,1,QImage::Format_RGBA8888);img.fill(Qt::white);batch->uploadTexture(m_white.get(),img);m_whiteUploaded=true;}
        if(!m_frame||m_frame->size.isEmpty()||!m_vs.isValid()||!m_fs.isValid())return false;
        if(m_mainPass!=renderTarget()->renderPassDescriptor()||m_mainSamples!=renderTarget()->sampleCount()){
            m_mainPipelines.clear();m_mainPass=renderTarget()->renderPassDescriptor();m_mainSamples=renderTarget()->sampleCount();
        }
        if(m_surfaceSize!=renderTarget()->pixelSize()){
            m_bindings.clear();m_maskPool={};m_groupPool={};m_surfaceSize=renderTarget()->pixelSize();
        }
        trim(m_maskPool);trim(m_groupPool);
        if(m_revision!=m_frame->textureRevision){
            bool removed=false;
            for(auto it=m_textures.begin();it!=m_textures.end();){
                if(m_frame->textures.contains(it->first)){++it;continue;}
                if(!removed){m_bindings.clear();removed=true;}
                it=m_textures.erase(it);
            }
            for(auto it=m_frame->textures.cbegin();it!=m_frame->textures.cend();++it){
                if(m_textures.count(it.key()))continue;
                auto t=std::unique_ptr<QRhiTexture>(rhi()->newTexture(QRhiTexture::RGBA8,it.value().size(),1));
                if(t->create()){batch->uploadTexture(t.get(),it.value());m_textures.emplace(it.key(),std::move(t));}
            }
            m_revision=m_frame->textureRevision;
        }
        for(size_t mode=0;mode<m_modeUniforms.size();++mode){
            const auto values=uniformValues(float(mode));
            if(values!=m_modeUniformValues[mode]){batch->updateDynamicBuffer(m_modeUniforms[mode].get(),0,80,values.data());m_modeUniformValues[mode]=values;}
        }
        return true;
    }
    void recordFrame(QRhiCommandBuffer* cb,QRhiResourceUpdateBatch* batch,size_t slot,bool liveReady) {
        if(m_streams.size()<=slot)m_streams.resize(slot+1);
        m_slot=slot;auto& stream=m_streams[slot];
        m_vertexData.clear();m_indexData.clear();m_mainDraws.clear();m_uniqueMasks.clear();m_groupOrder.clear();
        m_maskPool.used=0;m_groupPool.used=0;
        size_t transitionCount=0;
        for(size_t i=0;i<m_frame->draws.size();++i){
            const auto& c=m_frame->draws[i];
            const auto source=m_frame->transitions.constFind(int(i));
            if(source==m_frame->transitions.cend()){appendCommand(m_mainDraws,c,false);continue;}
            if(!m_transitionFs.isValid())continue;
            if(transitionCount==stream.transitions.size()){
                auto u=std::unique_ptr<QRhiBuffer>(rhi()->newBuffer(QRhiBuffer::Dynamic,QRhiBuffer::UniformBuffer,80));
                if(!u->create())continue;
                stream.transitions.push_back(std::move(u));
            }
            auto* before=acquire(m_groupPool);auto* after=before?acquire(m_groupPool):nullptr;
            if(!after)continue;
            for(const auto& g:source->before)appendCommand(before->draws,g,true);
            for(const auto& g:source->after)appendCommand(after->draws,g,true);
            m_groupOrder.push_back(before);m_groupOrder.push_back(after);
            auto* uniform=stream.transitions[transitionCount++].get();
            const auto values=uniformValues(std::clamp(source->progress,0.f,1.f));
            batch->updateDynamicBuffer(uniform,0,80,values.data());
            append(m_mainDraws,c.vertices,c.indices,pipeline(SlBlendMode::Normal,true,true,false),
                bindings(uniform,before->texture.get(),after->texture.get()));
        }
        if(liveReady&&m_liveTexture){
            const float w=float(m_frame->size.width()),h=float(m_frame->size.height());
            std::vector<SlVertex2D> quad(4);
            quad[0].pos={0,0,0};quad[0].uv={0,0};quad[1].pos={w,0,0};quad[1].uv={1,0};
            quad[2].pos={w,h,0};quad[2].uv={1,1};quad[3].pos={0,h,0};quad[3].uv={0,1};
            append(m_mainDraws,quad,{0,1,2,2,3,0},pipeline(SlBlendMode::Normal,true,false,false),
                bindings(m_modeUniforms[0].get(),m_liveTexture.get(),nullptr));
        }
        const bool streamed=!m_vertexData.empty()
            &&upload(stream.vertex,QRhiBuffer::VertexBuffer,m_vertexData.data(),quint32(m_vertexData.size()*sizeof(Vertex)),batch)
            &&upload(stream.index,QRhiBuffer::IndexBuffer,m_indexData.data(),quint32(m_indexData.size()*sizeof(unsigned short)),batch);
        cb->resourceUpdate(batch);
        if(streamed){
            for(const auto& mask:m_uniqueMasks)renderSurface(cb,*mask.surface);
            for(auto* group:m_groupOrder)renderSurface(cb,*group);
        }
        cb->beginPass(renderTarget(),m_frame->clearColor,{1,0});
        const auto sz=renderTarget()->pixelSize();cb->setViewport({0,0,float(sz.width()),float(sz.height())});
        if(streamed)draw(cb,m_mainDraws);
        cb->endPass();
    }
    void renderBatch(QRhiCommandBuffer* cb) {
        auto job=std::exchange(m_batch,{});
        const size_t count=job->frames.size();
        job->images.assign(count,{});
        m_activeBatch=job;
        m_batchReadbacks.clear();
        if(count==0){job->error=QStringLiteral("The export batch is empty.");job->completed.store(true,std::memory_order_release);return;}
        job->remaining.store(int(count),std::memory_order_relaxed);
        const auto settle=[job]{if(job->remaining.fetch_sub(1,std::memory_order_acq_rel)==1)job->completed.store(true,std::memory_order_release);};
        const auto fail=[&](const QString& error){if(job->error.isEmpty())job->error=error;settle();};
        const bool flip=rhi()->isYUpInFramebuffer();
        quint64 revision=0;QSize size;
        for(size_t i=0;i<count;++i){
            const auto& frame=job->frames[i];
            m_frame=frame.snapshot;
            if(!m_frame||(i>0&&(m_frame->textureRevision!=revision||m_frame->size!=size))){
                fail(QStringLiteral("The export frame changed during rendering."));continue;
            }
            bool liveReady=false;
            if(m_frame->live2d){
#if defined(Q_OS_WIN)
                m_live2d=m_frame->live2d;
                const int height=std::max(1,m_frame->size.height());
                cb->beginExternal();
                const bool drawn=rhi()->backend()==QRhi::D3D11&&m_live2d->renderExportFrame(frame.live2dMotion,frame.live2dStep,
                    frame.live2dAdvance,m_frame->size.width(),m_frame->size.height(),0,-m_frame->titleHeight/height);
                cb->endExternal();
                liveReady=drawn&&(i==0||m_liveTexture)&&wrapLiveTexture();
                if(liveReady){m_lastLiveFrame=m_frame;m_lastLiveTime=m_frame->animationTime;}
#endif
                if(!liveReady){fail(QStringLiteral("Live2D export frame could not be rendered."));continue;}
            }
            auto* batch=rhi()->nextResourceUpdateBatch();
            if(i==0){
                if(!prepare(batch)){
                    batch->release();
                    for(size_t rest=0;rest<count;++rest)fail(QStringLiteral("The render stage is not ready for capture."));
                    return;
                }
                revision=m_frame->textureRevision;size=m_frame->size;
            }
            recordFrame(cb,batch,i,liveReady);
            auto* result=m_batchReadbacks.emplace_back(std::make_unique<QRhiReadbackResult>()).get();
            result->completed=[job,i,result,flip,settle]{
                const QSize pixels=result->pixelSize;
                if(pixels.isEmpty()||result->data.size()<qsizetype(pixels.width())*pixels.height()*4){
                    if(job->error.isEmpty())job->error=QStringLiteral("GPU returned an incomplete stage image.");
                }else{
                    auto* bytes=new QByteArray(std::move(result->data));
                    QImage image(reinterpret_cast<const uchar*>(bytes->constData()),pixels.width(),pixels.height(),
                        qsizetype(pixels.width())*4,QImage::Format_RGBA8888_Premultiplied,
                        [](void* data){delete static_cast<QByteArray*>(data);},bytes);
                    job->images[i]=flip?image.mirrored(false,true):std::move(image);
                }
                settle();
            };
            auto* readbackBatch=rhi()->nextResourceUpdateBatch();
            readbackBatch->readBackTexture(QRhiReadbackDescription(resolveTexture()?resolveTexture():colorTexture()),result);
            cb->resourceUpdate(readbackBatch);
        }
    }
    void render(QRhiCommandBuffer* cb) override {
        if(m_batch&&!batchPending()){
            if(!m_batch->submitted.exchange(true)){renderBatch(cb);if(batchPending())update();return;}
            m_batch.reset();
        }
        bool liveReady=false;
        if(m_frame&&m_frame->live2d){
            m_live2d=m_frame->live2d;
#if defined(Q_OS_WIN)
            if(rhi()->backend()==QRhi::D3D11){
                const auto* native=static_cast<const QRhiD3D11NativeHandles*>(rhi()->nativeHandles());
                cb->beginExternal();
                const bool ready=m_live2d->initialize(static_cast<ID3D11Device*>(native->dev),static_cast<ID3D11DeviceContext*>(native->context),m_frame->live2dShaderPath);
                const float dt=m_frame->capture?0.f:float(std::clamp(m_frame->animationTime-m_lastLiveTime,0.0,0.1));
                if(ready){
                    const bool alreadyRendered=m_lastLiveFrame==m_frame&&m_live2d->nativeTexture()
                        &&m_live2d->textureSize()==m_frame->size;
                    liveReady=alreadyRendered||m_live2d->render(dt,m_frame->size.width(),m_frame->size.height(),0,-m_frame->titleHeight/std::max(1,m_frame->size.height()));
                    if(liveReady)m_lastLiveFrame=m_frame;
                }
                m_lastLiveTime=m_frame->animationTime;
                cb->endExternal();
                if(liveReady)liveReady=wrapLiveTexture();
            }else m_live2d->initialize(nullptr,nullptr,{});
#else
            m_live2d->initialize(nullptr,nullptr,{});
#endif
        }else if(m_frame){
            if(m_live2d){cb->beginExternal();m_live2d->processPendingCommands();cb->endExternal();}
            m_lastLiveTime=m_frame->animationTime;
        }
        auto* batch=rhi()->nextResourceUpdateBatch();
        if(!prepare(batch)){
            cb->beginPass(renderTarget(),Qt::black,{1,0},batch);cb->endPass();return;
        }
        recordFrame(cb,batch,0,liveReady);
        if(m_frame->capture&&(!m_pendingCapture||m_pendingCapture->completed.load(std::memory_order_acquire))&&!m_frame->capture->submitted.exchange(true)){
            m_pendingCapture=m_frame->capture;
            auto request=m_pendingCapture;
            const bool flip=rhi()->isYUpInFramebuffer();
            m_readback.completed=[this,request,flip]{
                const QSize size=m_readback.pixelSize;
                if(size.isEmpty()||m_readback.data.size()<qsizetype(size.width())*size.height()*4){
                    request->error=QStringLiteral("GPU returned an incomplete stage image.");
                }else{
                    auto image=QImage(reinterpret_cast<const uchar*>(m_readback.data.constData()),size.width(),size.height(),size.width()*4,QImage::Format_RGBA8888_Premultiplied).copy();
                    if(flip)image=image.mirrored(false,true);
                    request->image=std::move(image);
                }
                request->completed.store(true,std::memory_order_release);
            };
            auto* readbackBatch=rhi()->nextResourceUpdateBatch();
            readbackBatch->readBackTexture(QRhiReadbackDescription(resolveTexture()?resolveTexture():colorTexture()),&m_readback);
            cb->resourceUpdate(readbackBatch);
        }
        if((m_pendingCapture&&!m_pendingCapture->completed.load(std::memory_order_acquire))||batchPending()||m_batch)update();
    }
};
}
SpineScene::SpineScene(QQuickItem* parent):QQuickRhiItem(parent){
    setAcceptedMouseButtons(Qt::NoButton);setAcceptHoverEvents(false);setAlphaBlending(true);
    connect(this,&QQuickItem::windowChanged,this,[this]{updateViewport();});
}
void SpineScene::setController(ViewerController* controller){
    if(m_controller==controller)return;
    if(m_controller)disconnect(m_controller,nullptr,this,nullptr);
    m_controller=controller;
    setAcceptedMouseButtons(controller?Qt::AllButtons:Qt::NoButton);setAcceptHoverEvents(controller!=nullptr);
    ++m_sourceGeneration;
    if(m_controller)connect(m_controller,&ViewerController::frameChanged,this,&QQuickItem::update);
    updateViewport();emit controllerChanged();update();
}
void SpineScene::setFrameSource(QObject* source){
    auto* provider=qobject_cast<SceneSource*>(source);if(m_frameSource==provider)return;
    if(m_frameSource)disconnect(m_frameSource,nullptr,this,nullptr);
    m_frameSource=provider;++m_sourceGeneration;
    if(provider){
        connect(provider,&SceneSource::frameChanged,this,&QQuickItem::update);
        connect(provider,&QObject::destroyed,this,[this]{++m_sourceGeneration;emit frameSourceChanged();updateViewport();update();});
    }
    updateViewport();emit frameSourceChanged();update();
}
std::shared_ptr<const SceneSnapshot> SpineScene::snapshot() const{return m_frameSource?m_frameSource->snapshot():m_controller?m_controller->snapshot():nullptr;}
QQuickRhiItemRenderer* SpineScene::createRenderer(){return new SceneRenderer;}
void SpineScene::updateViewport(){
    if(!window())return;
    const qreal dpr=window()->devicePixelRatio()*m_renderScale;
    const auto preferred=m_frameSource?m_frameSource->preferredViewport():QSize{};
    const int w=preferred.isEmpty()?std::max(1,int(std::ceil(width()*dpr))):preferred.width(),h=preferred.isEmpty()?std::max(1,int(std::ceil(height()*dpr))):preferred.height();
    setFixedColorBufferWidth(w);setFixedColorBufferHeight(h);
    if(m_frameSource)m_frameSource->setViewport(QSizeF(width(),height()),dpr);
    else if(m_controller&&!m_frameSource)m_controller->setViewport(QSizeF(width(),height()),dpr);
}
void SpineScene::setRenderScale(qreal scale){
    scale=std::isfinite(scale)?std::clamp(scale,.25,4.):1.;if(qFuzzyCompare(scale,m_renderScale))return;
    m_renderScale=scale;updateViewport();emit renderScaleChanged();update();
}
void SpineScene::geometryChange(const QRectF& now,const QRectF& before){QQuickRhiItem::geometryChange(now,before);updateViewport();}
void SpineScene::itemChange(ItemChange change,const ItemChangeData& data){
    QQuickRhiItem::itemChange(change,data);
    if(change==ItemSceneChange&&window())updateViewport();
    if(change==ItemDevicePixelRatioHasChanged)updateViewport();
}
bool SpineScene::contains(const QPointF& point)const{
    return QQuickRhiItem::contains(point)&&(!m_controller||m_frameSource||m_controller->petHitTest(point.x(),point.y()));
}
void SpineScene::mousePressEvent(QMouseEvent* e){
    if(!contains(e->position())){e->ignore();return;}
    if(!m_controller||!m_controller->desktopPetMode())forceActiveFocus(Qt::MouseFocusReason);
    if(m_controller&&!m_frameSource)m_controller->pointerPress(e->position(),e->button(),e->modifiers());e->accept();
}
void SpineScene::mouseMoveEvent(QMouseEvent* e){if(m_controller&&!m_frameSource)m_controller->pointerMove(e->position(),e->buttons(),e->modifiers());e->accept();}
void SpineScene::mouseReleaseEvent(QMouseEvent* e){if(m_controller&&!m_frameSource)m_controller->pointerRelease(e->position(),e->button(),e->modifiers());e->accept();}
void SpineScene::wheelEvent(QWheelEvent* e){if(!m_controller||m_frameSource){e->ignore();return;}m_controller->wheel(e->position(),e->angleDelta().y(),e->buttons(),e->modifiers());e->accept();}
void SpineScene::hoverMoveEvent(QHoverEvent* e){if(m_controller&&!m_frameSource)m_controller->hover(e->position());}
void SpineScene::hoverLeaveEvent(QHoverEvent*){if(m_controller&&!m_frameSource)m_controller->hover(QPointF(-1,-1));}
}
