#ifndef SPINELOVE_DXLIB_SHARED_DLL_ENTRY_H_
#define SPINELOVE_DXLIB_SHARED_DLL_ENTRY_H_

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#include "slot_mesh_data.h"

#if defined(_WIN32)
#if defined(SPINE_RUNTIME_DLL_BUILD)
#define SPINE_EXTERN __declspec(dllexport)
#else
#define SPINE_EXTERN __declspec(dllimport)
#endif
#else
#define SPINE_EXTERN
#endif

class __declspec(novtable) ISpinePlayer
{
public:
	virtual ~ISpinePlayer() = default;

	virtual bool loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel) = 0;
	virtual bool loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, bool isBinarySkel) = 0;

	virtual bool addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel) = 0;

	virtual size_t getNumberOfSpines() const noexcept = 0;
	virtual bool hasSpineBeenLoaded() const noexcept = 0;

	virtual void update(float fDelta) = 0;
	virtual void draw() = 0;

	virtual void resetScale() = 0;

	virtual void addOffset(int iX, int iY) = 0;

	virtual void shiftAnimation() = 0;
	virtual void shiftAnimationBack() = 0;
	virtual void shiftSkin() = 0;

	virtual void setAnimationByIndex(size_t nIndex) = 0;
	virtual void setAnimationByName(const char* szAnimationName) = 0;
	virtual void restartAnimation(bool loop = true) = 0;

	virtual void setSkinByIndex(size_t nIndex) = 0;
	virtual void setSkinByName(const char* szSkinName) = 0;
	virtual void setupSkin() = 0;


	virtual void togglePma() = 0;
	virtual void toggleBlendModeAdoption() = 0;


	virtual bool isAlphaPremultiplied(size_t nDrawableIndex = 0) = 0;
	virtual bool isBlendModeNormalForced(size_t nDrawableIndex = 0) = 0;
	virtual bool isDrawOrderReversed() const noexcept = 0;

	virtual bool premultiplyAlpha(bool isToBePremultiplied, size_t nDrawableIndex = 0) = 0;
	virtual bool forceBlendModeNormal(bool isToForce, size_t nDrawableIndex = 0) = 0;
	virtual void setDrawOrder(bool isToBeReversed) = 0;

	virtual std::string getCurrentAnimationName() = 0;
	virtual std::string getCurrentSkinName() = 0;
	virtual void getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd) = 0;
	virtual float getAnimationDuration(const char* animationName) = 0;

	virtual const std::vector<std::string>& getSlotNames() const noexcept = 0;
	virtual const std::vector<std::string>& getSkinNames() const noexcept = 0;
	virtual const std::vector<std::string>& getAnimationNames() const noexcept = 0;

	virtual void setSlotsToExclude(const std::vector<std::string>& slotNames) = 0;
	virtual void mixSkins(const std::vector<std::string>& skinNames) = 0;
	virtual void addAnimationTracks(const std::vector<std::string>& animationNames, bool loop = false) = 0;
	virtual void setSlotExcludeCallback(bool (*pFunc)(const char*, size_t)) = 0;


	virtual std::unordered_map<std::string, std::vector<std::string>> getSlotNamesWithTheirAttachments() = 0;
	virtual bool replaceAttachment(const char* szSlotName, const char* szAttachmentName) = 0;


	virtual DxLib::FLOAT2 getBaseSize() const noexcept = 0;
	virtual void setBaseSize(float fWidth, float fHeight) = 0;
	virtual void resetBaseSize() = 0;

	virtual DxLib::FLOAT2 getOffset() const noexcept = 0;
	virtual void setOffset(float fX, float fY) noexcept = 0;

	virtual float getSkeletonScale() const noexcept = 0;
	virtual void setSkeletonScale(float fScale) = 0;

	virtual float getCanvasScale() const noexcept = 0;
	virtual void setCanvasScale(float fScale) = 0;

	virtual float getTimeScale() const noexcept = 0;
	virtual void setTimeScale(float fTimeScale) = 0;

	virtual void setDefaultMix(float mixTime) = 0;

	virtual bool isFlipX() const noexcept = 0;
	virtual void toggleFlipX() noexcept = 0;
	virtual int getRotationSteps() const noexcept = 0;
	virtual void rotate90() noexcept = 0;

	virtual bool isResetOffsetOnLoad() const noexcept = 0;
	virtual void setResetOffsetOnLoad(bool bReset) noexcept = 0;

	virtual DxLib::MATRIX calculateTransformMatrix() const noexcept = 0;
	virtual DxLib::FLOAT4 getCurrentBoundingOfSlot(const std::string& slotName) const = 0;
	virtual bool getSlotMeshData(const std::string& slotName, SlotMeshData& outData) const = 0;

	virtual void setRenderScreenSize(int width, int height) noexcept = 0;
private:

};

extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer21();
extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer34();
extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer35();
extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer36();
extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer37();
extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer38();
extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer40();
extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer41();
extern "C" SPINE_EXTERN ISpinePlayer* CreateSpinePlayer42();

extern "C" SPINE_EXTERN void DestroySpinePlayer21(ISpinePlayer* pSpinePlayer);
extern "C" SPINE_EXTERN void DestroySpinePlayer34(ISpinePlayer* pSpinePlayer);
extern "C" SPINE_EXTERN void DestroySpinePlayer35(ISpinePlayer* pSpinePlayer);
extern "C" SPINE_EXTERN void DestroySpinePlayer36(ISpinePlayer* pSpinePlayer);
extern "C" SPINE_EXTERN void DestroySpinePlayer37(ISpinePlayer* pSpinePlayer);
extern "C" SPINE_EXTERN void DestroySpinePlayer38(ISpinePlayer* pSpinePlayer);
extern "C" SPINE_EXTERN void DestroySpinePlayer40(ISpinePlayer* pSpinePlayer);
extern "C" SPINE_EXTERN void DestroySpinePlayer41(ISpinePlayer* pSpinePlayer);
extern "C" SPINE_EXTERN void DestroySpinePlayer42(ISpinePlayer* pSpinePlayer);

#endif

