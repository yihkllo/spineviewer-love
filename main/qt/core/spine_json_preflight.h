#pragma once
#include <QByteArray>
#include <charconv>
#include <cmath>
#include <string>
#include <vector>

namespace slqt {
class SpineJsonPreflight final {
    static constexpr int MaxDepth=1024;
    const unsigned char* p;
    const unsigned char* const end;
    int depth=0;
    bool bonesArray=false,hasBones=false;
    std::string rootKey;
    std::vector<unsigned char> stack;
    SpineJsonPreflight(const unsigned char* begin,const unsigned char* finish):p(begin),end(finish){}
    void whitespace(){while(p<end&&(*p==' '||*p=='\t'||*p=='\n'||*p=='\r'))++p;}
    void value(){
        if(depth==1&&rootKey=="bones")hasBones=false;
        if(depth==2&&bonesArray)hasBones=true;
    }
    bool hex4(unsigned& codepoint){
        if(end-p<4)return false;
        codepoint=0;
        for(int i=0;i<4;++i){
            const unsigned c=p[i];
            unsigned digit;
            if(c>='0'&&c<='9')digit=c-'0';
            else if(c>='a'&&c<='f')digit=c-'a'+10;
            else if(c>='A'&&c<='F')digit=c-'A'+10;
            else return false;
            codepoint=codepoint*16+digit;
        }
        p+=4;return true;
    }
    static void appendUtf8(std::string& out,unsigned cp){
        if(cp<0x80)out.push_back(char(cp));
        else if(cp<0x800){out.push_back(char(0xC0|(cp>>6)));out.push_back(char(0x80|(cp&0x3F)));}
        else if(cp<0x10000){out.push_back(char(0xE0|(cp>>12)));out.push_back(char(0x80|((cp>>6)&0x3F)));out.push_back(char(0x80|(cp&0x3F)));}
        else{out.push_back(char(0xF0|(cp>>18)));out.push_back(char(0x80|((cp>>12)&0x3F)));out.push_back(char(0x80|((cp>>6)&0x3F)));out.push_back(char(0x80|(cp&0x3F)));}
    }
    bool continuation(int count){
        for(int i=0;i<count;++i){if(p>=end||(*p&0xC0)!=0x80)return false;++p;}
        return true;
    }
    bool within(unsigned char low,unsigned char high){
        if(p>=end||*p<low||*p>high)return false;
        ++p;return true;
    }
    bool string(std::string* decoded){
        while(p<end){
            const unsigned char c=*p++;
            if(c=='"')return true;
            if(c<0x20)return false;
            if(c=='\\'){
                if(p>=end)return false;
                const unsigned char escape=*p++;
                char plain=0;
                switch(escape){
                case '"':case '\\':case '/':plain=char(escape);break;
                case 'b':plain='\b';break;
                case 'f':plain='\f';break;
                case 'n':plain='\n';break;
                case 'r':plain='\r';break;
                case 't':plain='\t';break;
                case 'u':{
                    unsigned cp;
                    if(!hex4(cp))return false;
                    if(cp>=0xD800&&cp<=0xDBFF){
                        unsigned low;
                        if(end-p<2||p[0]!='\\'||p[1]!='u')return false;
                        p+=2;
                        if(!hex4(low)||low<0xDC00||low>0xDFFF)return false;
                        cp=0x10000+((cp-0xD800)<<10)+(low-0xDC00);
                    }else if(cp>=0xDC00&&cp<=0xDFFF)return false;
                    if(decoded)appendUtf8(*decoded,cp);
                    continue;
                }
                default:return false;
                }
                if(decoded)decoded->push_back(plain);
                continue;
            }
            if(c<0x80){if(decoded)decoded->push_back(char(c));continue;}
            const unsigned char* start=p-1;
            bool ok=false;
            if(c>=0xC2&&c<=0xDF)ok=continuation(1);
            else if(c==0xE0)ok=within(0xA0,0xBF)&&continuation(1);
            else if((c>=0xE1&&c<=0xEC)||c==0xEE||c==0xEF)ok=continuation(2);
            else if(c==0xED)ok=within(0x80,0x9F)&&continuation(1);
            else if(c==0xF0)ok=within(0x90,0xBF)&&continuation(2);
            else if(c>=0xF1&&c<=0xF3)ok=continuation(3);
            else if(c==0xF4)ok=within(0x80,0x8F)&&continuation(2);
            if(!ok)return false;
            if(decoded)decoded->append(reinterpret_cast<const char*>(start),size_t(p-start));
        }
        return false;
    }
    bool digits(){
        if(p>=end||*p<'0'||*p>'9')return false;
        while(p<end&&*p>='0'&&*p<='9')++p;
        return true;
    }
    bool number(){
        const unsigned char* start=p;
        if(*p=='-')++p;
        const unsigned char* integer=p;
        if(p<end&&*p=='0')++p;
        else if(!digits())return false;
        const unsigned char *integerEnd=p,*fraction=p,*fractionEnd=p;
        if(p<end&&*p=='.'){++p;fraction=p;if(!digits())return false;fractionEnd=p;}
        long exponent=0;
        if(p<end&&(*p=='e'||*p=='E')){
            ++p;
            const bool negative=p<end&&*p=='-';
            if(p<end&&(*p=='+'||*p=='-'))++p;
            const unsigned char* digitsStart=p;
            if(!digits())return false;
            for(auto* d=digitsStart;d<p;++d)if(exponent<1000000)exponent=exponent*10+(*d-'0');
            if(negative)exponent=-exponent;
        }
        long magnitude=0;bool zero=true;
        for(auto* d=integer;d<integerEnd;++d)if(*d!='0'){magnitude=long(integerEnd-d)-1;zero=false;break;}
        if(zero)for(auto* d=fraction;d<fractionEnd;++d)if(*d!='0'){magnitude=-long(d-fraction)-1;zero=false;break;}
        if(zero||magnitude+exponent<308)return true;
        double parsed=0;
        const auto result=std::from_chars(reinterpret_cast<const char*>(start),reinterpret_cast<const char*>(p),parsed);
        return result.ec==std::errc{}&&std::isfinite(parsed);
    }
    bool literal(const char* word){
        for(;*word;++word,++p)if(p>=end||*p!=static_cast<unsigned char>(*word))return false;
        return true;
    }
    bool key(){
        whitespace();
        if(p>=end||*p!='"')return false;
        ++p;
        if(depth==1){rootKey.clear();if(!string(&rootKey))return false;}
        else if(!string(nullptr))return false;
        whitespace();
        if(p>=end||*p!=':')return false;
        ++p;return true;
    }
    void close(){
        if(stack.back()=='['&&depth==2)bonesArray=false;
        ++p;--depth;stack.pop_back();
    }
    bool run(){
        if(p<end&&*p==0xEF){
            if(end-p<3||p[1]!=0xBB||p[2]!=0xBF)return false;
            p+=3;
        }
        whitespace();
        if(p>=end||*p!='{')return false;
        bool expectValue=true;
        for(;;){
            whitespace();
            if(expectValue){
                if(p>=end)return false;
                const unsigned char c=*p;
                value();
                if(c=='{'||c=='['){
                    if(depth>=MaxDepth)return false;
                    if(c=='['&&depth==1)bonesArray=rootKey=="bones";
                    ++depth;stack.push_back(c);++p;
                    whitespace();
                    if(p<end&&*p==(c=='{'?'}':']')){close();expectValue=false;continue;}
                    if(c=='{'&&!key())return false;
                    continue;
                }
                bool ok;
                if(c=='"'){++p;ok=string(nullptr);}
                else if(c=='-'||(c>='0'&&c<='9'))ok=number();
                else if(c=='t')ok=literal("true");
                else if(c=='f')ok=literal("false");
                else if(c=='n')ok=literal("null");
                else ok=false;
                if(!ok)return false;
                expectValue=false;
                continue;
            }
            if(stack.empty())return p==end;
            if(p>=end)return false;
            const unsigned char top=stack.back();
            if(*p==','){
                ++p;
                if(top=='{'&&!key())return false;
                expectValue=true;
                continue;
            }
            if(*p==(top=='{'?'}':']')){close();continue;}
            return false;
        }
    }
public:
    static bool valid(const QByteArray& bytes){
        const auto* begin=reinterpret_cast<const unsigned char*>(bytes.constData());
        SpineJsonPreflight check(begin,begin+bytes.size());
        return check.run()&&check.hasBones;
    }
};
}
