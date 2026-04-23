
#include <spine/extension.h>

#include "controller.h"
#include "loader.h"

CSpinePlayerC::CSpinePlayerC()
{

}

CSpinePlayerC::~CSpinePlayerC()
{

}

bool CSpinePlayerC::loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	m_lastError.clear();
	if (atlasPaths.size() != skelPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromFile(strAtlasPath.c_str(), nullptr);
		if (atlas.get() == nullptr)
		{
			m_lastError = "Failed to create atlas from file: " + strAtlasPath;
			continue;
		}

		std::shared_ptr<spSkeletonData> skeletonData = isBinarySkel ?
			spine_loader_c::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)
		{
			m_lastError = "Failed to parse skeleton: " + strSkeletonPath;
			continue;
		}

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}
bool CSpinePlayerC::loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	m_lastError.clear();
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData[i];
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonData = skelData[i];

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromMemory(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), nullptr);
		if (atlas.get() == nullptr)
		{
			m_lastError = "Failed to create atlas from memory: " + strAtlasPath;
			continue;
		}

		std::shared_ptr<spSkeletonData> skeletonData = isBinarySkel ?
			spine_loader_c::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>((strSkeletonData.c_str())), static_cast<int>(strSkeletonData.size()), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromMemory(strSkeletonData.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)
		{
			m_lastError = "Failed to parse skeleton from memory: " + strAtlasPath;
			continue;
		}

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}

bool CSpinePlayerC::addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel)
{
	if (m_drawables.empty() || szAtlasPath == nullptr || szSkelPath == nullptr)return false;

	std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromFile(szAtlasPath, nullptr);
	if (atlas.get() == nullptr)return false;

	std::shared_ptr<spSkeletonData> skeletonData = isBinarySkel ?
		spine_loader_c::ReadBinarySkeletonFromFile(szSkelPath, atlas.get()) :
		spine_loader_c::ReadTextSkeletonFromFile(szSkelPath, atlas.get());
	if (skeletonData.get() == nullptr)return false;

	const size_t newIndex = m_drawables.size();
	if (!addDrawable(skeletonData.get()))return false;

	m_atlases.push_back(std::move(atlas));
	m_skeletonData.push_back(std::move(skeletonData));

	const auto& addedSkeletonData = m_skeletonData.back();
	std::vector<std::string> animationNames;
	for (size_t i = 0; i < addedSkeletonData->animationsCount; ++i)
	{
		const char* szAnimationName = addedSkeletonData->animations[i]->name;
		if (szAnimationName == nullptr)continue;
		const auto& iter = std::find(animationNames.begin(), animationNames.end(), szAnimationName);
		if (iter == animationNames.cend())animationNames.push_back(szAnimationName);
	}
	m_drawableAnimationNames.push_back(std::move(animationNames));

	std::vector<std::string> skinNames;
	for (size_t i = 0; i < addedSkeletonData->skinsCount; ++i)
	{
		const char* szSkinName = addedSkeletonData->skins[i]->name;
		if (szSkinName == nullptr)continue;
		const auto& iter = std::find(skinNames.begin(), skinNames.end(), szSkinName);
		if (iter == skinNames.cend())skinNames.push_back(szSkinName);
	}
	m_drawableSkinNames.push_back(std::move(skinNames));

	std::vector<std::string> slotNames;
	for (size_t i = 0; i < addedSkeletonData->slotsCount; ++i)
	{
		const char* szSlotName = addedSkeletonData->slots[i]->name;
		if (szSlotName == nullptr)continue;
		const auto& iter = std::find(slotNames.begin(), slotNames.end(), szSlotName);
		if (iter == slotNames.cend())slotNames.push_back(szSlotName);
	}
	m_drawableSlotNames.push_back(std::move(slotNames));

	m_selectedDrawableIndex = newIndex;
	for (size_t i = newIndex; i > 0; --i)
		moveSpineUp(i);
	refreshSelectedResourceLists();
	restartAnimation();
	resetScale();
	return true;
}

size_t CSpinePlayerC::getNumberOfSpines() const noexcept
{
	return m_drawables.size();
}

size_t CSpinePlayerC::getSelectedSpineIndex() const noexcept
{
	return m_selectedDrawableIndex;
}

bool CSpinePlayerC::setSelectedSpineIndex(size_t index) noexcept
{
	if (index >= m_drawables.size()) return false;
	m_selectedDrawableIndex = index;
	try
	{
		refreshSelectedResourceLists();
	}
	catch (...)
	{
		return false;
	}
	if (const SDrawableState* state = getSelectedDrawableState())
	{
		m_fOffset = state->offset;
		m_fSkeletonScale = state->skeletonScale;
		m_bFlipX = state->flipX;
		m_iRotationSteps = state->rotationSteps;
	}
	syncSelectionIndices();
	return true;
}

