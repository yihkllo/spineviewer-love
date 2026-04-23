
#include "controller.h"
#include "loader.h"

CSpinePlayer::CSpinePlayer()
{

}

CSpinePlayer::~CSpinePlayer()
{
	for (spine::Skin* s : m_dynamicSkins) delete s;
	m_dynamicSkins.clear();
}

bool CSpinePlayer::loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
{
	m_lastError.clear();
	if (atlasPaths.size() != skelPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)
		{
			m_lastError = "Failed to create atlas from file: " + strAtlasPath;
			continue;
		}

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromFile(strSkeletonPath.c_str(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)
		{
			m_lastError = "Failed to parse skeleton: " + strSkeletonPath;
			return false;
		}

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}
bool CSpinePlayer::loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	m_lastError.clear();
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData[i];
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonData = skelData[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)
		{
			m_lastError = "Failed to create atlas from memory: " + strAtlasPath;
			continue;
		}

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(strSkeletonData.data()), static_cast<int>(strSkeletonData.size()), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromMemory(strSkeletonData.data(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)
		{
			m_lastError = "Failed to parse skeleton from memory: " + strAtlasPath;
			return false;
		}

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}

bool CSpinePlayer::addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel)
{
	if (m_drawables.empty() || szAtlasPath == nullptr || szSkelPath == nullptr)return false;

	std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(szAtlasPath, &m_textureLoader);
	if (atlas.get() == nullptr)return false;

	std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
		spine_loader::ReadBinarySkeletonFromFile(szSkelPath, atlas.get(), 1.f) :
		spine_loader::ReadTextSkeletonFromFile(szSkelPath, atlas.get(), 1.f);
	if (skeletonData.get() == nullptr)return false;

	const size_t newIndex = m_drawables.size();
	if (!addDrawable(skeletonData.get()))return false;

	m_atlases.push_back(std::move(atlas));
	m_skeletonData.push_back(std::move(skeletonData));

	const auto& addedSkeletonData = m_skeletonData.back();
	std::vector<std::string> animationNames;
	auto& animations = addedSkeletonData->getAnimations();
	for (size_t i = 0; i < animations.size(); ++i)
	{
		const char* szAnimationName = animations[i]->getName().buffer();
		if (szAnimationName == nullptr)continue;
		const auto& iter = std::find(animationNames.begin(), animationNames.end(), szAnimationName);
		if (iter == animationNames.cend())animationNames.push_back(szAnimationName);
	}
	m_drawableAnimationNames.push_back(std::move(animationNames));

	std::vector<std::string> skinNames;
	auto& skins = addedSkeletonData->getSkins();
	for (size_t i = 0; i < skins.size(); ++i)
	{
		const char* szSkinName = skins[i]->getName().buffer();
		if (szSkinName == nullptr)continue;
		const auto& iter = std::find(skinNames.begin(), skinNames.end(), szSkinName);
		if (iter == skinNames.cend())skinNames.push_back(szSkinName);
	}
	m_drawableSkinNames.push_back(std::move(skinNames));

	std::vector<std::string> slotNames;
	auto& slots = addedSkeletonData->getSlots();
	for (size_t i = 0; i < slots.size(); ++i)
	{
		const char* szSlotName = slots[i]->getName().buffer();
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

size_t CSpinePlayer::getNumberOfSpines() const noexcept
{
	return m_drawables.size();
}

size_t CSpinePlayer::getSelectedSpineIndex() const noexcept
{
	return m_selectedDrawableIndex;
}

bool CSpinePlayer::setSelectedSpineIndex(size_t index) noexcept
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

bool CSpinePlayer::isSpineVisible(size_t index) const noexcept
{
	return index < m_drawableStates.size() ? m_drawableStates[index].visible : false;
}

bool CSpinePlayer::setSpineVisible(size_t index, bool visible) noexcept
{
	if (index >= m_drawableStates.size()) return false;
	m_drawableStates[index].visible = visible;
	return true;
}

bool CSpinePlayer::moveSpineUp(size_t index) noexcept
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

bool CSpinePlayer::moveSpineDown(size_t index) noexcept
{
	if (index + 1 >= m_drawables.size()) return false;
	return moveSpineUp(index + 1);
}

bool CSpinePlayer::hasSpineBeenLoaded() const noexcept
{
	return !m_drawables.empty();
}
void CSpinePlayer::update(float fDelta)
{
	for (const auto& drawable : m_drawables)
	{
		drawable->update(fDelta * m_fTimeScale);
	}
}
void CSpinePlayer::resetScale()
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
void CSpinePlayer::addOffset(int iX, int iY)
{
	float dx, dy;
	SDrawableState* state = getSelectedDrawableState();
	const int rotationSteps = state ? state->rotationSteps : m_iRotationSteps;
	const bool flipX = state ? state->flipX : m_bFlipX;
	const float skeletonScale = state ? state->skeletonScale : m_fSkeletonScale;
	FPoint2& offset = state ? state->offset : m_fOffset;

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
	{
		m_fOffset = state->offset;
	}
	updatePosition();
}
void CSpinePlayer::shiftAnimation()
{
	syncSelectionIndices();
	++m_nAnimationIndex;
	if (m_nAnimationIndex >= m_animationNames.size())m_nAnimationIndex = 0;

	clearAnimationTracks();
	restartAnimation();
}
void CSpinePlayer::shiftAnimationBack()
{
	if (m_animationNames.empty()) return;
	if (m_nAnimationIndex == 0)
		m_nAnimationIndex = m_animationNames.size() - 1;
	else
		--m_nAnimationIndex;

	clearAnimationTracks();
	restartAnimation();
}
void CSpinePlayer::shiftSkin()
{
	if (m_skinNames.empty())return;

	syncSelectionIndices();
	++m_nSkinIndex;
	if (m_nSkinIndex >= m_skinNames.size())m_nSkinIndex = 0;

	setupSkin();
}

void CSpinePlayer::setAnimationByIndex(size_t nIndex)
{
	if (nIndex < m_animationNames.size())
	{
		m_nAnimationIndex = nIndex;
		clearAnimationTracks();
		restartAnimation();
	}
}

void CSpinePlayer::setAnimationByName(const char* szAnimationName)
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
void CSpinePlayer::restartAnimation(bool loop)
{
	if (m_nAnimationIndex >= m_animationNames.size())return;
	const char* szAnimationName = m_animationNames[m_nAnimationIndex].c_str();
	CSpineDrawable* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr)
	{
		spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(szAnimationName);
		if (pAnimation != nullptr)
		{
			pDrawable->animationState()->setAnimation(0, pAnimation->getName(), loop);
			pDrawable->update(0.f);
		}
	}
}

void CSpinePlayer::setSkinByIndex(size_t nIndex)
{
	if (nIndex < m_skinNames.size())
	{
		m_nSkinIndex = nIndex;
		setupSkin();
	}
}

void CSpinePlayer::setSkinByName(const char* szSkinName)
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

void CSpinePlayer::setupSkin()
{
	for (spine::Skin* s : m_dynamicSkins) delete s;
	m_dynamicSkins.clear();

	if (m_nSkinIndex >= m_skinNames.size())return;
	const char* szSkinName = m_skinNames[m_nSkinIndex].c_str();
	CSpineDrawable* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr)
	{
		spine::Skin* skin = pDrawable->skeleton()->getData()->findSkin(szSkinName);
		if (skin != nullptr)
		{
			pDrawable->skeleton()->setSkin(skin);
			pDrawable->skeleton()->setSlotsToSetupPose();

			if (pDrawable->animationState() != nullptr)
			{
				pDrawable->animationState()->apply(*pDrawable->skeleton());
#if defined(SPINE_42)
				pDrawable->skeleton()->updateWorldTransform(spine::Physics::Physics_Update);
#else
				pDrawable->skeleton()->updateWorldTransform();
#endif
			}
		}
	}
}
void CSpinePlayer::togglePma()
{
	if (m_selectedDrawableIndex >= m_drawables.size()) return;
	const bool bNewPma = !m_drawables[m_selectedDrawableIndex]->isAlphaPremultiplied();
	m_drawables[m_selectedDrawableIndex]->premultiplyAlpha(bNewPma);
}
void CSpinePlayer::toggleBlendModeAdoption()
{
	if (m_selectedDrawableIndex < m_drawables.size())
		m_drawables[m_selectedDrawableIndex]->forceBlendModeNormal(!m_drawables[m_selectedDrawableIndex]->isBlendModeNormalForced());
}
bool CSpinePlayer::isAlphaPremultiplied(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isAlphaPremultiplied();
	}

	return false;
}
bool CSpinePlayer::isBlendModeNormalForced(size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		return m_drawables[nDrawableIndex]->isBlendModeNormalForced();
	}

	return false;
}

