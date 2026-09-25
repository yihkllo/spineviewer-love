#include "core/asset_library.h"
#include "sl_path_util.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <iostream>

int main(int argc,char** argv)
{
    QCoreApplication app(argc,argv);
    const auto args=app.arguments();
    if(args.size()!=3){std::cerr<<"Usage: asset_library_audit input-directory output-report.json\n";return 2;}
    const QString root=args[1];
    if(!QDir(root).exists()){std::cerr<<"Input directory does not exist\n";return 2;}
    const auto spine=slqt::AssetLibrary::scanSpine(root);
    const auto live2d=slqt::AssetLibrary::scanLive2D(root);
    std::vector<std::wstring> oldSpine,oldLive2d;
    path_util::ScanSkeletonFilesRecursive(root.toStdWString(),oldSpine);
    path_util::ScanLive2DModelsRecursive(root.toStdWString(),oldLive2d);
    const auto normalize=[](const std::vector<std::wstring>& paths){QStringList result;for(const auto& p:paths)result.append(QDir::fromNativeSeparators(QString::fromStdWString(p)));return result;};
    const bool spineSame=normalize(oldSpine)==spine,liveSame=normalize(oldLive2d)==live2d;
    QJsonArray entries;int invalid=0;
    for(const auto& path:spine+live2d){
        const auto item=slqt::AssetLibrary::inspect(path);
        if(!item.isValid())++invalid;
        entries.append(QJsonObject{{"path",item.path},{"displayName",item.displayName},{"atlasPath",item.atlasPath},
                                   {"version",item.spineVersion},{"textureDirectory",item.textureDirectory},
                                   {"kind",item.kind==slqt::AssetKind::Live2D?"Live2D":item.kind==slqt::AssetKind::SpineBinary?"SpineBinary":"SpineJson"},
                                   {"valid",item.isValid()},{"error",item.error}});
    }
    QJsonObject report{{"root",root},{"spineCount",spine.size()},{"live2dCount",live2d.size()},
                       {"legacySpineOrderIdentical",spineSame},{"legacyLive2dOrderIdentical",liveSame},
                       {"invalidEntries",invalid},{"entries",entries}};
    QFile output(args[2]);if(!output.open(QIODevice::WriteOnly)){std::cerr<<"Report output is not writable\n";return 2;}
    output.write(QJsonDocument(report).toJson());
    std::cout<<"Spine: "<<spine.size()<<", Live2D: "<<live2d.size()<<", invalid: "<<invalid
             <<", old Spine order equal: "<<spineSame<<", old Live2D order equal: "<<liveSame<<'\n';
    return spineSame&&liveSame&&invalid==0?0:1;
}