bool CSpinePlayerC::isSpineVisible(size_t index) const noexcept
{
	return index < m_drawableStates.size() ? m_drawableStates[index].visible : false;
}

bool CSpinePlayerC::setSpineVisible(size_t index, bool visible) noexcept
{
	if (index >= m_drawableStates.size()) return false;
	m_drawableStates[index].visible = visible;
	return true;
}

bool CSpinePlayerC::moveSpineUp(size_t index) noexcept
{
	if (index == 0 || index >= m_drawables.size()) return false;
	const size_t target = index - 1;
	std::swap(m_drawables[index], m_drawables[target]);
	std::swap(m_drawableStates[index], m_drawableStates[target]);
	std::swap(m_drawableAnimationNames[index], m_drawableAnimationNames[target]);
	std::swap(m_drawableSkinNames[index], m_drawableSkinNames[target]);
	std::swap(m_drawableSlotNames[index], m_drawableSlotNames[target]);
	std::swap(m_atlases[index], m_atlases[target]);
	std::swap(m_skeletonData[index], m_skeletonData[target]);
	if (m_selectedDrawableIndex == index)
		m_selectedDrawableIndex = target;
	else if (m_selectedDrawableIndex == target)
		m_selectedDrawableIndex = index;
	refreshSelectedResourceLists();
	if (const SDrawableState* state = getSelectedDrawableState())
	{
		m_fOffset = state->offset;
		m_fSkeletonScale = state->skeletonScale;
		m_bFlipX = state->flipX;
		m_iRotationSteps = state->rotationSteps;
	}
	syncSelectionIndices();
	return true;
}

bool CSpinePlayerC::moveSpineDown(size_t index) noexcept
{
	if (index + 1 >= m_drawables.size()) return false;
	return moveSpineUp(index + 1);
}

bool CSpinePlayerC::hasSpineBeenLoaded() const noexcept
{
	return !m_drawables.empty();
}

void CSpinePlayerC::update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->update(fDelta * m_fTimeScale);
	}
}

void CSpinePlayerC::resetScale()
{
	m_fTimeScale = 1.0f;
	m_fSkeletonScale = m_fDefaultScale;
	m_fCanvasScale = m_fDefaultScale;
	m_fOffset = m_fDefaultOffset;
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->skeletonScale = m_fSkeletonScale;
		state->offset = m_fOffset;
	}

	updatePosition();
}
void CSpinePlayerC::addOffset(int iX, int iY)
{
	SDrawableState* state = getSelectedDrawableState();
	const int rotationSteps = state ? state->rotationSteps : m_iRotationSteps;
	const bool flipX = state ? state->flipX : m_bFlipX;
	const float skeletonScale = state ? state->skeletonScale : m_fSkeletonScale;
	FPoint2& offset = state ? state->offset : m_fOffset;

	float dx, dy;
	switch (rotationSteps)
	{
	case 1:  dx =  iY; dy = -iX; break;
	case 2:  dx = -iX; dy = -iY; break;
	case 3:  dx = -iY; dy =  iX; break;
	default: dx =  iX; dy =  iY; break;
	}
	if (flipX) dx = -dx;
	offset.x += dx / skeletonScale;
	offset.y += dy / skeletonScale;
	if (state)
		m_fOffset = state->offset;
	updatePosition();
}
void CSpinePlayerC::shiftAnimation()
{
	syncSelectionIndices();
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	clearAnimationTracks();
	restartAnimation();
}
void CSpinePlayerC::shiftAnimationBack()
{
	if (m_animationNames.empty()) return;
	if (m_nAnimationIndex == 0)
		m_nAnimationIndex = m_animationNames.size() - 1;
	else
		--m_nAnimationIndex;

	clearAnimationTracks();
	restartAnimation();
}
void CSpinePlayerC::shiftSkin()
{
	if (m_skinNames.empty())return;

	syncSelectionIndices();
	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	setupSkin();
}

void CSpinePlayerC::setAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		clearAnimationTracks();
		restartAnimation();
	}
}
void CSpinePlayerC::setAnimationByName(const char* szAnimationName)
{
	if (szAnimationName != nullptr)
	{
		const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
		if (iter != m_animationNames.cend())
		{
			m_nAnimationIndex = std::distance(m_animationNames.begin(), iter);
			clearAnimationTracks();
			restartAnimation();
		}
	}
}
void CSpinePlayerC::restartAnimation(bool loop)
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* szAnimationName = m_animationNames[m_nAnimationIndex].c_str();
	CSpineDrawableC* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr)
	{
		spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton()->data, szAnimationName);
		if (pAnimation != nullptr)
		{
			spAnimationState_setAnimationByName(pDrawable->animationState(), 0, pAnimation->name, loop ? -1 : 0);
			pDrawable->update(0.f);
		}
	}
}