bool CSpinePlayer::isDrawOrderReversed() const noexcept
{
	return m_isDrawOrderReversed;
}

bool CSpinePlayer::premultiplyAlpha(bool premultiplied, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->premultiplyAlpha(premultiplied);
		return true;
	}

	return false;
}

bool CSpinePlayer::forceBlendModeNormal(bool toForce, size_t nDrawableIndex)
{
	if (nDrawableIndex < m_drawables.size())
	{
		m_drawables[nDrawableIndex]->forceBlendModeNormal(toForce);
		return true;
	}

	return false;
}

void CSpinePlayer::setDrawOrder(bool reversed)
{
	m_isDrawOrderReversed = reversed;
}
std::string CSpinePlayer::getCurrentAnimationName()
{
	CSpineDrawable* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr)
	{
		auto& tracks = pDrawable->animationState()->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				return pAnimation->getName().buffer();
			}
		}
	}

	return std::string();
}

std::string CSpinePlayer::getCurrentSkinName()
{
	CSpineDrawable* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr && pDrawable->skeleton()->getSkin() != nullptr)
		return pDrawable->skeleton()->getSkin()->getName().buffer();
	return std::string();
}

std::string CSpinePlayer::getLastError() const
{
	return m_lastError;
}

void CSpinePlayer::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	CSpineDrawable* pDrawable = getSelectedDrawable();
	if (pDrawable != nullptr)
	{
		auto& tracks = pDrawable->animationState()->getTracks();
		for (size_t i = 0; i < tracks.size(); ++i)
		{
			spine::Animation* pAnimation = tracks[i]->getAnimation();
			if (pAnimation != nullptr)
			{
				if (fTrack != nullptr)*fTrack = tracks[i]->getTrackTime();
				if (fLast != nullptr)*fLast = tracks[i]->getAnimationLast();
				if (fStart != nullptr)*fStart = tracks[i]->getAnimationStart();
				if (fEnd != nullptr)*fEnd = tracks[i]->getAnimationEnd();

				return;
			}
		}
	}
}

