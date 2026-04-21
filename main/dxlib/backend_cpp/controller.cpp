#include <Windows.h>
#include <string>
#include <vector>
#include "controller.h"
#include "loader.h"

namespace
{
	std::wstring WidenUtf8Path(const std::string& path)
	{
		if (path.empty()) return {};
		int len = ::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), static_cast<int>(path.size()), nullptr, 0);
		if (len <= 0) return {};
		std::wstring wide(static_cast<size_t>(len), L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, path.c_str(), static_cast<int>(path.size()), &wide[0], len);
		return wide;
	}

	std::string ReadFileAsUtf8PathBytes(const std::string& path)
	{
		std::wstring widePath = WidenUtf8Path(path);
		if (widePath.empty()) return {};

		HANDLE hFile = ::CreateFileW(widePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE) return {};

		LARGE_INTEGER size{};
		if (!::GetFileSizeEx(hFile, &size) || size.QuadPart <= 0)
		{
			::CloseHandle(hFile);
			return {};
		}

		std::string bytes(static_cast<size_t>(size.QuadPart), '\0');
		DWORD read = 0;
		BOOL ok = ::ReadFile(hFile, bytes.empty() ? nullptr : &bytes[0], static_cast<DWORD>(size.QuadPart), &read, nullptr);
		::CloseHandle(hFile);
		if (!ok || read == 0) return {};
		bytes.resize(read);
		return bytes;
	}

	void StripUtf8Bom(std::string& bytes)
	{
		if (bytes.size() >= 3 &&
			static_cast<unsigned char>(bytes[0]) == 0xEF &&
			static_cast<unsigned char>(bytes[1]) == 0xBB &&
			static_cast<unsigned char>(bytes[2]) == 0xBF)
		{
			bytes.erase(0, 3);
		}
	}

	std::string GetParentDirUtf8(const std::string& path)
	{
		std::wstring widePath = WidenUtf8Path(path);
		if (widePath.empty()) return {};

		size_t pos = widePath.find_last_of(L"\\/");
		if (pos == std::wstring::npos) return {};

		std::wstring wideDir = widePath.substr(0, pos + 1);
		int len = ::WideCharToMultiByte(CP_UTF8, 0, wideDir.c_str(), static_cast<int>(wideDir.size()), nullptr, 0, nullptr, nullptr);
		if (len <= 0) return {};
		std::string utf8Dir(static_cast<size_t>(len), '\0');
		::WideCharToMultiByte(CP_UTF8, 0, wideDir.c_str(), static_cast<int>(wideDir.size()), &utf8Dir[0], len, nullptr, nullptr);
		return utf8Dir;
	}
}

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
	if (atlasPaths.size() != skelPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasPaths.size(); ++i)
	{
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonPath = skelPaths[i];
		std::string atlasBytes = ReadFileAsUtf8PathBytes(strAtlasPath);
		if (atlasBytes.empty()) continue;
		StripUtf8Bom(atlasBytes);
		std::string atlasDir = GetParentDirUtf8(strAtlasPath);

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(atlasBytes.c_str(), static_cast<int>(atlasBytes.size()), atlasDir.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::string skeletonBytes = ReadFileAsUtf8PathBytes(strSkeletonPath);
		if (skeletonBytes.empty()) continue;
		if (!isBinarySkel) StripUtf8Bom(skeletonBytes);

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(skeletonBytes.data()), static_cast<int>(skeletonBytes.size()), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromMemory(skeletonBytes.c_str(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}

bool CSpinePlayer::loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData[i];
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonData = skelData[i];

		std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), &m_textureLoader);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
			spine_loader::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(strSkeletonData.data()), static_cast<int>(strSkeletonData.size()), atlas.get(), 1.f) :
			spine_loader::ReadTextSkeletonFromMemory(strSkeletonData.data(), atlas.get(), 1.f);
		if (skeletonData.get() == nullptr)return false;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}

