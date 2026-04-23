
#include "dll_entry.h"

#if defined(SPINE_C)
#include "../backend_c/player.h"
using CDxLibSpinePlayer = CDxLibSpinePlayerC;
#elif defined(SPINE_CPP)
#include "../backend_cpp/player.h"
#endif

#if defined(SPINE_21)
#define SPINE_VERSION 21
#elif defined(SPINE_34)
#define SPINE_VERSION 34
#elif defined(SPINE_35)
#define SPINE_VERSION 35
#elif defined(SPINE_36)
#define SPINE_VERSION 36
#elif defined(SPINE_37)
#define SPINE_VERSION 37
#elif defined(SPINE_38)
#define SPINE_VERSION 38
#elif defined(SPINE_40)
#define SPINE_VERSION 40
#elif defined(SPINE_41)
#define SPINE_VERSION 41
#elif defined(SPINE_42)
#define SPINE_VERSION 42
#else
#define SPINE_VERSION 38
#endif

#define SPINE_PLAYER(VERSION) CSpinePlayer##VERSION
#define SPINE_PLAYER_VERSION(VERSION) SPINE_PLAYER(VERSION)

#define SPCLASS SPINE_PLAYER_VERSION(SPINE_VERSION)

class SPCLASS : public ISpinePlayer
{
public:
	SPCLASS() = default;
	virtual ~SPCLASS() = default;

	bool loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel) override;
	bool loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, bool isBinarySkel) override;
	bool addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel) override;
	size_t getNumberOfSpines() const noexcept override;
	size_t getSelectedSpineIndex() const noexcept override;
	bool setSelectedSpineIndex(size_t index) noexcept override;
	bool isSpineVisible(size_t index) const noexcept override;
	bool setSpineVisible(size_t index, bool visible) noexcept override;
	bool moveSpineUp(size_t index) noexcept override;
	bool moveSpineDown(size_t index) noexcept override;
	bool hasSpineBeenLoaded() const noexcept override;

	void update(float fDelta) override;
	void draw() override;

	void resetScale() override;

	void addOffset(int iX, int iY) override;

	void shiftAnimation() override;
	void shiftAnimationBack() override;
	void shiftSkin() override;

	void setAnimationByIndex(size_t nIndex) override;
	void setAnimationByName(const char* szAnimationName) override;
	void restartAnimation(bool loop = true) override;

	void setSkinByIndex(size_t nIndex) override;
	void setSkinByName(const char* szSkinName) override;
	void setupSkin() override;

	void togglePma() override;
	void toggleBlendModeAdoption() override;

	bool isAlphaPremultiplied(size_t nDrawableIndex = 0) override;
	bool isBlendModeNormalForced(size_t nDrawableIndex = 0) override;
	bool isDrawOrderReversed() const noexcept override;

	bool premultiplyAlpha(bool isToBePremultiplied, size_t nDrawableIndex = 0) override;
	bool forceBlendModeNormal(bool isToForce, size_t nDrawableIndex = 0) override;
	void setDrawOrder(bool isToBeReversed) override;

	std::string getCurrentAnimationName() override;
	std::string getCurrentSkinName() override;
	std::string getLastError() const override;
	void getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd) override;
	float getAnimationDuration(const char* animationName) override;

	const std::vector<std::string>& getSlotNames() const noexcept override;
	const std::vector<std::string>& getSkinNames() const noexcept override;
	const std::vector<std::string>& getAnimationNames() const noexcept override;

	void setSlotsToExclude(const std::vector<std::string>& slotNames) override;
	void mixSkins(const std::vector<std::string>& skinNames) override;
	void addAnimationTracks(const std::vector<std::string>& animationNames, bool loop = false) override;
	void setSlotExcludeCallback(bool (*pFunc)(const char*, size_t)) override;

	std::unordered_map<std::string, std::vector<std::string>> getSlotNamesWithTheirAttachments() override;
	bool replaceAttachment(const char* szSlotName, const char* szAttachmentName) override;

	DxLib::FLOAT2 getBaseSize() const noexcept override;
	void setBaseSize(float fWidth, float fHeight) override;
	void resetBaseSize() override;

	DxLib::FLOAT2 getOffset() const noexcept override;
	void setOffset(float fX, float fY) noexcept override;

	float getSkeletonScale() const noexcept override;
	void setSkeletonScale(float fScale) override;
	float getSkeletonScaleAt(size_t index) const noexcept override;
	bool setSkeletonScaleAt(size_t index, float fScale) noexcept override;

	float getCanvasScale() const noexcept override;
	void setCanvasScale(float fScale) override;

	float getTimeScale() const noexcept override;
	void setTimeScale(float fTimeScale) override;

	void setDefaultMix(float mixTime) override;

	bool isFlipX() const noexcept override;
	void toggleFlipX() noexcept override;
	int getRotationSteps() const noexcept override;
	void rotate90() noexcept override;

	bool isResetOffsetOnLoad() const noexcept override;
	void setResetOffsetOnLoad(bool bReset) noexcept override;

	DxLib::MATRIX calculateTransformMatrix() const noexcept override;
	DxLib::FLOAT4 getCurrentBoundingOfSlot(const std::string& slotName) const override;
	bool getSlotMeshData(const std::string& slotName, SlotMeshData& outData) const override;
	void setRenderScreenSize(int width, int height) noexcept override;
