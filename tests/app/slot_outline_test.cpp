#include "render/slot_outline.h"

#include <iostream>
#include <cmath>

int main()
{
    int failures=0;
    const auto check=[&](bool ok,const char* message){if(!ok){std::cerr<<"FAIL: "<<message<<'\n';++failures;}};
    ReadSlotMeshData mesh;mesh.textureHandle=2;mesh.worldVertices={0,0,4,0,4,4,0,4};mesh.uvs={0,0,1,0,1,1,0,1};mesh.triangles={0,1,2,2,3,0};mesh.isRegion=true;
    QImage texture(4,4,QImage::Format_RGBA8888);texture.fill(Qt::transparent);
    texture.setPixelColor(1,1,Qt::white);texture.setPixelColor(2,1,Qt::white);texture.setPixelColor(1,2,Qt::white);texture.setPixelColor(2,2,Qt::white);
    const SlColor green(0,1,0,1);
    const auto contour=slqt::buildSlotOutline(mesh,texture,SlIdentityMatrix4(),green);
    check(contour.method==slqt::SlotOutlineMethod::AlphaContour,"opaque texture islands use alpha contour before mesh boundary");
    check(contour.contourPointCount==4&&contour.draws.commands.size()==1&&contour.draws.commands[0].vertices.size()==16,
          "only four opaque island edge pixels emit contour dots, rather than outer rectangle");
    const auto& first=contour.draws.commands[0].vertices[0];
    check(std::abs(first.pos.x+.6f)<.0001f&&std::abs(first.pos.y+.6f)<.0001f,
          "padding is removed and 3.2-pixel local contour dots have the legacy origin");
    const auto shifted=slqt::buildSlotOutline(mesh,texture,SlMatrixTranslate(10,20,0),green);
    check(std::abs(shifted.draws.commands[0].vertices[0].pos.x-(first.pos.x+10))<.0001f,
          "contour dot corners are transformed after local thickness expansion");
    texture.fill(QColor(255,255,255,16));
    const auto threshold=slqt::buildSlotOutline(mesh,texture,SlIdentityMatrix4(),green);
    check(threshold.method==slqt::SlotOutlineMethod::TriangleBoundary,
          "alpha exactly 16 is rejected, including pixels on the shared triangle edge");
    check(threshold.draws.commands.size()==1&&threshold.draws.commands[0].vertices.size()==16,
          "triangle fallback draws four boundary edges without interior diagonal");
    texture.fill(QColor(255,255,255,17));
    const auto accepted=slqt::buildSlotOutline(mesh,texture,SlIdentityMatrix4(),green);
    check(accepted.method==slqt::SlotOutlineMethod::AlphaContour&&accepted.contourPointCount==12,
          "alpha 17 yields the twelve perimeter pixels of a 4x4 island");
    const auto missing=slqt::buildSlotOutline(mesh,QImage{},SlIdentityMatrix4(),green);
    check(missing.method==slqt::SlotOutlineMethod::TriangleBoundary,"unavailable texture preserves geometric fallback");
    ReadSlotMeshData bilinearMesh=mesh;bilinearMesh.worldVertices={0,0,2,0,2,2,0,2};bilinearMesh.uvs={.25f,0,.75f,0,.75f,1,.25f,1};
    QImage bilinearTexture(2,1,QImage::Format_RGBA8888);bilinearTexture.fill(Qt::transparent);bilinearTexture.setPixelColor(1,0,Qt::white);
    const auto bilinear=slqt::buildSlotOutline(bilinearMesh,bilinearTexture,SlIdentityMatrix4(),green);
    check(bilinear.method==slqt::SlotOutlineMethod::AlphaContour&&bilinear.contourPointCount==4,
          "bilinear UV sampling retains partially covered edge pixels that nearest sampling loses");
    mesh.triangles.clear();mesh.hullLength=8;
    const auto hull=slqt::buildSlotOutline(mesh,QImage{},SlIdentityMatrix4(),green);
    check(hull.method==slqt::SlotOutlineMethod::OrderedHull&&hull.draws.commands[0].vertices.size()==16,
          "ordered fallback accepts legacy hullLength measured as coordinate pairs");
    mesh.worldVertices={0,0,5000,0,5000,10,0,10};mesh.triangles={0,1,2,2,3,0};
    const auto oversized=slqt::buildSlotOutline(mesh,texture,SlIdentityMatrix4(),green);
    check(oversized.method==slqt::SlotOutlineMethod::TriangleBoundary,"oversized raster target falls back without allocation");
    mesh.worldVertices={0,0,4,0,4,4,0,4};mesh.triangles={0,1,99};mesh.isRegion=false;mesh.hullLength=0;
    const auto malformed=slqt::buildSlotOutline(mesh,QImage{},SlIdentityMatrix4(),green);
    check(malformed.draws.commands.empty(),"bad triangle indices do not access beyond vertex memory");
    std::cout<<"Slot-outline compatibility checks: "<<failures<<" failures\n";
    return failures==0?0:1;
}