void CSpinePlayerC::setSkinByIndex(size_t nIndex)
{
	if (nIndex < m_skinNames.size())
	{
		m_nSkinIndex = nIndex;
		setupSkin();
	}
}

void CSpinePlayerC::setSkinByName(const char* szSkinName)
{
	if (szSkinName != nullptr)
	{
		const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
		if (iter != m_skinNames.cend())
		{
			m_nSkinIndex = std::distance(m_skinNames.begin(), iter);
			setupSkin();
		}
	}
}

void CSpinePlayerC::setupSkin()
{
	if (m_nSkinIndex >= m_skinNames.size())return;
	const char* szSkinName = m_skinNames[m_nSkinIndex].c_str();
	CSpineDrawableC* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr)
	{
		spSkin* pSkin = spSkeletonData_findSkin(pDrawable->skeleton()->data, szSkinName);
		if (pSkin != nullptr)
		{
			spSkeleton_setSkin(pDrawable->skeleton(), pSkin);
			spSkeleton_setToSetupPose(pDrawable->skeleton());

			if (pDrawable->animationState() != nullptr)
			{
				spAnimationState_apply(pDrawable->animationState(), pDrawable->skeleton());
#if defined(SPINE_42)
				spSkeleton_updateWorldTransform(pDrawable->skeleton(), spPhysics::SP_PHYSICS_UPDATE);
#else
				spSkeleton_updateWorldTransform(pDrawable->skeleton());
#endif
			}
		}
	}
}
void CSpinePlayerC::togglePma()
{
	if (m_selectedDrawableIndex < m_drawables.size())
		m_drawables[m_selectedDrawableIndex]->premultiplyAlpha(!m_drawables[m_selectedDrawableIndex]->isAlphaPremultiplied());
}
void CSpinePlayerC::toggleBlendModeAdoption()
{
	if (m_selectedDrawableIndex < m_drawables.size())
		m_drawables[m_selectedDrawableIndex]->forceBlendModeNormal(!m_drawables[m_selectedDrawableIndex]->isBlendModeNormalForced());
}
bool CSpinePlayerC::isAlphaPremultiplied(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isAlphaPremultiplied();
	}

	return false;
}
bool CSpinePlayerC::isBlendModeNormalForced(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isBlendModeNormalForced();
	}

	return false;
}

bool CSpinePlayerC::isDrawOrderReversed() const noexcept
{
	return m_isDrawOrderReversed;
}

bool CSpinePlayerC::premultiplyAlpha(bool premultiplied, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->premultiplyAlpha(premultiplied);
		return true;
	}

	return false;
}

bool CSpinePlayerC::forceBlendModeNormal(bool toForce, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->forceBlendModeNormal(toForce);
		return true;
	}

	return false;
}

void CSpinePlayerC::setDrawOrder(bool reversed)
{
	m_isDrawOrderReversed = reversed;
}

std::string CSpinePlayerC::getCurrentAnimationName()
{
	CSpineDrawableC* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr)
	{
		for (size_t i = 0; i < pDrawable->animationState()->tracksCount; ++i)
		{
			spTrackEntry* pTrackEntry = pDrawable->animationState()->tracks[i];
			if (pTrackEntry != nullptr)
			{
				spAnimation* pAnimation = pTrackEntry->animation;
				if (pAnimation != nullptr && pAnimation->name != nullptr)
				{
					return pAnimation->name;
				}
			}
		}
	}

	return std::string();
}

std::string CSpinePlayerC::getCurrentSkinName()
{
	CSpineDrawableC* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr && pDrawable->skeleton()->skin != nullptr && pDrawable->skeleton()->skin->name != nullptr)
		return pDrawable->skeleton()->skin->name;
	return std::string();
}

std::string CSpinePlayerC::getLastError() const
{
	return m_lastError;
}

void CSpinePlayerC::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	CSpineDrawableC* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr)
	{
		for (size_t i = 0; i < pDrawable->animationState()->tracksCount; ++i)
		{
			spTrackEntry* pTrackEntry = pDrawable->animationState()->tracks[i];
			if (pTrackEntry != nullptr)
			{
				spAnimation* pAnimation = pTrackEntry->animation;
				if (pAnimation != nullptr)
				{
#ifdef SPINE_21
					if (fTrack != nullptr)*fTrack = pTrackEntry->time;
					if (fLast != nullptr)*fLast = ::fmodf(pTrackEntry->time, pTrackEntry->endTime);
					if (fStart != nullptr)*fStart = pTrackEntry->delay;
					if (fEnd != nullptr)*fEnd = pTrackEntry->endTime;
#elif defined(SPINE_34)
					if (fTrack != nullptr)*fTrack = pTrackEntry->time;
					if (fLast != nullptr)*fLast = pTrackEntry->lastTime;
					if (fStart != nullptr)*fStart = 0.f;
					if (fEnd != nullptr)*fEnd = pTrackEntry->endTime;
#else
					if (fTrack != nullptr)*fTrack = pTrackEntry->trackTime;
					if (fLast != nullptr)*fLast = pTrackEntry->animationLast;
					if (fStart != nullptr)*fStart = pTrackEntry->animationStart;
					if (fEnd != nullptr)*fEnd = pTrackEntry->animationEnd;
#endif
				}
			}
		}
	}
}

