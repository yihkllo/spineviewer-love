#ifndef SPINE_PLAYER_C_H_
#define SPINE_PLAYER_C_H_

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

using FPoint2 = struct { float x, y; };
#ifdef SPINE_21
#include "Spine21/dxlib_spine_c_21.h"
using CSpineDrawableC = CDxLibSpineDrawableC21;
#elif defined(SPINE_34)
#include "Spine34/dxlib_spine_c_34.h"
using CSpineDrawableC = CDxLibSpineDrawableC34;
#else
#include "drawable.h"
using CSpineDrawableC = CDxLibSpineDrawableC;
#endif


class CSpinePlayerC
{
public:
	struct SDrawableState
	{
		FPoint2 offset{};
		float skeletonScale = 1.f;
		bool visible = true;
		bool flipX = false;
		int rotationSteps = 0;
	};

	CSpinePlayerC();
	virtual ~CSpinePlayerC();

	bool loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel);
	bool loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel);
	bool addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel);

	size_t getNumberOfSpines() const noexcept;
	size_t getSelectedSpineIndex() const noexcept;
	bool setSelectedSpineIndex(size_t index) noexcept;
	bool isSpineVisible(size_t index) const noexcept;
	bool setSpineVisible(size_t index, bool visible) noexcept;
	bool moveSpineUp(size_t index) noexcept;
	bool moveSpineDown(size_t index) noexcept;
	bool hasSpineBeenLoaded() const noexcept;

	void update(float fDelta);

	void resetScale();

	void addOffset(int iX, int iY);

	void shiftAnimation();
	void shiftAnimationBack();
	void shiftSkin();

	void setAnimationByIndex(size_t nIndex);
	void setAnimationByName(const char* szAnimationName);
	void restartAnimation(bool loop = true);

	void setSkinByIndex(size_t nIndex);
	void setSkinByName(const char* szSkinName);
	void setupSkin();

	void togglePma();
	void toggleBlendModeAdoption();

	bool isAlphaPremultiplied(size_t nDrawableIndex = 0);
	bool isBlendModeNormalForced(size_t nDrawableIndex = 0);
	bool isDrawOrderReversed() const noexcept;

	bool premultiplyAlpha(bool premultiplied, size_t nDrawableIndex = 0);
	bool forceBlendModeNormal(bool toForce, size_t nDrawableIndex = 0);
	void setDrawOrder(bool reversed);

	std::string getCurrentAnimationName();
	std::string getCurrentSkinName();
	std::string getLastError() const;
	void getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd);
	float getAnimationDuration(const char* animationName);

	const std::vector<std::string>& getSlotNames() const noexcept;
	const std::vector<std::string>& getSkinNames() const noexcept;
	const std::vector<std::string>& getAnimationNames() const noexcept;

	void setSlotsToExclude(const std::vector<std::string>& slotNames);
	void mixSkins(const std::vector<std::string>& skinNames);
	void addAnimationTracks(const std::vector<std::string>& animationNames, bool loop = false);
	void setSlotExcludeCallback(bool (*pFunc)(const char*, size_t));

	std::unordered_map<std::string, std::vector<std::string>> getSlotNamesWithTheirAttachments();
	bool replaceAttachment(const char* szSlotName, const char* szAttachmentName);

	FPoint2 getBaseSize() const noexcept;
	void setBaseSize(float fWidth, float fHeight);
	void resetBaseSize();

	FPoint2 getOffset() const noexcept;
	void setOffset(float fX, float fY) noexcept;

	float getSkeletonScale() const noexcept;
	void setSkeletonScale(float fScale) noexcept;
	float getSkeletonScaleAt(size_t index) const noexcept;
	bool setSkeletonScaleAt(size_t index, float fScale) noexcept;

	float getCanvasScale() const noexcept;
	void setCanvasScale(float fScale) noexcept;

	float getTimeScale() const noexcept;
	void setTimeScale(float fTimeScale) noexcept;

	void setDefaultMix(float mixTime);

	bool isFlipX() const noexcept;
	void toggleFlipX() noexcept;
	int getRotationSteps() const noexcept;
	void rotate90() noexcept;

	bool isResetOffsetOnLoad() const noexcept;
	void setResetOffsetOnLoad(bool bReset) noexcept;
protected:
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

	std::vector<std::shared_ptr<spAtlas>> m_atlases;
	std::vector<std::shared_ptr<spSkeletonData>> m_skeletonData;
	std::vector<std::unique_ptr<CSpineDrawableC>> m_drawables;
	std::vector<SDrawableState> m_drawableStates;
	std::vector<std::vector<std::string>> m_drawableAnimationNames;
	std::vector<std::vector<std::string>> m_drawableSkinNames;
	std::vector<std::vector<std::string>> m_drawableSlotNames;
	size_t m_selectedDrawableIndex = 0;
	FPoint2 m_fBaseSize = FPoint2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	FPoint2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	float m_fCanvasScale = 1.f;
	FPoint2 m_fOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	std::vector<std::string> m_slotNames;
	std::string m_lastError;

	bool m_isDrawOrderReversed = false;
	bool m_bFlipX = false;
	int m_iRotationSteps = 0;
	bool m_bResetOffsetOnLoad = false;

	void clearDrawables();
	bool addDrawable(spSkeletonData* pSkeletonData);
	bool setupDrawables();

	void workOutDefaultSize();
	virtual void workOutDefaultScale() = 0;
	virtual void workOutDefaultOffset() = 0;

	SDrawableState* getSelectedDrawableState() noexcept;
	const SDrawableState* getSelectedDrawableState() const noexcept;
	CSpineDrawableC* getSelectedDrawable() const noexcept;
	void syncSelectionIndices();
	void refreshSelectedResourceLists();
	void applyDrawablePosition(size_t index);
	void updatePosition();

	void clearAnimationTracks();
};

#endif // !SPINE_PLAYER_C_H_