private:
	CDxLibSpinePlayer m_dxLibSpinePlayer;
};

#define CREATE_SPINE_PLAYER(VERSION) CreateSpinePlayer##VERSION
#define CREATE_SPINE_PLAYER_VERSION(VERSION) CREATE_SPINE_PLAYER(VERSION)
#define SPCREATE CREATE_SPINE_PLAYER_VERSION(SPINE_VERSION)

ISpinePlayer* SPCREATE()
{
	return new SPCLASS();
}

#define DESTROY_SPINE_PLAYER(VERSION) DestroySpinePlayer##VERSION
#define DESTROY_SPINE_PLAYER_VERSION(VERSION) DESTROY_SPINE_PLAYER(VERSION)
#define SPDESTROY DESTROY_SPINE_PLAYER_VERSION(SPINE_VERSION)

void SPDESTROY(ISpinePlayer* pSpinePlayer)
{
	delete pSpinePlayer;
}

bool SPCLASS::loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	return m_dxLibSpinePlayer.loadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
}

bool SPCLASS::loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	return m_dxLibSpinePlayer.loadSpineFromMemory(atlasData, textureDirectories, skelData, isBinarySkel);
}

bool SPCLASS::addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel)
{
	return m_dxLibSpinePlayer.addSpineFromFile(szAtlasPath, szSkelPath, isBinarySkel);
}

size_t SPCLASS::getNumberOfSpines() const noexcept
{
	return m_dxLibSpinePlayer.getNumberOfSpines();
}

size_t SPCLASS::getSelectedSpineIndex() const noexcept
{
	return m_dxLibSpinePlayer.getSelectedSpineIndex();
}

bool SPCLASS::setSelectedSpineIndex(size_t index) noexcept
{
	return m_dxLibSpinePlayer.setSelectedSpineIndex(index);
}

bool SPCLASS::isSpineVisible(size_t index) const noexcept
{
	return m_dxLibSpinePlayer.isSpineVisible(index);
}

bool SPCLASS::setSpineVisible(size_t index, bool visible) noexcept
{
	return m_dxLibSpinePlayer.setSpineVisible(index, visible);
}

bool SPCLASS::moveSpineUp(size_t index) noexcept
{
	return m_dxLibSpinePlayer.moveSpineUp(index);
}

bool SPCLASS::moveSpineDown(size_t index) noexcept
{
	return m_dxLibSpinePlayer.moveSpineDown(index);
}

bool SPCLASS::hasSpineBeenLoaded() const noexcept
{
	return m_dxLibSpinePlayer.hasSpineBeenLoaded();
}

void SPCLASS::update(float fDelta)
{
	m_dxLibSpinePlayer.update(fDelta);
}

void SPCLASS::draw()
{
	m_dxLibSpinePlayer.draw();
}

void SPCLASS::resetScale()
{
	m_dxLibSpinePlayer.resetScale();
}

void SPCLASS::addOffset(int iX, int iY)
{
	m_dxLibSpinePlayer.addOffset(iX, iY);
}