float CSpinePlayer::getAnimationDuration(const char* animationName)
{
	if (const CSpineDrawable* pDrawable = getSelectedDrawable())
	{
		spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(animationName);
		if (pAnimation != nullptr)
		{
			return pAnimation->getDuration();
		}
	}

	return 0.f;
}

const std::vector<std::string>& CSpinePlayer::getSlotNames() const noexcept
{
	return m_slotNames;
}
const std::vector<std::string>& CSpinePlayer::getSkinNames() const noexcept
{
	return m_skinNames;
}
const std::vector<std::string>& CSpinePlayer::getAnimationNames() const noexcept
{
	return m_animationNames;
}
void CSpinePlayer::setSlotsToExclude(const std::vector<std::string>& slotNames)
{
	spine::Vector<spine::String> leaveOutList;
	for (const auto& slotName : slotNames)
	{
		leaveOutList.add(slotName.c_str());
	}

	if (CSpineDrawable* pDrawable = getSelectedDrawable())
		pDrawable->setLeaveOutList(leaveOutList);
}
void CSpinePlayer::mixSkins(const std::vector<std::string>& skinNames)
{
	if (CSpineDrawable* pDrawable = getSelectedDrawable())
		pDrawable->skeleton()->setSkin(nullptr);
	for (spine::Skin* s : m_dynamicSkins) delete s;
	m_dynamicSkins.clear();

	if (CSpineDrawable* pDrawable = getSelectedDrawable())
	{
		spine::Skin* dynamicSkin = new spine::Skin("__dynamic__");
		for (const auto& skinName : skinNames)
		{
			spine::Skin* s = pDrawable->skeleton()->getData()->findSkin(skinName.c_str());
			if (s != nullptr) dynamicSkin->addSkin(s);
		}
		pDrawable->skeleton()->setSkin(dynamicSkin);
		pDrawable->skeleton()->setSlotsToSetupPose();
		m_dynamicSkins.push_back(dynamicSkin);

		if (pDrawable->animationState() != nullptr)
		{
			pDrawable->animationState()->apply(*pDrawable->skeleton());
#if defined(SPINE_42)
			pDrawable->skeleton()->updateWorldTransform(spine::Physics::Physics_Update);
#else
			pDrawable->skeleton()->updateWorldTransform();
#endif
		}
	}
}
void CSpinePlayer::addAnimationTracks(const std::vector<std::string>& animationNames, bool loop)
{
	clearAnimationTracks();

	if (animationNames.empty())return;

	const auto& firstAnimationName = animationNames[0];
	const auto& currentIter = std::find(m_animationNames.begin(), m_animationNames.end(), firstAnimationName);
	if (currentIter != m_animationNames.cend())
	{
		m_nAnimationIndex = std::distance(m_animationNames.begin(), currentIter);
	}

	if (CSpineDrawable* pDrawable = getSelectedDrawable())
	{
		spine::Animation* firstAnimation = pDrawable->skeleton()->getData()->findAnimation(firstAnimationName.c_str());
		if (firstAnimation == nullptr)return;

		pDrawable->animationState()->setAnimation(0, firstAnimation->getName(), loop);

		int iTrack = 1;
		for (size_t i = 1; i < animationNames.size(); ++i)
		{
			const auto& animationName = animationNames[i];
			if (animationName != firstAnimationName)
			{
				spine::Animation* animation = pDrawable->skeleton()->getData()->findAnimation(animationName.c_str());
				if (animation != nullptr)
				{
					pDrawable->animationState()->addAnimation(iTrack, animation, loop, 0.f);
					++iTrack;
				}
			}
		}
	}
}

