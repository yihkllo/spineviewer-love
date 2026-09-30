#include "slot_outline.h"

#include <QSysInfo>
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <utility>

namespace slqt {
namespace {
constexpr int padding = 3;
constexpr int alphaThreshold = 16;
constexpr size_t maximumQuadsPerBatch = 12000;
constexpr double maximumRaster = 1024;

SlVec2 transformed(float x,float y,const SlMatrix4& matrix)
{
    const auto point=SlVec3Transform(SlVec3(x,y,0),matrix);
    return {point.x,point.y};
}

void quad(SlDrawList& draws,const SlVec2& a,const SlVec2& b,const SlVec2& c,const SlVec2& d,const SlColor& color)
{
    if(draws.commands.empty()||draws.commands.back().vertices.size()/4>=maximumQuadsPerBatch){
        SlDrawCommand command;command.textureId=1;command.blendMode=SlBlendMode::Normal;command.premultipliedAlpha=false;
        draws.commands.push_back(std::move(command));
    }
    auto& command=draws.commands.back();const auto base=static_cast<unsigned short>(command.vertices.size());
    for(const auto point:{a,b,c,d}){SlVertex2D vertex;vertex.pos={point.x,point.y,0};vertex.uv={0,0};vertex.color=color;command.vertices.push_back(vertex);}
    for(const unsigned short offset:{0,1,2,2,3,0})command.indices.push_back(static_cast<unsigned short>(base+offset));
}

void line(SlDrawList& draws,const SlVec2& a,const SlVec2& b,float thickness,const SlColor& color)
{
    const float dx=b.x-a.x,dy=b.y-a.y,length=std::sqrt(dx*dx+dy*dy);
    if(!std::isfinite(length)||length<=0.001f)return;
    const float half=std::max(1.f,thickness)*.5f,nx=-dy/length*half,ny=dx/length*half;
    quad(draws,{a.x+nx,a.y+ny},{b.x+nx,b.y+ny},{b.x-nx,b.y-ny},{a.x-nx,a.y-ny},color);
}

struct Vertex { float x,y,u,v; };
float edge(const Vertex& a,const Vertex& b,float x,float y)
{ return (b.x-a.x)*(y-a.y)-(b.y-a.y)*(x-a.x); }
bool topLeft(const Vertex& a,const Vertex& b)
{ return b.y<a.y||(b.y==a.y&&b.x>a.x); }
bool accepted(float edgeValue,bool included)
{ return edgeValue>0||(edgeValue==0&&included); }

int alphaOffset(const QImage& image)
{
    switch(image.format()){
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
        return 3;
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        return QSysInfo::ByteOrder==QSysInfo::LittleEndian?3:0;
    default:
        return -1;
    }
}

float alphaSample(const QImage& image,int offset,float u,float v)
{
    const float x=std::clamp(u*image.width()-.5f,0.f,float(image.width()-1));
    const float y=std::clamp(v*image.height()-.5f,0.f,float(image.height()-1));
    const int left=int(x),top=int(y),right=std::min(left+1,image.width()-1),bottom=std::min(top+1,image.height()-1);
    const float fx=x-left,fy=y-top;
    const auto* row0=image.constScanLine(top);const auto* row1=image.constScanLine(bottom);
    const float a=row0[left*4+offset]*(1-fx)+row0[right*4+offset]*fx;
    const float b=row1[left*4+offset]*(1-fx)+row1[right*4+offset]*fx;
    return a*(1-fy)+b*fy;
}

bool meshSpan(const ReadSlotMeshData& mesh,float& minX,float& minY,float& spanX,float& spanY)
{
    const size_t count=mesh.worldVertices.size()/2;if(count==0)return false;
    minX=mesh.worldVertices[0];minY=mesh.worldVertices[1];float maxX=minX,maxY=minY;
    for(size_t i=0;i<count;++i){
        const float x=mesh.worldVertices[i*2],y=mesh.worldVertices[i*2+1];
        if(!std::isfinite(x)||!std::isfinite(y))return false;
        minX=std::min(minX,x);maxX=std::max(maxX,x);minY=std::min(minY,y);maxY=std::max(maxY,y);
    }
    spanX=maxX-minX;spanY=maxY-minY;
    return true;
}

bool alphaAnchors(std::vector<SlotOutlineAnchor>& anchors,float& dot,const ReadSlotMeshData& mesh,const QImage& source,float thickness)
{
    anchors.clear();
    if(mesh.textureHandle<=0||source.isNull()||mesh.worldVertices.size()<4||mesh.uvs.size()<mesh.worldVertices.size()||mesh.triangles.size()<3)return false;
    const size_t count=mesh.worldVertices.size()/2;
    for(size_t i=0;i<count*2;++i)if(!std::isfinite(mesh.uvs[i]))return false;
    float minX=0,minY=0,rawX=0,rawY=0;
    if(!meshSpan(mesh,minX,minY,rawX,rawY))return false;
    const double spanX=rawX,spanY=rawY;
    if(spanX<=0||spanY<=0||spanX>4089||spanY>4089)return false;
    const double scale=std::min(1.0,maximumRaster/std::max(spanX,spanY));
    const int width=int(std::ceil(spanX*scale))+padding*2+1,height=int(std::ceil(spanY*scale))+padding*2+1;
    if(width<=0||height<=0||width>4096||height>4096)return false;
    QImage converted;const QImage* image=&source;int offset=alphaOffset(source);
    if(offset<0){converted=source.convertToFormat(QImage::Format_RGBA8888);if(converted.isNull())return false;image=&converted;offset=3;}
    std::vector<unsigned char> mask(size_t(width)*height,0);
    std::vector<int> owner(size_t(width)*height,-1);
    std::vector<Vertex> vertices;vertices.reserve(count);
    for(size_t i=0;i<count;++i)vertices.push_back({float((mesh.worldVertices[i*2]-minX)*scale+padding),float((mesh.worldVertices[i*2+1]-minY)*scale+padding),mesh.uvs[i*2],mesh.uvs[i*2+1]});
    std::vector<std::array<unsigned short,3>> ordered(mesh.triangles.size()/3);
    for(size_t i=0;i+2<mesh.triangles.size();i+=3){
        unsigned short ia=mesh.triangles[i],ib=mesh.triangles[i+1],ic=mesh.triangles[i+2];
        if(ia>=count||ib>=count||ic>=count)continue;
        Vertex a=vertices[ia],b=vertices[ib],c=vertices[ic];
        float area=edge(a,b,c.x,c.y);if(!std::isfinite(area)||std::abs(area)<1e-8f)continue;
        if(area<0){std::swap(b,c);std::swap(ib,ic);area=-area;}
        ordered[i/3]={ia,ib,ic};
        const int firstX=std::max(0,int(std::floor(std::min({a.x,b.x,c.x}))));
        const int lastX=std::min(width-1,int(std::ceil(std::max({a.x,b.x,c.x})))-1);
        const int firstY=std::max(0,int(std::floor(std::min({a.y,b.y,c.y}))));
        const int lastY=std::min(height-1,int(std::ceil(std::max({a.y,b.y,c.y})))-1);
        const bool ab=topLeft(a,b),bc=topLeft(b,c),ca=topLeft(c,a);
        const float inverse=1.f/area;
        for(int y=firstY;y<=lastY;++y)for(int x=firstX;x<=lastX;++x){
            const float px=x+.5f,py=y+.5f;
            const float ea=edge(a,b,px,py),eb=edge(b,c,px,py),ec=edge(c,a,px,py);
            if(!accepted(ea,ab)||!accepted(eb,bc)||!accepted(ec,ca))continue;
            const float u=(eb*a.u+ec*b.u+ea*c.u)*inverse,v=(eb*a.v+ec*b.v+ea*c.v)*inverse;
            const float alpha=alphaSample(*image,offset,u,v);
            const size_t at=size_t(y)*width+x;
            mask[at]=static_cast<unsigned char>(std::clamp(int(alpha+mask[at]*(1.f-alpha/255.f)+.5f),0,255));
            owner[at]=int(i/3);
        }
    }
    const auto alphaAt=[&](int x,int y){return x<0||y<0||x>=width||y>=height?0:int(mask[size_t(y)*width+x]);};
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){
        if(alphaAt(x,y)<=alphaThreshold)continue;
        if(alphaAt(x-1,y)>alphaThreshold&&alphaAt(x+1,y)>alphaThreshold&&alphaAt(x,y-1)>alphaThreshold&&alphaAt(x,y+1)>alphaThreshold)continue;
        const int triangle=owner[size_t(y)*width+x];if(triangle<0)continue;
        const auto& t=ordered[size_t(triangle)];
        const Vertex& a=vertices[t[0]];const Vertex& b=vertices[t[1]];const Vertex& c=vertices[t[2]];
        const float area=edge(a,b,c.x,c.y);if(!(area>0))continue;
        const float px=float(x),py=float(y);
        anchors.push_back({t[0],t[1],t[2],edge(b,c,px,py)/area,edge(c,a,px,py)/area,edge(a,b,px,py)/area});
    }
    dot=std::max({1.f,thickness,float(1.0/scale)});
    return !anchors.empty();
}

size_t emitAnchors(SlDrawList& draws,const std::vector<SlotOutlineAnchor>& anchors,const ReadSlotMeshData& mesh,
                   const SlMatrix4& transform,float dot,const SlColor& color)
{
    const size_t count=mesh.worldVertices.size()/2;const float* v=mesh.worldVertices.data();const float half=dot*.5f;size_t emitted=0;
    for(const auto& p:anchors){
        if(p.a>=count||p.b>=count||p.c>=count)continue;
        const float x=p.wa*v[p.a*2]+p.wb*v[p.b*2]+p.wc*v[p.c*2];
        const float y=p.wa*v[p.a*2+1]+p.wb*v[p.b*2+1]+p.wc*v[p.c*2+1];
        if(!std::isfinite(x)||!std::isfinite(y))continue;
        quad(draws,transformed(x-half,y-half,transform),transformed(x+half,y-half,transform),
             transformed(x+half,y+half,transform),transformed(x-half,y+half,transform),color);
        ++emitted;
    }
    return emitted;
}

bool triangleBoundary(SlDrawList& draws,const ReadSlotMeshData& mesh,const SlMatrix4& transform,float thickness,const SlColor& color)
{
    const size_t count=mesh.worldVertices.size()/2;if(count<2||mesh.triangles.size()<3)return false;
    struct Edge { unsigned short a,b;int hits; };std::vector<Edge> edges;
    std::map<std::pair<unsigned short,unsigned short>,size_t> lookup;
    const auto add=[&](unsigned short a,unsigned short b){if(a==b)return;if(b<a)std::swap(a,b);const auto key=std::make_pair(a,b);const auto found=lookup.find(key);if(found==lookup.end()){lookup[key]=edges.size();edges.push_back({a,b,1});}else ++edges[found->second].hits;};
    for(size_t i=0;i+2<mesh.triangles.size();i+=3){
        const auto a=mesh.triangles[i],b=mesh.triangles[i+1],c=mesh.triangles[i+2];if(a>=count||b>=count||c>=count)continue;
        add(a,b);add(b,c);add(c,a);
    }
    bool any=false;
    for(const auto& e:edges)if(e.hits==1){line(draws,transformed(mesh.worldVertices[e.a*2],mesh.worldVertices[e.a*2+1],transform),
        transformed(mesh.worldVertices[e.b*2],mesh.worldVertices[e.b*2+1],transform),thickness,color);any=true;}
    return any;
}

bool orderedHull(SlDrawList& draws,const ReadSlotMeshData& mesh,const SlMatrix4& transform,float thickness,const SlColor& color)
{
    const int vertices=int(mesh.worldVertices.size()/2);int count=0;
    if(mesh.hullLength>1){if(mesh.hullLength<=vertices)count=mesh.hullLength;else if(mesh.hullLength%2==0&&mesh.hullLength/2<=vertices)count=mesh.hullLength/2;}
    if(count<=0&&mesh.isRegion)count=vertices;if(count<2)return false;
    auto previous=transformed(mesh.worldVertices[(count-1)*2],mesh.worldVertices[(count-1)*2+1],transform);
    for(int i=0;i<count;++i){const auto current=transformed(mesh.worldVertices[i*2],mesh.worldVertices[i*2+1],transform);line(draws,previous,current,thickness,color);previous=current;}
    return true;
}

void fallback(SlotOutlineResult& result,const ReadSlotMeshData& mesh,const SlMatrix4& transform,float thickness,const SlColor& color)
{
    if(triangleBoundary(result.draws,mesh,transform,thickness,color))result.method=SlotOutlineMethod::TriangleBoundary;
    else if(orderedHull(result.draws,mesh,transform,thickness,color))result.method=SlotOutlineMethod::OrderedHull;
}

bool similarSpan(float cached,float current)
{
    if(cached<=0||current<=0)return cached==current;
    const float ratio=current/cached;
    return ratio>.8f&&ratio<1.25f;
}
}