void SPCLASS::shiftAnimation()
{
	m_dxLibSpinePlayer.shiftAnimation();
}

void SPCLASS::shiftAnimationBack()
{
	m_dxLibSpinePlayer.shiftAnimationBack();
}

void SPCLASS::shiftSkin()
{
	m_dxLibSpinePlayer.shiftSkin();
}

void SPCLASS::setAnimationByIndex(size_t nIndex)
{
	m_dxLibSpinePlayer.setAnimationByIndex(nIndex);
}

void SPCLASS::setAnimationByName(const char* szAnimationName)
{
	m_dxLibSpinePlayer.setAnimationByName(szAnimationName);
}

void SPCLASS::restartAnimation(bool loop)
{
	m_dxLibSpinePlayer.restartAnimation(loop);
}

void SPCLASS::setSkinByIndex(size_t nIndex)
{
	m_dxLibSpinePlayer.setSkinByIndex(nIndex);
}

void SPCLASS::setSkinByName(const char* szSkinName)
{
	m_dxLibSpinePlayer.setSkinByName(szSkinName);
}

void SPCLASS::setupSkin()
{
	m_dxLibSpinePlayer.setupSkin();
}

void SPCLASS::togglePma()
{
	m_dxLibSpinePlayer.togglePma();
}

void SPCLASS::toggleBlendModeAdoption()
{
	m_dxLibSpinePlayer.toggleBlendModeAdoption();
}

bool SPCLASS::isAlphaPremultiplied(size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.isAlphaPremultiplied(nDrawableIndex);
}

bool SPCLASS::isBlendModeNormalForced(size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.isBlendModeNormalForced(nDrawableIndex);
}

bool SPCLASS::isDrawOrderReversed() const noexcept
{
	return m_dxLibSpinePlayer.isDrawOrderReversed();
}

bool SPCLASS::premultiplyAlpha(bool isToBePremultiplied, size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.premultiplyAlpha(isToBePremultiplied, nDrawableIndex);
}

bool SPCLASS::forceBlendModeNormal(bool isToForce, size_t nDrawableIndex)
{
	return m_dxLibSpinePlayer.forceBlendModeNormal(isToForce, nDrawableIndex);
}

void SPCLASS::setDrawOrder(bool isToBeReversed)
{
	m_dxLibSpinePlayer.setDrawOrder(isToBeReversed);
}

std::string SPCLASS::getCurrentAnimationName()
{
	return m_dxLibSpinePlayer.getCurrentAnimationName();
}

std::string SPCLASS::getCurrentSkinName()
{
	return m_dxLibSpinePlayer.getCurrentSkinName();
}

std::string SPCLASS::getLastError() const
{
	return m_dxLibSpinePlayer.getLastError();
}

float SPCLASS::getAnimationDuration(const char* animationName)
{
	return m_dxLibSpinePlayer.getAnimationDuration(animationName);
}

void SPCLASS::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	m_dxLibSpinePlayer.getCurrentAnimationTime(fTrack, fLast, fStart, fEnd);
}

const std::vector<std::string>& SPCLASS::getSlotNames() const noexcept
{
	return m_dxLibSpinePlayer.getSlotNames();
}

const std::vector<std::string>& SPCLASS::getSkinNames() const noexcept
{
	return m_dxLibSpinePlayer.getSkinNames();
}

const std::vector<std::string>& SPCLASS::getAnimationNames() const noexcept
{
	return m_dxLibSpinePlayer.getAnimationNames();
}

void SPCLASS::setSlotsToExclude(const std::vector<std::string>& slotNames)
{
	m_dxLibSpinePlayer.setSlotsToExclude(slotNames);
}

void SPCLASS::mixSkins(const std::vector<std::string>& skinNames)
{
	m_dxLibSpinePlayer.mixSkins(skinNames);
}

void SPCLASS::addAnimationTracks(const std::vector<std::string>& animationNames, bool loop)
{
	m_dxLibSpinePlayer.addAnimationTracks(animationNames, loop);
}

void SPCLASS::setSlotExcludeCallback(bool(*pFunc)(const char*, size_t))
{
	m_dxLibSpinePlayer.setSlotExcludeCallback(pFunc);
}