void CSpinePlayer::setSlotExcludeCallback(bool(*pFunc)(const char*, size_t))
{
	if (CSpineDrawable* pDrawable = getSelectedDrawable())
		pDrawable->setLeaveOutCallback(pFunc);
}

std::unordered_map<std::string, std::vector<std::string>> CSpinePlayer::getSlotNamesWithTheirAttachments()
{
	std::unordered_map<std::string, std::vector<std::string>> slotAttachmentMap;

	if (m_selectedDrawableIndex < m_skeletonData.size())
	{
		const auto& skeletonDatum = m_skeletonData[m_selectedDrawableIndex];
		spine::Skin* pSkin = skeletonDatum->getDefaultSkin();

		auto& slots = skeletonDatum->getSlots();
		for (size_t i = 0; i < slots.size(); ++i)
		{
			spine::Vector<spine::Attachment*> pAttachments;
			pSkin->findAttachmentsForSlot(i, pAttachments);
			if (pAttachments.size() > 1)
			{
				std::vector<std::string> attachmentNames;

				for (size_t ii = 0; ii < pAttachments.size(); ++ii)
				{
					const char* szName = pAttachments[ii]->getName().buffer();
					const auto& iter = std::find(attachmentNames.begin(), attachmentNames.end(), szName);
					if (iter == attachmentNames.cend())attachmentNames.push_back(szName);
				}

				slotAttachmentMap.insert({ slots[i]->getName().buffer(), attachmentNames });
			}
		}
	}

	return slotAttachmentMap;
}
bool CSpinePlayer::replaceAttachment(const char* szSlotName, const char* szAttachmentName)
{
	if (szSlotName == nullptr || szAttachmentName == nullptr)return false;

	const auto FindSlot = [&szSlotName](spine::Skeleton* const skeleton)
		-> spine::Slot*
		{
			for (size_t i = 0; i < skeleton->getSlots().size(); ++i)
			{
				const spine::String& slotName = skeleton->getDrawOrder()[i]->getData().getName();
				if (!slotName.isEmpty() && slotName == szSlotName)
				{
					return skeleton->getDrawOrder()[i];
				}
			}
			return nullptr;
		};

	const auto FindAttachment = [&szAttachmentName](spine::SkeletonData* const skeletonDatum)
		->spine::Attachment*
		{
			spine::Skin::AttachmentMap::Entries attachmentMapEntries = skeletonDatum->getDefaultSkin()->getAttachments();
			for (; attachmentMapEntries.hasNext();)
			{
				spine::Skin::AttachmentMap::Entry attachmentMapEntry = attachmentMapEntries.next();

				if (attachmentMapEntry._name == szAttachmentName)
				{
					return attachmentMapEntry._attachment;
				}
			}
			return nullptr;
		};

	if (CSpineDrawable* pDrawable = getSelectedDrawable())
	{
		spine::Slot* pSlot = FindSlot(pDrawable->skeleton());
		if (pSlot == nullptr)return false;

		spine::Attachment* pAttachment = FindAttachment(pDrawable->skeleton()->getData());
		if (pAttachment == nullptr)return false;

		if (pSlot->getAttachment() != nullptr)
		{
			const char* animationName = m_animationNames[m_nAnimationIndex].c_str();
			spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(animationName);
			if (pAnimation == nullptr)return false;

			spine::Vector<spine::Timeline*>& timelines = pAnimation->getTimelines();
			for (size_t i = 0; i < timelines.size(); ++i)
			{
				if (timelines[i]->getRTTI().isExactly(spine::AttachmentTimeline::rtti))
				{
					const auto& attachmentTimeline = static_cast<spine::AttachmentTimeline*>(timelines[i]);

					spine::Vector<spine::String>& attachmentNames = attachmentTimeline->getAttachmentNames();
					for (size_t ii = 0; ii < attachmentNames.size(); ++ii)
					{
						const char* szName = attachmentNames[ii].buffer();
						if (szName == nullptr)continue;

						if (strcmp(szName, pSlot->getAttachment()->getName().buffer()) == 0)
						{
							attachmentNames[ii] = szAttachmentName;
						}
					}
				}
			}
		}

		pSlot->setAttachment(pAttachment);
	}

	return true;
}
FPoint2 CSpinePlayer::getBaseSize() const noexcept
{
	return m_fBaseSize;
}