float CSpinePlayerC::getAnimationDuration(const char* animationName)
{
	if (const CSpineDrawableC* pDrawable = getSelectedDrawable())
	{
		spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton()->data, animationName);
		if (pAnimation != nullptr)
		{
			return pAnimation->duration;
		}
	}

	return 0.f;
}
const std::vector<std::string>& CSpinePlayerC::getSlotNames() const noexcept
{
	return m_slotNames;
}
const std::vector<std::string>& CSpinePlayerC::getSkinNames() const noexcept
{
	return m_skinNames;
}
const std::vector<std::string>& CSpinePlayerC::getAnimationNames() const noexcept
{
	return m_animationNames;
}

void CSpinePlayerC::setSlotsToExclude(const std::vector<std::string>& slotNames)
{
	std::vector<const char*> vBuffer;
	vBuffer.resize(slotNames.size());
	for (size_t i = 0; i < slotNames.size(); ++i)
	{
		vBuffer[i] = slotNames[i].data();
	}
	if (CSpineDrawableC* pDrawable = getSelectedDrawable())
		pDrawable->setLeaveOutList(vBuffer.data(), static_cast<int>(vBuffer.size()));
}
void CSpinePlayerC::mixSkins(const std::vector<std::string>& skinNames)
{
#if defined(SPINE_38) || defined(SPINE_40) || defined(SPINE_41) || defined(SPINE_42)
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	if (CSpineDrawableC* pDrawable = getSelectedDrawable())
	{
		spSkin* skinToSet = spSkeletonData_findSkin(pDrawable->skeleton()->data, currentSkinName.c_str());
		if (skinToSet == nullptr)return;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spSkin* skinToAdd = spSkeletonData_findSkin(pDrawable->skeleton()->data, skinName.c_str());
				if (skinToAdd != nullptr)
				{
					spSkin_addSkin(skinToSet, skinToAdd);
				}
			}
		}
		spSkeleton_setSkin(pDrawable->skeleton(), skinToSet);
		spSkeleton_setToSetupPose(pDrawable->skeleton());

		if (pDrawable->animationState() != nullptr)
		{
			spAnimationState_apply(pDrawable->animationState(), pDrawable->skeleton());
#if defined(SPINE_42)
			spSkeleton_updateWorldTransform(pDrawable->skeleton(), spPhysics::SP_PHYSICS_UPDATE);
#else
			spSkeleton_updateWorldTransform(pDrawable->skeleton());
#endif
		}
	}
