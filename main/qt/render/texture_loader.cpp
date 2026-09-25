#include "spinelove/texture_loader.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QImageReader>
#include <limits>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace slqt {
namespace {
QImage decodeStb(const QString& path){
    QFile f(path);if(!f.open(QIODevice::ReadOnly)||f.size()>(std::numeric_limits<int>::max)())return {};
    const auto bytes=f.readAll();int w=0,h=0,channels=0;
    auto* p=stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bytes.constData()),int(bytes.size()),&w,&h,&channels,4);
    if(!p)return {};
    return QImage(p,w,h,w*4,QImage::Format_RGBA8888,[](void* pixels){stbi_image_free(pixels);},p);
}
}
QImage loadTextureImage(const QString& path,bool premultiply,QString* error){
    QImage image=decodeStb(path);
    if(image.isNull()){
        QImageReader reader(path);reader.setAutoTransform(false);image=reader.read().convertToFormat(QImage::Format_RGBA8888);
        if(image.isNull()){if(error)*error=reader.errorString();return {};}
    }
    const QFileInfo info(path);
    if(!info.suffix().isEmpty()&&!info.completeBaseName().endsWith("_alpha",Qt::CaseInsensitive)){
        const QString leaf=info.completeBaseName()+"_alpha."+info.suffix();
        QString alphaPath=info.dir().filePath(leaf);
        if(!QFileInfo::exists(alphaPath)){
            const auto names=info.dir().entryList(QDir::Files|QDir::Hidden);
            for(const auto& name:names)if(name.compare(leaf,Qt::CaseInsensitive)==0){alphaPath=info.dir().filePath(name);break;}
        }
        const auto alpha=decodeStb(alphaPath);
        if(!alpha.isNull()&&alpha.size()==image.size())for(int y=0;y<image.height();++y){
            auto* dst=image.scanLine(y);const auto* src=alpha.constScanLine(y);
            for(int x=0;x<image.width();++x)dst[x*4+3]=src[x*4];
        }
    }
    if(premultiply){
        auto* row=image.bits();
        for(int y=0;y<image.height();++y,row+=image.bytesPerLine()){
            auto* p=row;
            for(int x=0;x<image.width();++x,p+=4){
                const unsigned alpha=p[3];if(alpha==255)continue;
                if(alpha==0){p[0]=p[1]=p[2]=0;continue;}
                p[0]=uchar((unsigned(p[0])*alpha+127u)/255u);
                p[1]=uchar((unsigned(p[1])*alpha+127u)/255u);
                p[2]=uchar((unsigned(p[2])*alpha+127u)/255u);
            }
        }
    }
    return image;
}
}
