#pragma once
#include "runtime_api.h"
#include <string>
namespace sl_test {
inline sl_runtime_v2::LoadRequest Fixture(const std::string& version, bool twoLooks = false) {
    sl_runtime_v2::LoadRequest request;
    request.atlasData.emplace_back(
        "probe.png\nsize: 32, 32\nformat: RGBA8888\nfilter: Linear, Linear\nrepeat: none\n"
        "probe\n  rotate: false\n  xy: 0, 0\n  size: 32, 32\n  orig: 32, 32\n  offset: 0, 0\n  index: -1\n");
    request.textureDirectories.emplace_back("fixtures/");
    const std::string attachment = R"({"body":{"probe":{"type":"region","path":"probe","width":32,"height":32}}})";
    const bool arraySkins = version >= "3.8";
    const std::string alternate = R"({"body":{"probe":{"type":"region","path":"probe","x":7,"width":32,"height":32}}})";
    const std::string skins = arraySkins
        ? "[{\"name\":\"default\",\"attachments\":" + attachment + "}" + (twoLooks ? ",{\"name\":\"alternate\",\"attachments\":" + alternate + "}" : "") + "]"
        : "{\"default\":" + attachment + (twoLooks ? ",\"alternate\":" + alternate : "") + "}";
    request.skeletonData.emplace_back(
        "{\"skeleton\":{\"hash\":\"synthetic-runtime-contract\",\"spine\":\"" + version + ".0\",\"width\":32,\"height\":32},"
        R"("bones":[{"name":"root"}],"slots":[{"name":"body","bone":"root","attachment":"probe"}],"skins":)" + skins +
        R"(,"animations":{"move":{"bones":{"root":{"translate":[{"time":0,"x":0,"y":0},{"time":1,"x":16,"y":0}]}}},"empty":{}}})");
    return request;
}

}