bool CSpinePlayer::addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool isBinarySkel)
{
	if (m_drawables.empty() || szAtlasPath == nullptr || szSkelPath == nullptr)return false;

	std::string atlasBytes = ReadFileAsUtf8PathBytes(szAtlasPath);
	if (atlasBytes.empty()) return false;
	StripUtf8Bom(atlasBytes);
	std::string atlasDir = GetParentDirUtf8(szAtlasPath);

	std::unique_ptr<spine::Atlas> atlas = std::make_unique<spine::Atlas>(atlasBytes.c_str(), static_cast<int>(atlasBytes.size()), atlasDir.c_str(), &m_textureLoader);
	if (atlas.get() == nullptr)return false;

	std::string skeletonBytes = ReadFileAsUtf8PathBytes(szSkelPath);
	if (skeletonBytes.empty()) return false;
	if (!isBinarySkel) StripUtf8Bom(skeletonBytes);

	std::shared_ptr<spine::SkeletonData> skeletonData = isBinarySkel ?
		spine_loader::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(skeletonBytes.data()), static_cast<int>(skeletonBytes.size()), atlas.get(), 1.f) :
		spine_loader::ReadTextSkeletonFromMemory(skeletonBytes.c_str(), atlas.get(), 1.f);
	if (skeletonData.get() == nullptr)return false;

	bool bRet = addDrawable(skeletonData.get());
	if (!bRet)return false;

	m_atlases.push_back(std::move(atlas));
	m_skeletonData.push_back(std::move(skeletonData));
	if (m_isDrawOrderReversed)
	{
		std::rotate(m_drawables.rbegin(), m_drawables.rbegin() + 1, m_drawables.rend());
	}

	restartAnimation();
	resetScale();

	return true;
}

size_t CSpinePlayer::getNumberOfSpines() const noexcept
{
	return m_drawables.size();
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

	updatePosition();
}

void CSpinePlayer::addOffset(int iX, int iY)
{

	float dx, dy;
	switch (m_iRotationSteps)
	{
	case 1:  dx =  iY; dy = -iX; break;
	case 2:  dx = -iX; dy = -iY; break;
	case 3:  dx = -iY; dy =  iX; break;
	default: dx =  iX; dy =  iY; break;
	}
	if (m_bFlipX) dx = -dx;
	m_fOffset.x += dx / m_fSkeletonScale;
	m_fOffset.y += dy / m_fSkeletonScale;
	updatePosition();
}

void CSpinePlayer::shiftAnimation()
{
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

	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(szAnimationName);
		if (pAnimation != nullptr)
		{
			pDrawable->animationState()->setAnimation(0, pAnimation->getName(), loop);
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

	for (const auto& pDrawable : m_drawables)
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
	
	bool bNewPma = false;
	if (!m_drawables.empty())
	{
		bNewPma = !m_drawables.front()->isAlphaPremultiplied();
	}
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->premultiplyAlpha(bNewPma);
	}

	
	
	
	m_textureLoader.setPremultiplyOnLoad(!bNewPma);

	
	
	for (const auto& pAtlas : m_atlases)
	{
		if (pAtlas.get() == nullptr) continue;
		spine::Vector<spine::AtlasPage*>& pages = pAtlas->getPages();
		for (size_t i = 0; i < pages.size(); ++i)
		{
			spine::AtlasPage* pPage = pages[i];
			if (pPage == nullptr) continue;

#if defined (SPINE_41) || defined (SPINE_42)
			void* pOldTexture = pPage->texture;
#else
			void* pOldTexture = pPage->getRendererObject();
#endif
			if (pOldTexture != nullptr)
			{
				m_textureLoader.unload(pOldTexture);
#if defined (SPINE_41) || defined (SPINE_42)
				pPage->texture = nullptr;
#else
				pPage->setRendererObject(nullptr);
#endif
			}

			
			if (pPage->texturePath.length() > 0)
			{
				m_textureLoader.load(*pPage, pPage->texturePath);
			}
		}
	}
}

void CSpinePlayer::toggleBlendModeAdoption()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->forceBlendModeNormal(!pDrawable->isBlendModeNormalForced());
	}
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
	for (const auto& pDrawable : m_drawables)
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
	if (m_nSkinIndex < m_skinNames.size())
		return m_skinNames[m_nSkinIndex];
	return std::string();
}

void CSpinePlayer::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
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
	for (const auto& pDrawable : m_drawables)
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

	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutList(leaveOutList);
	}
}

