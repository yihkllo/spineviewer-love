#include "spinelove/texture_loader.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QImage>
#include <cstdio>

int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir temp;
    if(!temp.isValid())return 1;
    const QString file=temp.filePath(QString::fromUtf8("材质.png"));
    const QString alpha=temp.filePath(QString::fromUtf8("材质_alpha.png"));
    QImage original(2,1,QImage::Format_RGBA8888);
    original.setPixelColor(0,0,QColor(201,111,77,244));original.setPixelColor(1,0,QColor(255,80,20,255));
    if(!original.save(file))return 2;
    QImage channel(2,1,QImage::Format_RGBA8888);channel.setPixelColor(0,0,QColor(128,9,17,255));channel.setPixelColor(1,0,QColor(0,255,255,255));
    if(!channel.save(alpha))return 3;
    auto raw=slqt::loadTextureImage(file,false);
    if(raw.pixelColor(0,0)!=QColor(201,111,77,128)||raw.pixelColor(1,0)!=QColor(255,80,20,0))return 4;
    auto pma=slqt::loadTextureImage(file,true);
    if(pma.pixelColor(0,0)!=QColor(101,56,39,128)||pma.pixelColor(1,0)!=QColor(0,0,0,0))return 5;
    QImage recursive(2,1,QImage::Format_RGBA8888);recursive.fill(Qt::black);
    recursive.save(temp.filePath(QString::fromUtf8("材质_alpha_alpha.png")));
    if(slqt::loadTextureImage(alpha,false).pixelColor(0,0).alpha()!=255)return 6;
    QImage mismatch(1,1,QImage::Format_RGBA8888);mismatch.fill(Qt::black);mismatch.save(alpha);
    if(slqt::loadTextureImage(file,false).pixelColor(0,0).alpha()!=244)return 7;
    QImage combinations(256,256,QImage::Format_RGBA8888);
    for(int a=0;a<256;++a)for(int value=0;value<256;++value)
        combinations.setPixelColor(value,a,QColor(value,255-value,value*37%256,a));
    const auto full=temp.filePath("all-alpha-values.png");if(!combinations.save(full))return 8;
    const auto converted=slqt::loadTextureImage(full,true);
    if(converted.format()!=QImage::Format_RGBA8888)return 9;
    for(int a=0;a<256;++a)for(int value=0;value<256;++value){
        const QColor expected((value*a+127)/255,((255-value)*a+127)/255,((value*37%256)*a+127)/255,a);
        if(converted.pixelColor(value,a)!=expected)return 10;
    }
    QImage retained;{const auto decoded=slqt::loadTextureImage(full,false);retained=decoded;}
    QImage shared=retained;retained.setPixelColor(100,128,Qt::green);
    if(shared.pixelColor(100,128)!=combinations.pixelColor(100,128)||retained.pixelColor(100,128)!=QColor(Qt::green))return 11;
    std::puts("Texture decoding, split alpha, all 65536 channel/alpha combinations and shared-buffer lifetime passed.");return 0;
}