void CSpinePlayer::setBaseSize(float fWidth, float fHeight)
{
	m_fBaseSize = { fWidth, fHeight };
	workOutDefaultScale();
	m_fDefaultOffset = m_fOffset;

	resetScale();
}

void CSpinePlayer::resetBaseSize()
{
	workOutDefaultSize();
	workOutDefaultScale();

	m_fOffset = {};
	updatePosition();
	for (const auto& drawable : m_drawables)
	{
		drawable->animationState()->setEmptyAnimations(0.f);
		drawable->update(0.f);
	}

	workOutDefaultOffset();
	resetScale();
	restartAnimation();
}

FPoint2 CSpinePlayer::getOffset() const noexcept
{
	if (const SDrawableState* state = getSelectedDrawableState())
		return state->offset;
	return m_fOffset;
}

void CSpinePlayer::setOffset(float fX, float fY) noexcept
{
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->offset = { fX, fY };
		m_fOffset = state->offset;
	}
	updatePosition();
}

float CSpinePlayer::getSkeletonScale() const noexcept
{
	if (const SDrawableState* state = getSelectedDrawableState())
		return state->skeletonScale;
	return m_fSkeletonScale;
}

void CSpinePlayer::setSkeletonScale(float fScale)
{
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->skeletonScale = fScale;
		m_fSkeletonScale = state->skeletonScale;
	}
}