void CSpinePlayer::mixSkins(const std::vector<std::string>& skinNames)
{

	for (const auto& pDrawable : m_drawables)
		pDrawable->skeleton()->setSkin(nullptr);
	for (spine::Skin* s : m_dynamicSkins) delete s;
	m_dynamicSkins.clear();

	for (const auto& pDrawable : m_drawables)
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

	for (const auto& pDrawable : m_drawables)
	{
		spine::Animation* firstAnimation = pDrawable->skeleton()->getData()->findAnimation(firstAnimationName.c_str());
		if (firstAnimation == nullptr)continue;

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
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutCallback(pFunc);
	}
}

std::unordered_map<std::string, std::vector<std::string>> CSpinePlayer::getSlotNamesWithTheirAttachments()
{
	std::unordered_map<std::string, std::vector<std::string>> slotAttachmentMap;


	for (const auto& skeletonDatum : m_skeletonData)
	{
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

	for (const auto& pDrawable : m_drawables)
	{
		spine::Slot* pSlot = FindSlot(pDrawable->skeleton());
		if (pSlot == nullptr)continue;

		spine::Attachment* pAttachment = FindAttachment(pDrawable->skeleton()->getData());
		if (pAttachment == nullptr)continue;


		if (pSlot->getAttachment() != nullptr)
		{
			const char* animationName = m_animationNames[m_nAnimationIndex].c_str();
			spine::Animation* pAnimation = pDrawable->skeleton()->getData()->findAnimation(animationName);
			if (pAnimation == nullptr)continue;

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
	return m_fOffset;
}

void CSpinePlayer::setOffset(float fX, float fY) noexcept
{
	 m_fOffset.x = fX;
	 m_fOffset.y = fY;
	 updatePosition();
}

float CSpinePlayer::getSkeletonScale() const noexcept
{
	return m_fSkeletonScale;
}

void CSpinePlayer::setSkeletonScale(float fScale)
{
	m_fSkeletonScale = fScale;
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
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->animationState()->getData()->setDefaultMix(mixTime);
	}
}

bool CSpinePlayer::isFlipX() const noexcept
{
	return m_bFlipX;
}

void CSpinePlayer::toggleFlipX() noexcept
{
	m_bFlipX = !m_bFlipX;
}

int CSpinePlayer::getRotationSteps() const noexcept
{
	return m_iRotationSteps;
}

void CSpinePlayer::rotate90() noexcept
{
	m_iRotationSteps = (m_iRotationSteps + 1) % 4;
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

		auto& animations = pSkeletonDatum->getAnimations();
		for (size_t i = 0; i < animations.size(); ++i)
		{
			const char* szAnimationName = animations[i]->getName().buffer();
			if (szAnimationName == nullptr)continue;

			const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(szAnimationName);
		}

		auto& skins = pSkeletonDatum->getSkins();
		for (size_t i = 0; i < skins.size(); ++i)
		{
			const char* szSkinName = skins[i]->getName().buffer();
			if (szSkinName == nullptr)continue;

			const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(szSkinName);
		}

		auto& slots = pSkeletonDatum->getSlots();
		for (size_t ii = 0; ii < slots.size(); ++ii)
		{
			const char* szName = slots[ii]->getName().buffer();
			const auto& iter = std::find(m_slotNames.begin(), m_slotNames.end(), szName);
			if (iter == m_slotNames.cend())m_slotNames.push_back(szName);
		}
	}

	workOutDefaultOffset();

	restartAnimation();

	resetScale();

	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->update(0.f);
	}

	return m_animationNames.size() > 0;
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
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->skeleton()->setPosition(m_fBaseSize.x / 2 - m_fOffset.x, m_fBaseSize.y / 2 - m_fOffset.y);
	}
}

void CSpinePlayer::clearAnimationTracks()
{
	for (const auto& pDrawable : m_drawables)
	{
		const auto& trackEntry = pDrawable->animationState()->getTracks();
		for (size_t iTrack = 1; iTrack < trackEntry.size(); ++iTrack)
		{
			pDrawable->animationState()->setEmptyAnimation(iTrack, 0.f);
		}
	}
}




