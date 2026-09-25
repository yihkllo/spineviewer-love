#pragma once
#include "native_layer_state.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace live2d {
class UnityPlayback {
    struct Key { double time; std::array<double,4> c; };
    struct Curve { bool part; size_t index; std::vector<Key> keys; };
    struct Motion { double duration=0; int layer=0; std::vector<Curve> curves; Curve opacity{}; };
    std::unordered_map<std::string,Motion> m_motions;
    std::vector<float> m_parameterDefaults,m_partDefaults,m_parameters,m_parts,m_oldParameters,m_oldParts;
    const Motion *m_motion=nullptr,*m_previous=nullptr;
    std::vector<std::pair<const Motion*,float>> m_layers;
    double m_layerTime=0;
    double m_time=0,m_previousTime=0,m_mix=0,m_start=0;
    float m_opacityDefault=1,m_opacity=1;
    bool m_loop=true,m_previousLoop=true,m_external=false,m_justStarted=false;
    struct LayerPose {std::vector<float> parameters,parts;float opacity=1;};
    struct BindingMask {std::vector<bool> parameters,parts;bool opacity=false;};
    std::vector<NativeLayerState> m_explicitLayers;
    std::vector<LayerPose> m_retainedLayers;
    std::unordered_map<int,BindingMask> m_layerBindings;
    std::unordered_map<std::string,size_t> m_parameterIndices,m_partIndices;
    std::vector<std::pair<size_t,float>> m_frameOverrides,m_partOverrides;
    bool m_explicit=false;

    void addBindings(BindingMask& mask,const Motion* motion)const{
        if(mask.parameters.size()!=m_parameterDefaults.size())mask.parameters.assign(m_parameterDefaults.size(),false);
        if(mask.parts.size()!=m_partDefaults.size())mask.parts.assign(m_partDefaults.size(),false);
        if(!motion)return;
        for(const auto& curve:motion->curves)(curve.part?mask.parts:mask.parameters)[curve.index]=true;
        mask.opacity|=!motion->opacity.keys.empty();
    }
    const Motion* findMotion(const std::string& name)const{
        const auto it=m_motions.find(name);return it==m_motions.end()?nullptr:&it->second;
    }
    float clipOpacity(const Motion* clip,double time,bool loop,float fallback)const{
        if(!clip||clip->opacity.keys.empty())return fallback;
        if(clip->duration>0){time=loop?std::fmod(time,clip->duration):std::clamp(time,0.,clip->duration);if(loop&&time<0)time+=clip->duration;}
        return float(sample(clip->opacity,time));
    }
    static double explicitTime(const Motion* motion,double time,bool loop){
        if(!motion||motion->duration<=0)return 0;
        if(!loop)return std::clamp(time,0.,motion->duration);
        const double clock=std::fmod(time,motion->duration);return clock<0?clock+motion->duration:clock;
    }
    void updateExplicit(float* parameters,float* parts){
        m_parameters=m_parameterDefaults;m_parts=m_partDefaults;m_opacity=m_opacityDefault;
        m_retainedLayers.resize(m_explicitLayers.size());
        for(size_t layer=0;layer<m_explicitLayers.size();++layer){
            const auto& state=m_explicitLayers[layer];auto& retained=m_retainedLayers[layer];
            if(retained.parameters.size()!=m_parameterDefaults.size()||retained.parts.size()!=m_partDefaults.size()){
                retained.parameters=m_parameterDefaults;retained.parts=m_partDefaults;retained.opacity=m_opacityDefault;
            }
            const auto* motion=findMotion(state.name);const auto* previous=findMotion(state.previousName);
            auto mask=m_layerBindings[int(layer)];addBindings(mask,motion);addBindings(mask,previous);
            LayerPose current=retained,old=retained;
            if(state.writeDefaults){current.parameters=m_parameterDefaults;current.parts=m_partDefaults;current.opacity=m_opacityDefault;}
            if(!motion){current.parameters=m_parameters;current.parts=m_parts;current.opacity=m_opacity;}
            if(state.previousWriteDefaults){old.parameters=m_parameterDefaults;old.parts=m_partDefaults;old.opacity=m_opacityDefault;}
            if(motion)evaluate(*motion,explicitTime(motion,state.time,state.loop),false,current.parameters,current.parts);
            if(previous)evaluate(*previous,explicitTime(previous,state.previousTime,state.previousLoop),false,old.parameters,old.parts);
            current.opacity=clipOpacity(motion,state.time,state.loop,current.opacity);
            old.opacity=clipOpacity(previous,state.previousTime,state.previousLoop,old.opacity);
            const float mix=state.previousName.empty()?1:std::clamp(state.mixWeight,0.f,1.f),weight=std::clamp(state.weight,0.f,1.f);
            for(size_t i=0;i<m_parameters.size();++i){
                current.parameters[i]=old.parameters[i]+(current.parameters[i]-old.parameters[i])*mix;
                if(mask.parameters[i])m_parameters[i]+=(current.parameters[i]-m_parameters[i])*weight;
            }
            for(size_t i=0;i<m_parts.size();++i){
                current.parts[i]=old.parts[i]+(current.parts[i]-old.parts[i])*mix;
                if(mask.parts[i])m_parts[i]+=(current.parts[i]-m_parts[i])*weight;
            }
            current.opacity=old.opacity+(current.opacity-old.opacity)*mix;
            if(mask.opacity)m_opacity+=(current.opacity-m_opacity)*weight;
            retained=std::move(current);
        }
        for(const auto& value:m_frameOverrides)m_parameters[value.first]=value.second;
        for(const auto& value:m_partOverrides)m_parts[value.first]=value.second;
        m_opacity=std::clamp(m_opacity,0.f,1.f);
        std::copy(m_parameters.begin(),m_parameters.end(),parameters);std::copy(m_parts.begin(),m_parts.end(),parts);
    }