float CSpinePlayer::getSkeletonScaleAt(size_t index) const noexcept
{
	if (index < m_drawableStates.size())
		return m_drawableStates[index].skeletonScale;
	return m_fSkeletonScale;
}

bool CSpinePlayer::setSkeletonScaleAt(size_t index, float fScale) noexcept
{
	if (index >= m_drawableStates.size()) return false;
	m_drawableStates[index].skeletonScale = fScale;
	if (index == m_selectedDrawableIndex)
		m_fSkeletonScale = m_drawableStates[index].skeletonScale;
	return true;
}

float CSpinePlayer::getCanvasScale() const noexcept
{
	return m_fCanvasScale;
}
void CSpinePlayer::setCanvasScale(float fScale) noexcept
{
	m_fCanvasScale = fScale;
}

float CSpinePlayer::getTimeScale() const noexcept
{
	return m_fTimeScale;
}

void CSpinePlayer::setTimeScale(float fTimeScale) noexcept
{
	m_fTimeScale = fTimeScale;
}

void CSpinePlayer::setDefaultMix(float mixTime)
{
	if (CSpineDrawable* pDrawable = getSelectedDrawable())
		pDrawable->animationState()->getData()->setDefaultMix(mixTime);
}

bool CSpinePlayer::isFlipX() const noexcept
{
	if (const SDrawableState* state = getSelectedDrawableState())
		return state->flipX;
	return m_bFlipX;
}

void CSpinePlayer::toggleFlipX() noexcept
{
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->flipX = !state->flipX;
		m_bFlipX = state->flipX;
	}
}

int CSpinePlayer::getRotationSteps() const noexcept
{
	if (const SDrawableState* state = getSelectedDrawableState())
		return state->rotationSteps;
	return m_iRotationSteps;
}

void CSpinePlayer::rotate90() noexcept
{
	if (SDrawableState* state = getSelectedDrawableState())
	{
		state->rotationSteps = (state->rotationSteps + 1) % 4;
		m_iRotationSteps = state->rotationSteps;
	}
}

bool CSpinePlayer::isResetOffsetOnLoad() const noexcept
{
	return m_bResetOffsetOnLoad;
}

void CSpinePlayer::setResetOffsetOnLoad(bool bReset) noexcept
{
	m_bResetOffsetOnLoad = bReset;
}

void CSpinePlayer::clearDrawables()
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

bool CSpinePlayer::addDrawable(spine::SkeletonData* pSkeletonData)
{
	if (pSkeletonData == nullptr)return false;
	auto pDrawable = std::make_unique<CSpineDrawable>(pSkeletonData);
	if (pDrawable.get() == nullptr)return false;

	pDrawable->skeleton()->setPosition(m_fBaseSize.x / 2, m_fBaseSize.y / 2);
	pDrawable->update(0.f);

	m_drawables.push_back(std::move(pDrawable));
	m_drawableStates.push_back({});

	return true;
}

