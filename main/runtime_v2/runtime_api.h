#ifndef SPINELOVE_RUNTIME_V2_API_H_
#define SPINELOVE_RUNTIME_V2_API_H_

#include <memory>
#include <string>
#include <vector>

namespace sl_runtime_v2 {

enum class RuntimeKind
{
	Cpp21,
	Cpp34,
	Cpp35,
	Cpp36,
	Cpp37,
	Cpp38,
	Cpp40,
	Cpp41,
	Cpp42,
};

struct RuntimeInfo
{
	RuntimeKind kind;
	const char* versionPrefix;
	const char* displayName;
};

struct LoadRequest
{
	std::vector<std::string> atlasPaths;
	std::vector<std::string> skeletonPaths;
	std::vector<std::string> atlasData;
	std::vector<std::string> textureDirectories;
	std::vector<std::string> skeletonData;
	bool binarySkeleton = false;
};

enum class BlendMode
{
	Normal,
	Additive,
	Multiply,
	Screen,
};

struct Color
{
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;
};

struct Vertex
{
	float x = 0.0f;
	float y = 0.0f;
	float u = 0.0f;
	float v = 0.0f;
	Color color;
};

struct DrawCommand
{
	unsigned long long textureId = 0;
	std::string slotName;
	BlendMode blendMode = BlendMode::Normal;
	bool premultipliedAlpha = false;
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
};

struct TextureInfo
{
	unsigned long long id = 0;
	std::string path;
	bool sourcePremultipliedAlpha = true;
	bool renderPremultipliedAlpha = true;
	bool hasPremultipliedAlphaMetadata = false;
};

struct Frame
{
	int width = 0;
	int height = 0;
	std::vector<DrawCommand> draws;
};

class IRuntime
{
public:
	virtual ~IRuntime() = default;

	virtual RuntimeInfo Info() const noexcept = 0;
	virtual bool Load(const LoadRequest& request) = 0;
	virtual void Clear() noexcept = 0;
	virtual bool HasSkeleton() const noexcept = 0;

	virtual void Update(float deltaSeconds) = 0;
	virtual void BuildFrame(int width, int height, Frame& outFrame) = 0;

	virtual const std::vector<std::string>& MotionNames() const noexcept = 0;
	virtual const std::vector<std::string>& LookNames() const noexcept = 0;
	virtual const std::vector<std::string>& SlotCatalog() const noexcept = 0;
	virtual const std::vector<TextureInfo>& TextureInfos() const noexcept = 0;
	virtual void StartMotion(const char* name, bool loop) = 0;
	virtual float MotionDuration(const char* name) const = 0;
	virtual void SetMotionBlendSeconds(float seconds) = 0;
	virtual void SetSecondaryMotions(const std::vector<std::string>& names, bool loop) = 0;
	virtual void ApplyLook(const char* name) = 0;
	virtual std::string LastError() const = 0;
};

std::unique_ptr<IRuntime> CreateCpp21Runtime();
std::unique_ptr<IRuntime> CreateCpp34Runtime();
std::unique_ptr<IRuntime> CreateCpp35Runtime();
std::unique_ptr<IRuntime> CreateCpp36Runtime();
std::unique_ptr<IRuntime> CreateCpp37Runtime();
std::unique_ptr<IRuntime> CreateCpp38Runtime();
std::unique_ptr<IRuntime> CreateCpp40Runtime();
std::unique_ptr<IRuntime> CreateCpp41Runtime();
std::unique_ptr<IRuntime> CreateCpp42Runtime();

}

#endif