#endif
}
void CSpinePlayerC::addAnimationTracks(const std::vector<std::string>& animationNames, bool loop)
{
	clearAnimationTracks();

	if (animationNames.empty())return;

	const auto& firstAnimationName = animationNames[0];
	const auto& currentIter = std::find(m_animationNames.begin(), m_animationNames.end(), firstAnimationName);
	if (currentIter != m_animationNames.cend())
	{
		m_nAnimationIndex = std::distance(m_animationNames.begin(), currentIter);
	}

	if (CSpineDrawableC* pDrawable = getSelectedDrawable())
	{
		spAnimation* pFirstAnimation = spSkeletonData_findAnimation(pDrawable->skeleton()->data, firstAnimationName.c_str());
		if (pFirstAnimation == nullptr)return;

		spAnimationState_setAnimationByName(pDrawable->animationState(), 0, pFirstAnimation->name, loop ? -1 : 0);

		int iTrack = 1;
		for (size_t i = 1; i < animationNames.size(); ++i)
		{
			const auto& animationName = animationNames[i];
			if (animationName != firstAnimationName)
			{
				spAnimation* pAnimationToAdd = spSkeletonData_findAnimation(pDrawable->skeleton()->data, animationName.c_str());
				if (pAnimationToAdd != nullptr)
				{
					spAnimationState_addAnimation(pDrawable->animationState(), iTrack, pAnimationToAdd, loop ? -1 : 0, 0.f);
					++iTrack;
				}
			}
		}
	}
}
void CSpinePlayerC::setSlotExcludeCallback(bool(*pFunc)(const char*, size_t))
{
	if (CSpineDrawableC* pDrawable = getSelectedDrawable())
		pDrawable->setLeaveOutCallback(pFunc);
}
std::unordered_map<std::string, std::vector<std::string>> CSpinePlayerC::getSlotNamesWithTheirAttachments()
{
	std::unordered_map<std::string, std::vector<std::string>> slotAttachmentMap;

	if (m_selectedDrawableIndex < m_skeletonData.size())
	{
		const auto& pSkeletonDatum = m_skeletonData[m_selectedDrawableIndex];
		spSkin* pSkin = pSkeletonDatum->defaultSkin;
		if (pSkin == nullptr)return slotAttachmentMap;

		for (int iSlotIndex = 0; iSlotIndex < pSkeletonDatum->slotsCount; ++iSlotIndex)
		{
			std::vector<std::string> attachmentNames;

			for (int iAttachmentIndex = 0;; ++iAttachmentIndex)
			{
				const char* attachmentName = spSkin_getAttachmentName(pSkeletonDatum->defaultSkin, iSlotIndex, iAttachmentIndex);
				if (attachmentName == nullptr)break;

				const auto& iter = std::find(attachmentNames.begin(), attachmentNames.end(), attachmentName);
				if (iter == attachmentNames.cend())attachmentNames.push_back(attachmentName);
			}

			if (attachmentNames.size() > 1)
			{
				slotAttachmentMap.insert({ pSkeletonDatum->slots[iSlotIndex]->name, attachmentNames });
			}
		}
	}

	return slotAttachmentMap;
}
bool CSpinePlayerC::replaceAttachment(const char* szSlotName, const char* szAttachmentName)
{
	if (szSlotName == nullptr || szAttachmentName == nullptr)return false;

	const auto FindSlot = [&szSlotName](spSkeleton* const pSkeleton)
		-> spSlot*
		{
			for (size_t i = 0; i < pSkeleton->slotsCount; ++i)
			{
				const char* slotName = pSkeleton->drawOrder[i]->data->name;
				if (slotName != nullptr && strcmp(slotName, szSlotName) == 0)
				{
					return pSkeleton->drawOrder[i];
				}
			}

			return nullptr;
		};

	const auto FindAttachment = [&szAttachmentName](spSkeletonData* const pSkeletonDatum)
		-> spAttachment*
		{
			spSkin* pSkin = pSkeletonDatum->defaultSkin;
			if (pSkin != nullptr)
			{
				for (int iSlotIndex = 0; iSlotIndex < pSkeletonDatum->slotsCount; ++iSlotIndex)
				{
					for (int iAttachmentIndex = 0;; ++iAttachmentIndex)
					{
						const char* attachmentName = spSkin_getAttachmentName(pSkeletonDatum->defaultSkin, iSlotIndex, iAttachmentIndex);
						if (attachmentName == nullptr)break;

						if (strcmp(attachmentName, szAttachmentName) == 0)
						{
							spAttachment* pAttachment = spSkin_getAttachment(pSkeletonDatum->defaultSkin, iSlotIndex, attachmentName);
							if (pAttachment != nullptr)
							{
								return pAttachment;
							}
						}
					}
				}
			}

			return nullptr;
		};

	if (CSpineDrawableC* pDrawable = getSelectedDrawable())
	{
		spSlot* pSlot = FindSlot(pDrawable->skeleton());
		if (pSlot == nullptr)return false;

		spAttachment* pAttachment = FindAttachment(pDrawable->skeleton()->data);
		if (pAttachment == nullptr)return false;

		if (pSlot->attachment != nullptr)
		{
			const char* animationName = m_animationNames[m_nAnimationIndex].c_str();
			spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton()->data, animationName);
			if (pAnimation == nullptr)return false;

#if !defined(SPINE_41) && !defined(SPINE_42)
			for (size_t i = 0; i < pAnimation->timelinesCount; ++i)
			{
				if (pAnimation->timelines[i]->type == SP_TIMELINE_ATTACHMENT)
				{
					spAttachmentTimeline* pAttachmentTimeline = (spAttachmentTimeline*)pAnimation->timelines[i];
					for (size_t ii = 0; ii < pAttachmentTimeline->framesCount; ++ii)
					{
						const char* szName = pAttachmentTimeline->attachmentNames[ii];
						if (szName == nullptr)continue;

						if (strcmp(szName, pSlot->attachment->name) == 0)
						{
							FREE(pAttachmentTimeline->attachmentNames[ii]);
							MALLOC_STR(pAttachmentTimeline->attachmentNames[ii], szAttachmentName);
						}
					}
				}
			}
#else
			for (size_t i = 0; i < pAnimation->timelines->size; ++i)
			{
				if (pAnimation->timelines->items[i]->type == SP_TIMELINE_ATTACHMENT)
				{
					spAttachmentTimeline* pAttachmentTimeline = (spAttachmentTimeline*)pAnimation->timelines->items[i];
					for (size_t ii = 0; ii < pAnimation->timelines->items[i]->frameCount; ++ii)
					{
						const char* szName = pAttachmentTimeline->attachmentNames[ii];
						if (szName == nullptr)continue;

						if (strcmp(szName, pSlot->attachment->name) == 0)
						{
							FREE(pAttachmentTimeline->attachmentNames[ii]);
							MALLOC_STR(pAttachmentTimeline->attachmentNames[ii], szAttachmentName);
						}
					}
				}
			}
#endif
		}

		spSlot_setAttachment(pSlot, pAttachment);
	}

	return true;
}

