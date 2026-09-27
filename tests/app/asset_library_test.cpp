#include "core/asset_library.h"
#include "core/archive_cache.h"

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
quint32 crc32(const QByteArray& bytes)
{
    quint32 crc = 0xFFFFFFFFu;
    for (const char value : bytes) {
        crc ^= static_cast<quint8>(value);
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}
void put16(QByteArray& out, quint16 value) { out.append(char(value & 0xFF)); out.append(char(value >> 8)); }
void put32(QByteArray& out, quint32 value) { put16(out, quint16(value & 0xFFFF)); put16(out, quint16(value >> 16)); }
bool writeZip(const QString& path, const QList<QPair<QByteArray, QByteArray>>& files)
{
    QByteArray body, directory;
    for (const auto& [name, data] : files) {
        const quint32 offset = quint32(body.size());
        const quint32 crc = crc32(data);
        put32(body, 0x04034b50); put16(body, 20); put16(body, 0x0800); put16(body, 0); put16(body, 0); put16(body, 0);
        put32(body, crc); put32(body, quint32(data.size())); put32(body, quint32(data.size()));
        put16(body, quint16(name.size())); put16(body, 0); body.append(name); body.append(data);
        put32(directory, 0x02014b50); put16(directory, 20); put16(directory, 20); put16(directory, 0x0800); put16(directory, 0);
        put16(directory, 0); put16(directory, 0); put32(directory, crc); put32(directory, quint32(data.size()));
        put32(directory, quint32(data.size())); put16(directory, quint16(name.size())); put16(directory, 0); put16(directory, 0);
        put16(directory, 0); put16(directory, 0); put32(directory, 0); put32(directory, offset); directory.append(name);
    }
    QByteArray zip = body + directory;
    put32(zip, 0x06054b50); put16(zip, 0); put16(zip, 0); put16(zip, quint16(files.size())); put16(zip, quint16(files.size()));
    put32(zip, quint32(directory.size())); put32(zip, quint32(body.size())); put16(zip, 0);
    return writeFile(path, zip);
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
    const QString shared = QDir(root).filePath("shared");
    check(writeFile(shared + "/hero-pro.json", skeletonJson()), "shared atlas skeleton exists");
    check(writeFile(shared + "/hero.atlas", "hero.png\nsize: 1,1\n"), "straight shared atlas exists");
    check(slqt::AssetLibrary::matchingAtlas(shared + "/hero-pro.json") == QDir::cleanPath(shared + "/hero.atlas"),
          "skeleton variant falls back to shared atlas by name prefix");
    check(writeFile(shared + "/hero-pma.atlas", "hero-pma.png\nsize: 1,1\n"), "premultiplied shared atlas exists");
    check(slqt::AssetLibrary::matchingAtlas(shared + "/hero-pro.json") == QDir::cleanPath(shared + "/hero-pma.atlas"),
          "premultiplied shared atlas is preferred");
    check(writeFile(shared + "/heroic.json", skeletonJson()) && writeFile(shared + "/settings.json", "{}"), "unrelated JSON fixtures exist");
    check(slqt::AssetLibrary::matchingAtlas(shared + "/heroic.json").isEmpty(), "atlas prefix must end at a name boundary");
    check(slqt::AssetLibrary::matchingAtlas(shared + "/settings.json").isEmpty(), "unrelated JSON does not borrow an atlas");
    check(writeFile(shared + "/unity.skel.bytes", binary) && writeFile(shared + "/unity.atlas.txt", "unity.png\nsize: 1,1\n"),
          "Unity export fixtures exist");
    check(slqt::AssetLibrary::isSpineFileName(shared + "/unity.skel.bytes"), "Unity binary skeleton name is recognized");
    check(slqt::AssetLibrary::skeletonStem(shared + "/unity.skel.bytes") == "unity", "Unity binary skeleton stem drops both suffixes");
    check(slqt::AssetLibrary::matchingAtlas(shared + "/unity.skel.bytes") == QDir::cleanPath(shared + "/unity.atlas.txt"),
          "Unity binary skeleton pairs with its text atlas");

    slqt::ArchiveCache::setRoot(QDir(root).filePath("cache"));
    const QString archive = QDir(root).filePath("pack.zip");
    check(writeZip(archive, {{"export/hero-pro.json", skeletonJson()}, {"export/hero.atlas", "hero.png\nsize: 1,1\n"},
                             {"export/hero.png", "png"}, {"__MACOSX/export/._hero.png", "junk"}, {"project.spine", "project"},
                             {"images/part.png", "part"}}), "archive fixture is written");
    check(slqt::ArchiveCache::isArchive(archive), "zip suffix is recognized as an archive");
    QString archiveError;
    const QString extracted = slqt::ArchiveCache::extract(archive, &archiveError);
    check(!extracted.isEmpty() && archiveError.isEmpty(), "archive extracts into the cache");
    check(QFileInfo(extracted + "/export/hero.png").isFile(), "texture keeps its archive-relative folder");
    check(!QFileInfo::exists(extracted + "/__MACOSX") && !QFileInfo::exists(extracted + "/project.spine"), "system and project files are skipped");
    const auto archived = slqt::AssetLibrary::scanSpine(extracted);
    check(archived.size() == 1 && archived.value(0).endsWith("/export/hero-pro.json"), "archived skeleton is found through shared atlas");
    check(slqt::AssetLibrary::inspect(archived.value(0)).isValid(), "archived skeleton inspects as a valid Spine model");
    const QString virtualPath = slqt::ArchiveCache::displayPath(archived.value(0));
    check(virtualPath == QDir::cleanPath(archive) + "/export/hero-pro.json", "cache path is shown as archive path plus inner path");
    check(slqt::ArchiveCache::sourceArchive(archived.value(0)) == QDir::cleanPath(archive), "cache path maps back to its archive");
    check(slqt::ArchiveCache::exists(virtualPath) && !slqt::ArchiveCache::exists(QDir(root).filePath("missing.zip/a.json")),
          "archive-relative paths exist only while the archive exists");
    check(QDir(slqt::ArchiveCache::root()).removeRecursively(), "archive cache can be cleared");
    check(slqt::ArchiveCache::resolve(virtualPath) == archived.value(0) && QFileInfo(archived.value(0)).isFile(),
          "archive-relative path re-extracts after the cache is cleared");
    check(slqt::ArchiveCache::extract(archive) == extracted, "unchanged archive reuses its cache folder");
    const QString unsafe = QDir(root).filePath("unsafe.zip");
    check(writeZip(unsafe, {{"../escape.png", "png"}, {"a.json", skeletonJson()}}), "unsafe archive fixture is written");
    check(slqt::ArchiveCache::extract(unsafe, &archiveError).isEmpty() && archiveError.contains("unsafe"), "path traversal is rejected");
    check(!QFileInfo::exists(QDir(root).filePath("cache/escape.png")), "rejected archive writes nothing outside the cache");
    const QString broken = QDir(root).filePath("broken.zip");
    check(writeFile(broken, "not a zip"), "broken archive fixture is written");
    archiveError.clear();
    check(slqt::ArchiveCache::extract(broken, &archiveError).isEmpty() && !archiveError.isEmpty(), "damaged archive reports an error");
    const QString empty = QDir(root).filePath("project-only.zip");
    check(writeZip(empty, {{"hero.spine", "project"}, {"images/a.png", "png"}}), "project-only archive fixture is written");
    archiveError.clear();
    slqt::ArchiveCache::extract(empty, &archiveError);
    check(slqt::AssetLibrary::scanSpine(slqt::ArchiveCache::extract(empty)).isEmpty(), "project-only archive yields no playable model");
    const QString library = QDir(root).filePath("library");
    makeSkeleton(library, "plain.json");
    check(writeZip(library + "/nested/bundle.zip", {{"export/a.json", skeletonJson()}, {"export/a.skel", binary},
                                                    {"export/a.atlas", "a.png\nsize: 1,1\n"}, {"export/b-pro.json", skeletonJson()},
                                                    {"export/b-pma.atlas", "b.png\nsize: 1,1\n"}, {"notes/readme.json", "{}"},
                                                    {"notes/other.atlas", "o.png\nsize: 1,1\n"}}), "nested archive fixture is written");
    const QString bundle = QDir::cleanPath(library + "/nested/bundle.zip");
    const auto cachedBefore = QDir(slqt::ArchiveCache::root()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    const auto listed = slqt::AssetLibrary::scanSpine(library);
    check(listed.contains(QDir::cleanPath(library + "/plain.json")), "folder scan keeps loose models");
    check(listed.contains(bundle + "/export/a.skel") && !listed.contains(bundle + "/export/a.json"),
          "folder scan lists archived models and prefers binary over duplicate JSON");
    check(listed.contains(bundle + "/export/b-pro.json"), "folder scan lists archived models with shared atlases");
    check(!listed.contains(bundle + "/notes/readme.json"), "folder scan skips unrelated JSON inside archives");
    check(listed.size() == 3, "folder scan lists exactly the playable models");
    check(QDir(slqt::ArchiveCache::root()).entryList(QDir::Dirs | QDir::NoDotAndDotDot) == cachedBefore, "listing an archive does not extract it");
    check(slqt::ArchiveCache::archiveOf(bundle + "/export/a.skel") == bundle, "archive-relative path maps to its archive");
    const QString opened = slqt::ArchiveCache::resolve(bundle + "/export/b-pro.json");
    check(QFileInfo(opened).isFile() && slqt::AssetLibrary::inspect(opened).isValid(), "listed archive model extracts and inspects on demand");
    check(slqt::ArchiveCache::displayPath(opened) == bundle + "/export/b-pro.json", "extracted model maps back to its listed path");
    slqt::ArchiveCache::setRoot({});

    qInfo() << (failures == 0 ? "Asset-library compatibility checks passed." : "Asset-library compatibility checks failed.")
            << failures;
    return failures == 0 ? 0 : 1;
}