std::unordered_map<std::string, std::vector<std::string>> SPCLASS::getSlotNamesWithTheirAttachments()
{
	return m_dxLibSpinePlayer.getSlotNamesWithTheirAttachments();
}

bool SPCLASS::replaceAttachment(const char* szSlotName, const char* szAttachmentName)
{
	return m_dxLibSpinePlayer.replaceAttachment(szSlotName, szAttachmentName);
}

DxLib::FLOAT2 SPCLASS::getBaseSize() const noexcept
{
	const auto& size = m_dxLibSpinePlayer.getBaseSize();
	return { size.x, size.y };
}

void SPCLASS::setBaseSize(float fWidth, float fHeight)
{
	m_dxLibSpinePlayer.setBaseSize(fWidth, fHeight);
}

void SPCLASS::resetBaseSize()
{
	m_dxLibSpinePlayer.resetBaseSize();
}

DxLib::FLOAT2 SPCLASS::getOffset() const noexcept
{
	const auto& offset = m_dxLibSpinePlayer.getOffset();
	return { offset.x, offset.y };
}
void SPCLASS::setOffset(float fX, float fY) noexcept
{
	m_dxLibSpinePlayer.setOffset(fX, fY);
}

float SPCLASS::getSkeletonScale() const noexcept
{
	return m_dxLibSpinePlayer.getSkeletonScale();
}

void SPCLASS::setSkeletonScale(float fScale)
{
	m_dxLibSpinePlayer.setSkeletonScale(fScale);
}

float SPCLASS::getSkeletonScaleAt(size_t index) const noexcept
{
	return m_dxLibSpinePlayer.getSkeletonScaleAt(index);
}

bool SPCLASS::setSkeletonScaleAt(size_t index, float fScale) noexcept
{
	return m_dxLibSpinePlayer.setSkeletonScaleAt(index, fScale);
}

float SPCLASS::getCanvasScale() const noexcept
{
	return m_dxLibSpinePlayer.getCanvasScale();
}

void SPCLASS::setCanvasScale(float fScale)
{
	m_dxLibSpinePlayer.setCanvasScale(fScale);
}

float SPCLASS::getTimeScale() const noexcept
{
	return m_dxLibSpinePlayer.getTimeScale();
}

void SPCLASS::setTimeScale(float fTimeScale)
{
	m_dxLibSpinePlayer.setTimeScale(fTimeScale);
}

void SPCLASS::setDefaultMix(float mixTime)
{
	m_dxLibSpinePlayer.setDefaultMix(mixTime);
}

bool SPCLASS::isFlipX() const noexcept
{
	return m_dxLibSpinePlayer.isFlipX();
}

void SPCLASS::toggleFlipX() noexcept
{
	m_dxLibSpinePlayer.toggleFlipX();
}

int SPCLASS::getRotationSteps() const noexcept
{
	return m_dxLibSpinePlayer.getRotationSteps();
}

void SPCLASS::rotate90() noexcept
{
	m_dxLibSpinePlayer.rotate90();
}

bool SPCLASS::isResetOffsetOnLoad() const noexcept
{
	return m_dxLibSpinePlayer.isResetOffsetOnLoad();
}

void SPCLASS::setResetOffsetOnLoad(bool bReset) noexcept
{
	m_dxLibSpinePlayer.setResetOffsetOnLoad(bReset);
}

DxLib::MATRIX SPCLASS::calculateTransformMatrix() const noexcept
{
	return m_dxLibSpinePlayer.calculateTransformMatrix();
}

DxLib::FLOAT4 SPCLASS::getCurrentBoundingOfSlot(const std::string& slotName) const
{
	return m_dxLibSpinePlayer.getCurrentBoundingOfSlot(slotName);
}

bool SPCLASS::getSlotMeshData(const std::string& slotName, SlotMeshData& outData) const
{
	return m_dxLibSpinePlayer.getSlotMeshData(slotName, outData);
}

void SPCLASS::setRenderScreenSize(int width, int height) noexcept
{
	m_dxLibSpinePlayer.setRenderScreenSize(width, height);
}