SlotOutlineResult buildSlotOutline(const ReadSlotMeshData& mesh,const QImage& texture,
    const SlMatrix4& transform,const SlColor& color,float thickness,QSize viewport)
{
    SlotOutlineResult result;result.draws.width=viewport.width();result.draws.height=viewport.height();
    if(mesh.worldVertices.size()<4||!std::isfinite(thickness))return result;
    std::vector<SlotOutlineAnchor> anchors;float dot=0;
    if(alphaAnchors(anchors,dot,mesh,texture,thickness)){
        result.contourPointCount=emitAnchors(result.draws,anchors,mesh,transform,dot,color);
        if(result.contourPointCount){result.method=SlotOutlineMethod::AlphaContour;return result;}
    }
    fallback(result,mesh,transform,thickness,color);
    return result;
}

SlotOutlineResult SlotOutlineCache::build(const std::string& slot,const ReadSlotMeshData& mesh,const QImage& texture,
    const SlMatrix4& transform,const SlColor& color,float thickness,QSize viewport)
{
    SlotOutlineResult result;result.draws.width=viewport.width();result.draws.height=viewport.height();
    if(mesh.worldVertices.size()<4||!std::isfinite(thickness))return result;
    float minX=0,minY=0,spanX=0,spanY=0;
    if(!meshSpan(mesh,minX,minY,spanX,spanY)){fallback(result,mesh,transform,thickness,color);return result;}
    auto& entry=m_entries[slot];
    const bool reuse=entry.ready&&entry.textureHandle==mesh.textureHandle&&entry.textureKey==texture.cacheKey()&&entry.thickness==thickness
        &&similarSpan(entry.spanX,spanX)&&similarSpan(entry.spanY,spanY)&&entry.uvs==mesh.uvs&&entry.triangles==mesh.triangles;
    if(!reuse){
        entry.contour=alphaAnchors(entry.anchors,entry.dot,mesh,texture,thickness);
        entry.ready=true;entry.textureHandle=mesh.textureHandle;entry.textureKey=texture.cacheKey();entry.thickness=thickness;
        entry.spanX=spanX;entry.spanY=spanY;entry.uvs=mesh.uvs;entry.triangles=mesh.triangles;
        ++m_rasters;
    }
    if(entry.contour){
        result.contourPointCount=emitAnchors(result.draws,entry.anchors,mesh,transform,entry.dot,color);
        if(result.contourPointCount){result.method=SlotOutlineMethod::AlphaContour;return result;}
    }
    fallback(result,mesh,transform,thickness,color);
    return result;
}

}