bool CSpinePlayer::setupDrawables()
{
	workOutDefaultSize();
	workOutDefaultScale();

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		bool bRet = addDrawable(pSkeletonDatum.get());
		if (!bRet)continue;

		std::vector<std::string> animationNames;
		auto& animations = pSkeletonDatum->getAnimations();
		for (size_t i = 0; i < animations.size(); ++i)
		{
			const char* szAnimationName = animations[i]->getName().buffer();
			if (szAnimationName == nullptr)continue;

			const auto& iter = std::find(animationNames.begin(), animationNames.end(), szAnimationName);
			if (iter == animationNames.cend())animationNames.push_back(szAnimationName);
		}
		m_drawableAnimationNames.push_back(std::move(animationNames));

		std::vector<std::string> skinNames;
		auto& skins = pSkeletonDatum->getSkins();
		for (size_t i = 0; i < skins.size(); ++i)
		{
			const char* szSkinName = skins[i]->getName().buffer();
			if (szSkinName == nullptr)continue;

			const auto& iter = std::find(skinNames.begin(), skinNames.end(), szSkinName);
			if (iter == skinNames.cend())skinNames.push_back(szSkinName);
		}
		m_drawableSkinNames.push_back(std::move(skinNames));

		std::vector<std::string> slotNames;
		auto& slots = pSkeletonDatum->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const char* szName = slots[ii]->getName().buffer();
			if (szName == nullptr)continue;
			const auto& iter = std::find(slotNames.begin(), slotNames.end(), szName);
			if (iter == slotNames.cend())slotNames.push_back(szName);
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
void CSpinePlayer::workOutDefaultSize()
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
		if (pSkeletonData.get()->getWidth() > 0 && pSkeletonData.get()->getHeight())
		{
			CompareDimention(pSkeletonData.get()->getWidth(), pSkeletonData.get()->getHeight());
		}
		else
		{
			const auto FindDefaultSkinAttachment = [&pSkeletonData]()
				-> spine::Attachment*
				{
					if (pSkeletonData.get()->getDefaultSkin() == nullptr)
					{
						return nullptr;
					}
					spine::Skin::AttachmentMap::Entries attachmentMapEntries = pSkeletonData.get()->getDefaultSkin()->getAttachments();
					for (; attachmentMapEntries.hasNext();)
					{
						spine::Skin::AttachmentMap::Entry attachmentMapEntry = attachmentMapEntries.next();
						if (attachmentMapEntry._slotIndex == 0)
						{
							return attachmentMapEntry._attachment;
						}
					}
					return nullptr;
				};

			spine::Attachment* pAttachment = FindDefaultSkinAttachment();
			if (pAttachment == nullptr)continue;

			if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
			{
				spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;

				CompareDimention(pRegionAttachment->getWidth() * pRegionAttachment->getScaleX(), pRegionAttachment->getHeight() * pRegionAttachment->getScaleY());
			}
			else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
			{
				spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;

				float fScale =
					::isgreater(pMeshAttachment->getWidth(), Constants::kMinAtlas) &&
					::isgreater(pMeshAttachment->getHeight(), Constants::kMinAtlas) ? 1.f : 2.f;

				CompareDimention(pMeshAttachment->getWidth() * fScale, pMeshAttachment->getHeight() * fScale);
			}
		}
	}
}

void CSpinePlayer::updatePosition()
{
	for (size_t i = 0; i < m_drawables.size(); ++i)
		applyDrawablePosition(i);
}
void CSpinePlayer::clearAnimationTracks()
{
	if (CSpineDrawable* pDrawable = getSelectedDrawable())
	{
		const auto& trackEntry = pDrawable->animationState()->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			pDrawable->animationState()->setEmptyAnimation(iTrack, 0.f);
		}
	}
}

CSpinePlayer::SDrawableState* CSpinePlayer::getSelectedDrawableState() noexcept
{
	if (m_selectedDrawableIndex < m_drawableStates.size())
		return &m_drawableStates[m_selectedDrawableIndex];
	return nullptr;
}

const CSpinePlayer::SDrawableState* CSpinePlayer::getSelectedDrawableState() const noexcept
{
	if (m_selectedDrawableIndex < m_drawableStates.size())
		return &m_drawableStates[m_selectedDrawableIndex];
	return nullptr;
}

CSpineDrawable* CSpinePlayer::getSelectedDrawable() const noexcept
{
	if (m_selectedDrawableIndex < m_drawables.size())
		return m_drawables[m_selectedDrawableIndex].get();
	return nullptr;
}

void CSpinePlayer::syncSelectionIndices()
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

void CSpinePlayer::refreshSelectedResourceLists()
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

void CSpinePlayer::applyDrawablePosition(size_t index)
{
	if (index >= m_drawables.size()) return;
	FPoint2 offset = m_fDefaultOffset;
	if (index < m_drawableStates.size())
		offset = m_drawableStates[index].offset;
	m_drawables[index]->skeleton()->setPosition(m_fBaseSize.x / 2 - offset.x, m_fBaseSize.y / 2 - offset.y);
}