FPoint2 CSpinePlayerC::getBaseSize() const noexcept
{
	return m_fBaseSize;
}

void CSpinePlayerC::setBaseSize(float fWidth, float fHeight)
{
	m_fBaseSize = { fWidth, fHeight };
	workOutDefaultScale();
	m_fDefaultOffset = m_fOffset;

	resetScale();
}

void CSpinePlayerC::resetBaseSize()
{
	workOutDefaultSize();
	workOutDefaultScale();

	m_fOffset = {};
	updatePosition();
	for (const auto& drawable : m_drawables)
	{
#if !defined(SPINE_21) && !defined(SPINE_34)
		spAnimationState_setEmptyAnimations(drawable->animationState(), 0.f);
#endif
		drawable->update(0.f);
	}

	workOutDefaultOffset();
	resetScale();
	restartAnimation();
}

FPoint2 CSpinePlayerC::getOffset() const noexcept
{
	if (const SDrawableState* state = getSelectedDrawableState())
		return state->offset;
	return m_fOffset;
}

void CSpinePlayerC::setOffset(float fX, float fY) noexcept
{
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->offset = { fX, fY };
		m_fOffset = state->offset;
	}
	updatePosition();
}

float CSpinePlayerC::getSkeletonScale() const noexcept
{
	if (const SDrawableState* state = getSelectedDrawableState())
		return state->skeletonScale;
	return m_fSkeletonScale;
}

void CSpinePlayerC::setSkeletonScale(float fScale) noexcept
{
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->skeletonScale = fScale;
		m_fSkeletonScale = state->skeletonScale;
	}
}

float CSpinePlayerC::getSkeletonScaleAt(size_t index) const noexcept
{
	if (index < m_drawableStates.size())
		return m_drawableStates[index].skeletonScale;
	return m_fSkeletonScale;
}

bool CSpinePlayerC::setSkeletonScaleAt(size_t index, float fScale) noexcept
{
	if (index >= m_drawableStates.size()) return false;
	m_drawableStates[index].skeletonScale = fScale;
	if (index == m_selectedDrawableIndex)
		m_fSkeletonScale = m_drawableStates[index].skeletonScale;
	return true;
}

float CSpinePlayerC::getCanvasScale() const noexcept
{
	return m_fCanvasScale;
}
void CSpinePlayerC::setCanvasScale(float fScale) noexcept
{
	m_fCanvasScale = fScale;
}

float CSpinePlayerC::getTimeScale() const noexcept
{
	return m_fTimeScale;
}

void CSpinePlayerC::setTimeScale(float fTimeScale) noexcept
{
	m_fTimeScale = fTimeScale;
}

void CSpinePlayerC::setDefaultMix(float mixTime)
{
	if (CSpineDrawableC* pDrawable = getSelectedDrawable())
		pDrawable->animationState()->data->defaultMix = mixTime;
}

bool CSpinePlayerC::isFlipX() const noexcept
{
	if (const SDrawableState* state = getSelectedDrawableState())
		return state->flipX;
	return m_bFlipX;
}

void CSpinePlayerC::toggleFlipX() noexcept
{
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->flipX = !state->flipX;
		m_bFlipX = state->flipX;
	}
}

int CSpinePlayerC::getRotationSteps() const noexcept
{
	if (const SDrawableState* state = getSelectedDrawableState())
		return state->rotationSteps;
	return m_iRotationSteps;
}

void CSpinePlayerC::rotate90() noexcept
{
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->rotationSteps = (state->rotationSteps + 1) % 4;
		m_iRotationSteps = state->rotationSteps;
	}
}

bool CSpinePlayerC::isResetOffsetOnLoad() const noexcept
{
	return m_bResetOffsetOnLoad;
}

void CSpinePlayerC::setResetOffsetOnLoad(bool bReset) noexcept
{
	m_bResetOffsetOnLoad = bReset;
}