    static double sample(const Curve& c,double time) {
        auto it=std::upper_bound(c.keys.begin(),c.keys.end(),time,[](double t,const Key& k){return t<k.time;});
        if(it!=c.keys.begin())--it;
        const double dt=std::max(0.,time-it->time);
        return ((it->c[0]*dt+it->c[1])*dt+it->c[2])*dt+it->c[3];
    }
    static void evaluate(const Motion& motion,double time,bool loop,std::vector<float>& parameters,std::vector<float>& parts,float weight=1) {
        if(motion.duration>0)time=loop&&time>motion.duration?std::fmod(time,motion.duration):std::min(time,motion.duration);
        for(const auto& curve:motion.curves){auto& value=(curve.part?parts:parameters)[curve.index];const auto v=float(sample(curve,time));value=weight==1?v:value+(v-value)*weight;}
    }
    void evaluatePose(const Motion* motion,double time,bool loop,std::vector<float>& parameters,std::vector<float>& parts)const{
        if(m_layers.empty()){if(motion)evaluate(*motion,time,loop,parameters,parts);return;}
        for(size_t i=0;i<m_layers.size();++i){const auto* clip=motion&&motion->layer==int(i)?motion:m_layers[i].first;if(clip)evaluate(*clip,clip==motion?time:m_layerTime,clip==motion?loop:true,parameters,parts,m_layers[i].second);}
    }
    float evaluateOpacity(const Motion* motion,double time,bool loop)const{
        float value=m_opacityDefault;
        auto apply=[&](const Motion* clip,double clock,bool looping,float weight){
            if(!clip||clip->opacity.keys.empty())return;
            if(clip->duration>0)clock=looping&&clock>clip->duration?std::fmod(clock,clip->duration):std::min(clock,clip->duration);
            value+=(float(sample(clip->opacity,clock))-value)*weight;
        };
        if(m_layers.empty())apply(motion,time,loop,1);
        else for(size_t i=0;i<m_layers.size();++i){const auto* clip=motion&&motion->layer==int(i)?motion:m_layers[i].first;apply(clip,clip==motion?time:m_layerTime,clip==motion?loop:true,m_layers[i].second);}
        return std::clamp(value,0.f,1.f);
    }
public:
    void load(const nlohmann::json& data,const std::vector<std::string>& parameterIds,const std::vector<std::string>& partIds) {
        m_explicit=false;m_explicitLayers.clear();m_retainedLayers.clear();m_frameOverrides.clear();m_partOverrides.clear();m_parameterIndices.clear();m_partIndices.clear();m_layerBindings.clear();
        for(size_t i=0;i<parameterIds.size();++i)m_parameterIndices[parameterIds[i]]=i;
        for(size_t i=0;i<partIds.size();++i)m_partIndices[partIds[i]]=i;
        m_motions.clear();m_layers.clear();m_motion=m_previous=nullptr;m_time=m_layerTime=m_previousTime=m_mix=m_start=0;m_external=m_justStarted=false;
        auto defaults=[&](const char* name,const std::vector<std::string>& ids){
            std::vector<float> out;const auto& rows=data.at(name);
            if(rows.size()!=ids.size())throw std::runtime_error("Unity/Cubism model binding count differs.");
            for(size_t i=0;i<ids.size();++i){if(rows[i].at("id")!=ids[i])throw std::runtime_error("Unity/Cubism model binding IDs differ.");out.push_back(rows[i].at("value").get<float>());}
            return out;
        };
        m_parameterDefaults=defaults("parameters",parameterIds);m_partDefaults=defaults("parts",partIds);
        m_opacityDefault=m_opacity=data.value("opacity",1.f);
        m_parameters=m_parameterDefaults;m_parts=m_partDefaults;m_oldParameters=m_parameters;m_oldParts=m_parts;
        for(auto it=data.at("motions").begin();it!=data.at("motions").end();++it){
            Motion motion;motion.duration=it.value().at("duration").get<double>();motion.layer=it.value().value("layer",0);
            for(const auto& key:it.value().value("opacity",nlohmann::json::array()))motion.opacity.keys.push_back({key.at(0).get<double>(),key.at(1).get<std::array<double,4>>()});
            for(const auto& row:it.value().at("curves")){
                Curve curve;curve.part=row.at("target")=="PartOpacity";curve.index=row.at("index").get<size_t>();
                const auto& ids=curve.part?partIds:parameterIds;
                if(curve.index>=ids.size()||row.at("id")!=ids[curve.index])throw std::runtime_error("Invalid Unity motion binding.");
                for(const auto& key:row.at("keys"))curve.keys.push_back({key.at(0).get<double>(),key.at(1).get<std::array<double,4>>()});
                if(curve.keys.empty()||!std::is_sorted(curve.keys.begin(),curve.keys.end(),[](const Key& a,const Key& b){return a.time<b.time;}))throw std::runtime_error("Invalid Unity motion keys.");
                motion.curves.push_back(std::move(curve));
            }
            m_motions.emplace(it.key(),std::move(motion));
        }
        for(const auto& entry:m_motions)addBindings(m_layerBindings[entry.second.layer],&entry.second);
        int bindingLayer=0;
        for(const auto& layer:data.value("layers",nlohmann::json::array())){
            if(layer.contains("motions")){
                BindingMask mask;addBindings(mask,nullptr);
                for(const auto& name:layer.at("motions"))addBindings(mask,findMotion(name.get<std::string>()));
                m_layerBindings[bindingLayer]=std::move(mask);
            }
            ++bindingLayer;
        }
        for(const auto& layer:data.value("layers",nlohmann::json::array())){
            auto it=m_motions.find(layer.at("motion").get<std::string>());m_layers.emplace_back(it==m_motions.end()?nullptr:&it->second,layer.value("weight",1.f));
        }
    }
    bool enabled()const{return !m_motions.empty();}
    float opacity()const{return m_opacity;}
    bool active()const{return m_explicit||m_motion!=nullptr||!m_layers.empty();}
    bool finished()const{return m_explicit?false:(!m_motion?m_layers.empty():(!m_loop&&m_time>=m_motion->duration));}
    void setLayerStates(const std::vector<NativeLayerState>& layers,const std::unordered_map<std::string,float>& overrides,const std::unordered_map<std::string,float>& parts={}){
        m_explicit=true;m_explicitLayers=layers;m_frameOverrides.clear();m_partOverrides.clear();
        for(auto& state:m_explicitLayers){
            if(!std::isfinite(state.time))state.time=0;if(!std::isfinite(state.previousTime))state.previousTime=0;
            if(!std::isfinite(state.weight))state.weight=0;if(!std::isfinite(state.mixWeight))state.mixWeight=1;
        }
        for(const auto& entry:overrides){const auto found=m_parameterIndices.find(entry.first);if(found!=m_parameterIndices.end()&&std::isfinite(entry.second))m_frameOverrides.emplace_back(found->second,entry.second);}
        for(const auto& entry:parts){const auto found=m_partIndices.find(entry.first);if(found!=m_partIndices.end()&&std::isfinite(entry.second))m_partOverrides.emplace_back(found->second,entry.second);}
    }
    void setLayers(const std::vector<std::string>& names){
        m_explicit=false;m_explicitLayers.clear();m_retainedLayers.clear();m_frameOverrides.clear();
        if(m_layers.empty())m_layers.resize(names.size(),{nullptr,1.f});
        for(size_t i=0;i<m_layers.size();++i){auto it=i<names.size()?m_motions.find(names[i]):m_motions.end();m_layers[i].first=it==m_motions.end()?nullptr:&it->second;}
        m_motion=m_previous=nullptr;m_time=m_layerTime=m_previousTime=m_mix=m_start=0;m_external=false;m_justStarted=true;
    }
    bool play(const std::string& name,bool loop,double mix=0,double time=-1) {
        auto found=m_motions.find(name);if(found==m_motions.end())return false;
        m_explicit=false;m_explicitLayers.clear();m_retainedLayers.clear();m_frameOverrides.clear();
        m_previous=m_motion;m_previousTime=m_time;m_previousLoop=m_loop;
        m_motion=&found->second;m_loop=loop;m_external=time>=0;m_time=m_external?time:0.;m_start=m_time;
        m_mix=std::max(0.,mix);m_justStarted=true;return true;
    }
    void seek(double time){if(std::isfinite(time)){m_time=std::max(0.,time);m_external=true;}}
    void update(double delta,float* parameters,float* parts) {
        if(m_explicit){updateExplicit(parameters,parts);return;}
        if(!m_external&&!m_justStarted)m_time+=std::max(0.,delta);
        m_layerTime+=std::max(0.,delta);
        if(m_motion&&!m_loop&&!m_layers.empty()&&m_time>=m_motion->duration){m_motion=m_previous=nullptr;m_time=m_previousTime=0;m_mix=0;}
        m_justStarted=false;m_parameters=m_parameterDefaults;m_parts=m_partDefaults;
        evaluatePose(m_motion,m_time,m_loop,m_parameters,m_parts);
        m_opacity=evaluateOpacity(m_motion,m_time,m_loop);
        const double elapsed=std::max(0.,m_time-m_start);
        if(m_previous&&elapsed<m_mix){
            m_oldParameters=m_parameterDefaults;m_oldParts=m_partDefaults;
            evaluatePose(m_previous,m_previousTime+elapsed,m_previousLoop,m_oldParameters,m_oldParts);
            const float weight=float(elapsed/m_mix);
            const float oldOpacity=evaluateOpacity(m_previous,m_previousTime+elapsed,m_previousLoop);m_opacity=oldOpacity+(m_opacity-oldOpacity)*weight;
            for(size_t i=0;i<m_parameters.size();++i)m_parameters[i]=m_oldParameters[i]+(m_parameters[i]-m_oldParameters[i])*weight;
            for(size_t i=0;i<m_parts.size();++i)m_parts[i]=m_oldParts[i]+(m_parts[i]-m_oldParts[i])*weight;
        }else m_previous=nullptr;
        std::copy(m_parameters.begin(),m_parameters.end(),parameters);std::copy(m_parts.begin(),m_parts.end(),parts);
    }
};
}
