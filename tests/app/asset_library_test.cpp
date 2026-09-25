#include "core/asset_library.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) {
        qCritical().noquote() << "FAIL:" << message;
        ++failures;
    }
}
bool writeFile(const QString& path, const QByteArray& bytes)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    return file.write(bytes) == bytes.size();
}
QByteArray skeletonJson(const QByteArray& version = "4.2.67")
{
    return "{\"skeleton\":{\"spine\":\"" + version + "\"},\"bones\":[{\"name\":\"root\"}]}";
}
QString makeSkeleton(const QString& folder, const QString& name, const QByteArray& bytes = skeletonJson())
{
    const QString path = QDir(folder).filePath(name);
    check(writeFile(path, bytes), "test skeleton fixture is writable");
    const QString atlas = QFileInfo(path).absolutePath() + '/' + QFileInfo(path).completeBaseName() + ".atlas";
    check(writeFile(atlas, "page.png\nsize: 1,1\n"), "test atlas fixture is writable");
    return path;
}
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir temporary;
    check(temporary.isValid(), "isolated temporary fixture directory exists");
    if (!temporary.isValid())
        return 1;
    const QString root = temporary.path();
    const QString model = makeSkeleton(root, QStringLiteral("角色2.v1.json"));
    const auto entry = slqt::AssetLibrary::inspect(model);
    check(entry.isValid() && entry.kind == slqt::AssetKind::SpineJson, "UTF-8 path loads through original Spine probe");
    check(entry.spineVersion == "4.2.67", "full exported version is retained without coercion");
    check(entry.displayName == QStringLiteral("角色2.v1"), "only final extension is removed from display name");
    check(entry.textureDirectory.endsWith('/'), "atlas-relative texture directory retains delimiter");
    check(writeFile(entry.atlasPath + ".txt", "alternate atlas"), "fallback atlas fixture exists");
    check(slqt::AssetLibrary::matchingAtlas(model) == entry.atlasPath, ".atlas has priority over .atlas.txt");
    check(QFile::remove(entry.atlasPath), "primary fixture atlas removed");
    check(slqt::AssetLibrary::matchingAtlas(model) == entry.atlasPath + ".txt", ".atlas.txt is accepted when .atlas is absent");
    check(writeFile(entry.atlasPath.left(entry.atlasPath.size() - 6) + ".ATLAS", "case atlas"), "uppercase atlas fixture exists");
    check(QFileInfo(slqt::AssetLibrary::matchingAtlas(model)).suffix().compare("atlas", Qt::CaseInsensitive) == 0,
          "case-insensitive .atlas retains priority on case-sensitive filesystems");

    const QString invalid = makeSkeleton(root, "config.json", "{\"version\":\"4.2.67\",\"spine\":\"4.2.67\"}");
    check(!slqt::AssetLibrary::inspect(invalid).isValid(), "ordinary JSON is not accepted as a Spine skeleton");
    const QString live2d = QDir(root).filePath("avatar.model3.json");
    check(writeFile(live2d, "{\"Version\":3,\"FileReferences\":{}}"), "Live2D fixture exists");
    check(slqt::AssetLibrary::inspect(live2d).kind == slqt::AssetKind::Live2D, "Live2D suffix is recognized before generic JSON");
    check(!slqt::AssetLibrary::readSpineBundle({live2d}).isValid(), "Live2D cannot enter a Spine bundle");

    QByteArray binary(8, '\0');
    binary.append(char(7));
    binary.append("4.2.67");
    const QString binaryPath = makeSkeleton(root, "binary.skel", binary);
    check(slqt::AssetLibrary::inspect(binaryPath).kind == slqt::AssetKind::SpineBinary,
          "fixed-hash Spine binary version probing is unchanged");
    const QString oldBinary = makeSkeleton(root, "legacy.skel", QByteArray("\x02h\x06" "3.8.1", 8));
    check(slqt::AssetLibrary::inspect(oldBinary).spineVersion == "3.8.1", "string-hash legacy binary header remains recognized");
    const auto mixed = slqt::AssetLibrary::readSpineBundle({model, binaryPath});
    check(!mixed.isValid() && mixed.items.isEmpty() && mixed.skeletonBytes.isEmpty(),
          "mixed JSON/binary opening fails atomically without a partial bundle");
    const QString otherPatch = makeSkeleton(root, "other.json", skeletonJson("4.2.68"));
    check(!slqt::AssetLibrary::readSpineBundle({model, otherPatch}).isValid(),
          "batch open preserves exact version restriction even within one runtime lane");
    const QString second = makeSkeleton(root, "model10.json");
    const auto good = slqt::AssetLibrary::readSpineBundle({second, model});
    check(good.isValid() && good.items.size() == 2 && good.items.front().path == second,
          "bundle preserves caller order for layer identity");
    check(good.skeletonBytes.front() == skeletonJson() && !good.atlasBytes.front().isEmpty(),
          "skeleton and atlas bytes are ready for original memory-loading API");
    const QList<QByteArray> boneCases{
        "\"bones\":[]}", "\"bones\":null}", "\"bones\":{}}",
        "\"metadata\":{\"bones\":[{}]}}", "\"bones\":[{}]}",
        "\"bones\":[{}],\"bones\":[]}", "\"bones\":[],\"bones\":[{}]}",
        "\"bones\":[{}],\"bones\":false}", "\"bones\":[{\"name\":\"root\"}],\"n\":1e400}",
        "\"bones\":[{\"name\":\"root\"}],\"extra\":{\"nested\":[1,2,true,null]}}",
        "\"bones\":[{}],\"extra\":[1,]}", "\"bones\":[{}]} trailing",
        "\"bones\":[{}],\"extra\":\"broken", "\"bones\":[{}],\"extra\":01}",
        "\"bones\":[{}],\"extra\":"+QByteArray(1030,'[')+"0"+QByteArray(1030,']')+"}"
    };
    for(const auto& tail:boneCases){
        const QByteArray bytes="{\"skeleton\":{\"spine\":\"3.6.53\"},"+tail;
        const auto document=QJsonDocument::fromJson(bytes);
        const bool expected=document.isObject()&&!document.object().value("bones").toArray().isEmpty();
        const auto path=makeSkeleton(root,"preflight.json",bytes);
        check(slqt::AssetLibrary::readSpineBundle({path}).isValid()==expected,
            "streaming preflight retains full syntax, nesting and last-root-bones validation");
    }

    QStringList sorted{root + "/a/model10.json", root + "/b/model2.json", root + "/a/model1.json"};
    slqt::AssetLibrary::naturalSort(sorted);
    check(QFileInfo(sorted[0]).fileName() == "model1.json" && QFileInfo(sorted[1]).fileName() == "model2.json",
          "browser naturally orders model1, model2, model10 by filename rather than parent path");
    const QString noAtlas = QDir(root).filePath("unpaired.json");
    check(writeFile(noAtlas, skeletonJson()), "unpaired fixture exists");
    const auto scan = slqt::AssetLibrary::scanSpine(root);
    check(!scan.contains(noAtlas), "browser excludes skeleton suffixes without a sibling atlas");
    check(scan.contains(invalid), "browser remains cheap suffix/atlas scan; validation is deferred until open");
    check(slqt::AssetLibrary::scanLive2D(root).contains(live2d), "Live2D recursive browser finds model3 manifest");
    QString nested = QDir(root).filePath("depths");
    QString atDepth7, atDepth8, atDepth12, atDepth13;
    for (int depth = 0; depth <= 13; ++depth) {
        if (depth == 7)
            atDepth7 = makeSkeleton(nested, "boundary7.json");
        if (depth == 8)
            atDepth8 = makeSkeleton(nested, "outside8.json");
        if (depth == 12) {
            atDepth12 = QDir(nested).filePath("boundary12.model3.json");
            check(writeFile(atDepth12, "{}"), "depth 12 fixture exists");
        }
        if (depth == 13) {
            atDepth13 = QDir(nested).filePath("outside13.model3.json");
            check(writeFile(atDepth13, "{}"), "depth 13 fixture exists");
        }
        nested += "/nested";
    }
    const auto depthSpine = slqt::AssetLibrary::scanSpine(QDir(root).filePath("depths"));
    const auto depthLive2d = slqt::AssetLibrary::scanLive2D(QDir(root).filePath("depths"));
    check(depthSpine.contains(atDepth7) && !depthSpine.contains(atDepth8), "Spine scan keeps inclusive depth 7 limit");
    check(depthLive2d.contains(atDepth12) && !depthLive2d.contains(atDepth13), "Live2D scan keeps inclusive depth 12 limit");
    QString urlError;
    check(slqt::AssetLibrary::localPath(QUrl::fromLocalFile(model), &urlError) == model && urlError.isEmpty(),
          "document-picker local URL preserves Unicode path");
    check(slqt::AssetLibrary::localPath(QUrl("content://documents/tree/123"), &urlError).isEmpty() && !urlError.isEmpty(),
          "Android content URI is not silently misinterpreted as a local path");
    qInfo() << (failures == 0 ? "Asset-library compatibility checks passed." : "Asset-library compatibility checks failed.")
            << failures;
    return failures == 0 ? 0 : 1;
}