void CSpinePlayerC::clearDrawables()
{
	m_drawables.clear();
	m_drawableStates.clear();
	m_drawableAnimationNames.clear();
	m_drawableSkinNames.clear();
	m_drawableSlotNames.clear();
	m_selectedDrawableIndex = 0;
	m_atlases.clear();
	m_skeletonData.clear();

	m_animationNames.clear();
	m_nAnimationIndex = 0;

	m_skinNames.clear();
	m_nSkinIndex = 0;

	m_slotNames.clear();

	m_isDrawOrderReversed = false;
}
bool CSpinePlayerC::addDrawable(spSkeletonData* pSkeletonData)
{
	if (pSkeletonData == nullptr)return false;

	auto pDrawable = std::make_unique<CSpineDrawableC>(pSkeletonData);
	if (pDrawable.get() == nullptr)return false;

	pDrawable->skeleton()->x = m_fBaseSize.x / 2;
	pDrawable->skeleton()->y = m_fBaseSize.y / 2;
	pDrawable->update(0.f);

	m_drawables.push_back(std::move(pDrawable));
	m_drawableStates.push_back({});

	return true;
}

bool CSpinePlayerC::setupDrawables()
{
	workOutDefaultSize();
	workOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		bool bRet = addDrawable(pSkeletonDatum.get());
		if (!bRet)continue;

		std::vector<std::string> animationNames;
		for (size_t i = 0; i < pSkeletonDatum->animationsCount; ++i)
		{
			const char* szAnimationName = pSkeletonDatum->animations[i]->name;
			if (szAnimationName == nullptr)continue;

			const auto& iter = std::find(animationNames.begin(), animationNames.end(), szAnimationName);
			if (iter == animationNames.cend())animationNames.push_back(szAnimationName);
		}
		m_drawableAnimationNames.push_back(std::move(animationNames));

		std::vector<std::string> skinNames;
		for (size_t i = 0; i < pSkeletonDatum->skinsCount; ++i)
		{
			const char* szSkinName = pSkeletonDatum->skins[i]->name;
			if (szSkinName == nullptr)continue;

			const auto& iter = std::find(skinNames.begin(), skinNames.end(), szSkinName);
			if (iter == skinNames.cend())skinNames.push_back(szSkinName);
		}
		m_drawableSkinNames.push_back(std::move(skinNames));

		std::vector<std::string> slotNames;
		for (size_t i = 0; i < pSkeletonDatum->slotsCount; ++i)
		{
			const char* szSlotName = pSkeletonDatum->slots[i]->name;
			if (szSlotName == nullptr)continue;

			const auto& iter = std::find(slotNames.begin(), slotNames.end(), szSlotName);
			if (iter == slotNames.cend())slotNames.push_back(szSlotName);
		}
		m_drawableSlotNames.push_back(std::move(slotNames));
	}

	workOutDefaultOffset();
	for (auto& state : m_drawableStates)
	{
		state.offset = m_fDefaultOffset;
		state.skeletonScale = m_fDefaultScale;
		state.flipX = false;
		state.rotationSteps = 0;
	}
	m_selectedDrawableIndex = 0;
	refreshSelectedResourceLists();
	restartAnimation();
	resetScale();

	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->update(0.f);
	}

	return !m_animationNames.empty();
}
void CSpinePlayerC::workOutDefaultSize()
{
	if (m_skeletonData.empty())return;

	float fMaxSize = 0.f;
	const auto CompareDimention = [this, &fMaxSize](float fWidth, float fHeight)
		-> bool
		{
			if (fWidth > 0.f && fHeight > 0.f && fWidth * fHeight > fMaxSize)
			{
				m_fBaseSize.x = fWidth;
				m_fBaseSize.y = fHeight;
				fMaxSize = fWidth * fHeight;
				return true;
			}

			return false;
		};

	for (const auto& pSkeletonData : m_skeletonData)
	{
		if (pSkeletonData->defaultSkin == nullptr)continue;

		const char* attachmentName = spSkin_getAttachmentName(pSkeletonData->defaultSkin, 0, 0);
		if (attachmentName == nullptr)continue;

		spAttachment* pAttachment = spSkin_getAttachment(pSkeletonData->defaultSkin, 0, attachmentName);
		if (pAttachment == nullptr)continue;

		if (pAttachment->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment* pRegionAttachment = (spRegionAttachment*)pAttachment;

			CompareDimention(pRegionAttachment->width * pRegionAttachment->scaleX, pRegionAttachment->height * pRegionAttachment->scaleY);
		}
		else if (pAttachment->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMeshAttachment = (spMeshAttachment*)pAttachment;

			spSlotData* pSlotData = spSkeletonData_findSlot(pSkeletonData.get(), attachmentName);

			float fScaleX = pSlotData != nullptr ? pSlotData->boneData->scaleX : 1.f;
			float fScaleY = pSlotData != nullptr ? pSlotData->boneData->scaleY : 1.f;

			CompareDimention(pMeshAttachment->width * fScaleX, pMeshAttachment->height * fScaleY);
		}
	}

	for (const auto& pSkeletonData : m_skeletonData)
	{
		CompareDimention(pSkeletonData->width, pSkeletonData->height);
	}
}
void CSpinePlayerC::workOutDefaultScale()
{
	m_fDefaultScale = 1.f;
	m_fDefaultOffset = {};

	int iSkeletonWidth = static_cast<int>(m_fBaseSize.x);
	int iSkeletonHeight = static_cast<int>(m_fBaseSize.y);

	int iDisplayWidth = 0;
	int iDisplayHeight = 0;
#if defined _WIN32
	DxLib::GetDisplayMaxResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __ANDROID__
	DxLib::GetAndroidDisplayResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __APPLE__
	DxLib::GetDisplayResolution_iOS(&iDisplayWidth, &iDisplayHeight);
#endif
	if (iDisplayWidth == 0 || iDisplayHeight == 0)return;

	if (iSkeletonWidth > iDisplayWidth || iSkeletonHeight > iDisplayHeight)
	{
		float fScaleX = static_cast<float>(iDisplayWidth) / iSkeletonWidth;
		float fScaleY = static_cast<float>(iDisplayHeight) / iSkeletonHeight;

		if (fScaleX > fScaleY)
		{
			m_fDefaultScale = fScaleY;
		}
		else
		{
			m_fDefaultScale = fScaleX;
		}
	}
}
void CSpinePlayerC::updatePosition()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
		applyDrawablePosition(i);
}
void CSpinePlayerC::clearAnimationTracks()
{
	if (CSpineDrawableC* pDrawable = getSelectedDrawable())
	{
		for (int iTrack = 1; iTrack < pDrawable->animationState()->tracksCount; ++iTrack)
		{
#if defined(SPINE_21) || defined(SPINE_34)
			spAnimationState_clearTrack(pDrawable->animationState(), iTrack);
#else
			spAnimationState_setEmptyAnimation(pDrawable->animationState(), iTrack, 0.f);
#endif
		}
	}
}

