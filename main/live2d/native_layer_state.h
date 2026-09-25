#pragma once
#include <string>

namespace live2d {
struct NativeLayerState {
    std::string name,previousName;
    double time=0,previousTime=0;
    float weight=1,mixWeight=1;
    bool loop=true,previousLoop=true,writeDefaults=true,previousWriteDefaults=true;
};
}
