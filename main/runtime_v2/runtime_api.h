#ifndef SPINELOVE_RUNTIME_V2_API_H_
#define SPINELOVE_RUNTIME_V2_API_H_

#include <memory>
#include <string>
#include <vector>
#include <array>
#include "spinelove/native_motion_track.h"

namespace sl_runtime_v2 {

enum class RuntimeKind
{
	Cpp21,
	Cpp31,
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

struct MaskDrawCommand
{
	unsigned long long textureId = 0;
	bool premultipliedAlpha = false;
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
};

struct DrawCommand
{
	unsigned long long textureId = 0;
	std::string slotName;
	std::string attachmentName;
	BlendMode blendMode = BlendMode::Normal;
	bool premultipliedAlpha = false;
	bool invertedMask = false;
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
	std::vector<MaskDrawCommand> masks;
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

struct AnimationEvent
{
	std::string name;
	std::string stringValue;
	int intValue = 0;
	float floatValue = 0.0f;
	float time = 0.0f;
};

struct AnimationCompletion
{
	std::string animation;
	int track = 0;
	float time = 0.0f;
};

enum class SlotAttachmentMode
{
	Preserve,
	SetupIfEmpty,
	NamedIfEmpty,
	Clear,
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
	virtual bool StartMotionWithMix(const char* name, bool loop, float mixSeconds)
	{
		StartMotion(name, loop);
		return true;
	}
	virtual bool QueueMotion(const char*, bool, float) { return false; }
	virtual bool SetCurrentMotionTimeScale(float) noexcept { return false; }
	virtual float MotionDuration(const char* name) const = 0;
	virtual void SetMotionBlendSeconds(float seconds) = 0;
	virtual void SetSecondaryMotions(const std::vector<std::string>& names, bool loop) = 0;
	virtual bool StartMotionOnTrack(int, const char*, bool, float) { return false; }
	virtual bool ClearMotionTrack(int, float) { return false; }
	virtual bool HoldMotionTrack(int, float) { return false; }
	virtual bool SetNativeMotionTrack(const SlNativeMotionTrack&) { return false; }
	virtual void ApplyLook(const char* name) = 0;
	virtual void ComposeLooks(const std::vector<std::string>& names) = 0;
	virtual bool SetNativeSkinMix(const std::vector<std::string>&) { return false; }
	virtual bool SetSlotOverride(const char*, float, SlotAttachmentMode) { return false; }
	virtual void ClearSlotOverrides() {}
	virtual void DrainAnimationEvents(std::vector<AnimationEvent>& events) { events.clear(); }
	virtual void EnableMotionCompletions(bool) {}
	virtual void DrainMotionCompletions(std::vector<AnimationCompletion>& events) { events.clear(); }
	virtual std::string LastError() const = 0;
	virtual bool ReadBoneTransform(const char*, std::array<float, 6>&) const { return false; }
};

std::unique_ptr<IRuntime> CreateCpp21Runtime();
std::unique_ptr<IRuntime> CreateCpp31Runtime();
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