CSpinePlayerC::SDrawableState* CSpinePlayerC::getSelectedDrawableState() noexcept
{
	if (m_selectedDrawableIndex < m_drawableStates.size())
		return &m_drawableStates[m_selectedDrawableIndex];
	return nullptr;
}

const CSpinePlayerC::SDrawableState* CSpinePlayerC::getSelectedDrawableState() const noexcept
{
	if (m_selectedDrawableIndex < m_drawableStates.size())
		return &m_drawableStates[m_selectedDrawableIndex];
	return nullptr;
}

CSpineDrawableC* CSpinePlayerC::getSelectedDrawable() const noexcept
{
	if (m_selectedDrawableIndex < m_drawables.size())
		return m_drawables[m_selectedDrawableIndex].get();
	return nullptr;
}

void CSpinePlayerC::syncSelectionIndices()
{
	const std::string currentAnimation = getCurrentAnimationName();
	auto animIt = std::find(m_animationNames.begin(), m_animationNames.end(), currentAnimation);
	if (animIt != m_animationNames.end())
		m_nAnimationIndex = static_cast<size_t>(std::distance(m_animationNames.begin(), animIt));
	else
		m_nAnimationIndex = 0;

	const std::string currentSkin = getCurrentSkinName();
	auto skinIt = std::find(m_skinNames.begin(), m_skinNames.end(), currentSkin);
	if (skinIt != m_skinNames.end())
		m_nSkinIndex = static_cast<size_t>(std::distance(m_skinNames.begin(), skinIt));
	else
		m_nSkinIndex = 0;
}

void CSpinePlayerC::refreshSelectedResourceLists()
{
	m_animationNames.clear();
	m_skinNames.clear();
	m_slotNames.clear();

	if (m_selectedDrawableIndex < m_drawableAnimationNames.size())
		m_animationNames = m_drawableAnimationNames[m_selectedDrawableIndex];
	if (m_selectedDrawableIndex < m_drawableSkinNames.size())
		m_skinNames = m_drawableSkinNames[m_selectedDrawableIndex];
	if (m_selectedDrawableIndex < m_drawableSlotNames.size())
		m_slotNames = m_drawableSlotNames[m_selectedDrawableIndex];

	if (m_nAnimationIndex >= m_animationNames.size())
		m_nAnimationIndex = 0;
	if (m_nSkinIndex >= m_skinNames.size())
		m_nSkinIndex = 0;
}

void CSpinePlayerC::applyDrawablePosition(size_t index)
{
	if (index >= m_drawables.size()) return;
	FPoint2 offset = m_fDefaultOffset;
	if (index < m_drawableStates.size())
		offset = m_drawableStates[index].offset;
	m_drawables[index]->skeleton()->x = m_fBaseSize.x / 2 - offset.x;
	m_drawables[index]->skeleton()->y = m_fBaseSize.y / 2 - offset.y;
}
