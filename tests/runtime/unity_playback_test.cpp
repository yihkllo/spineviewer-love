#include "live2d/unity_playback.h"
#include <fstream>
#include <filesystem>
#include <iostream>

using Json = nlohmann::json;
void require(bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);}
Json read(const std::string& path){std::ifstream f(std::filesystem::u8path(path));require(bool(f),path);return Json::parse(f);}
void close(float actual,double expected,const std::string& context){
    require(std::isfinite(actual)&&std::abs(actual-expected)<=2e-5*std::max(1.,std::abs(expected)),context+": "+std::to_string(actual)+" != "+std::to_string(expected));
}
std::vector<std::string> ids(const Json& rows){std::vector<std::string> out;for(const auto& row:rows)out.push_back(row.at("id"));return out;}
int main(int argc,char** argv){try{
    require(argc==2,"Pass the original Unity reference fixture.");
    const auto fixture=read(argv[1]);size_t samples=0,values=0,motions=0;
    for(const auto& reference:fixture.at("models")){
        const std::string name=reference.at("id");auto data=read(reference.at("playback"));
        require(data.at("parameters")==reference.at("parameters")&&data.at("parts")==reference.at("parts"),name+" source defaults");
        live2d::UnityPlayback player;player.load(data,ids(reference.at("parameters")),ids(reference.at("parts")));
        std::vector<float> params(data.at("parameters").size()),parts(data.at("parts").size());
        for(const auto& motion:reference.at("motions")){
            const std::string clip=motion.at("name");++motions;
            for(const auto& sample:motion.at("samples")){
                const double time=sample.at("time");
                require(player.play(clip,motion.at("loop"),0,time),name+" missing "+clip);
                player.update(.091,params.data(),parts.data());
                for(size_t i=0;i<params.size();++i){close(params[i],sample.at("parameters")[i],name+"/"+clip+" param "+std::to_string(i)+" at "+std::to_string(time));++values;}
                for(size_t i=0;i<parts.size();++i){close(parts[i],sample.at("parts")[i],name+"/"+clip+" part "+std::to_string(i)+" at "+std::to_string(time));++values;}
                ++samples;
            }
        }
        const auto& list=reference.at("motions");
        if(list.size()>1){
            live2d::UnityPlayback old,newer;old.load(data,ids(data.at("parameters")),ids(data.at("parts")));newer.load(data,ids(data.at("parameters")),ids(data.at("parts")));
            std::vector<float> p0(params.size()),p1(params.size()),s0(parts.size()),s1(parts.size());
            player.play(list[0].at("name"),true,0,.125);player.update(0,params.data(),parts.data());
            player.play(list[1].at("name"),true,.4,0);player.seek(.1);player.update(0,params.data(),parts.data());
            old.play(list[0].at("name"),true,0,.225);old.update(0,p0.data(),s0.data());newer.play(list[1].at("name"),true,0,.1);newer.update(0,p1.data(),s1.data());
            for(size_t i=0;i<params.size();++i)close(params[i],p0[i]+(p1[i]-p0[i])*.25,name+" parameter transition");
            for(size_t i=0;i<parts.size();++i)close(parts[i],s0[i]+(s1[i]-s0[i])*.25,name+" part transition");
            const auto p=params,s=parts;for(int i=0;i<10;++i)player.update(.1,params.data(),parts.data());require(params==p&&parts==s,name+" clock drift");
        }
    }
    {
        const auto data=Json::parse(R"({"parameters":[],"parts":[],"opacity":0.2,"motions":{"fade":{"duration":2,"curves":[],"opacity":[[0,[0,0,0.5,0]]]},"visible":{"duration":2,"curves":[],"opacity":[[0,[0,0,0,1]]]}}})");
        live2d::UnityPlayback player;player.load(data,{},{});player.update(0,nullptr,nullptr);close(player.opacity(),.2,"model opacity default");
        require(player.play("fade",true,0,1),"opacity motion missing");player.update(0,nullptr,nullptr);close(player.opacity(),.5,"model opacity curve");
        player.seek(3);player.update(0,nullptr,nullptr);close(player.opacity(),.5,"model opacity loop");
        player.play("visible",true,.4,0);player.seek(.2);player.update(0,nullptr,nullptr);close(player.opacity(),.8,"model opacity transition");
        player.load(data,{},{});player.update(0,nullptr,nullptr);close(player.opacity(),.2,"model opacity reload");
    }
    std::cout<<"PASS "<<fixture.at("models").size()<<" models, "<<motions<<" motions, "<<samples<<" source samples, "<<values<<" parameter/part comparisons; transitions and external clocks verified.\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
