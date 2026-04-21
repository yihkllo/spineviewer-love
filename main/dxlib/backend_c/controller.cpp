#include <Windows.h>
#include <string>
#include <vector>
#include <spine/extension.h>

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

CSpinePlayerC::CSpinePlayerC()
{

}

CSpinePlayerC::~CSpinePlayerC()
{

}


bool CSpinePlayerC::loadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel)
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

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromMemory(atlasBytes.c_str(), static_cast<int>(atlasBytes.size()), atlasDir.c_str(), nullptr);
		if (atlas.get() == nullptr)continue;

		std::string skeletonBytes = ReadFileAsUtf8PathBytes(strSkeletonPath);
		if (skeletonBytes.empty()) continue;
		if (!isBinarySkel) StripUtf8Bom(skeletonBytes);

		std::shared_ptr<spSkeletonData> skeletonData = isBinarySkel ?
			spine_loader_c::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(skeletonBytes.data()), static_cast<int>(skeletonBytes.size()), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromMemory(skeletonBytes.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)continue;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}

bool CSpinePlayerC::loadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool isBinarySkel)
{
	if (atlasData.size() != skelData.size() || atlasData.size() != atlasPaths.size())return false;
	clearDrawables();

	for (size_t i = 0; i < atlasData.size(); ++i)
	{
		const std::string& strAtlasDatum = atlasData[i];
		const std::string& strAtlasPath = atlasPaths[i];
		const std::string& strSkeletonData = skelData[i];

		std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromMemory(strAtlasDatum.c_str(), static_cast<int>(strAtlasDatum.size()), strAtlasPath.c_str(), nullptr);
		if (atlas.get() == nullptr)continue;

		std::shared_ptr<spSkeletonData> skeletonData = isBinarySkel ?
			spine_loader_c::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>((strSkeletonData.c_str())), static_cast<int>(strSkeletonData.size()), atlas.get()) :
			spine_loader_c::ReadTextSkeletonFromMemory(strSkeletonData.c_str(), atlas.get());
		if (skeletonData.get() == nullptr)continue;

		m_atlases.push_back(std::move(atlas));
		m_skeletonData.push_back(std::move(skeletonData));
	}

	if (m_skeletonData.empty())return false;

	return setupDrawables();
}

bool CSpinePlayerC::addSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool bBinary)
{
	if (m_drawables.empty() || szAtlasPath == nullptr || szSkelPath == nullptr)return false;

	std::string atlasBytes = ReadFileAsUtf8PathBytes(szAtlasPath);
	if (atlasBytes.empty()) return false;
	StripUtf8Bom(atlasBytes);
	std::string atlasDir = GetParentDirUtf8(szAtlasPath);

	std::shared_ptr<spAtlas> atlas = spine_loader_c::CreateAtlasFromMemory(atlasBytes.c_str(), static_cast<int>(atlasBytes.size()), atlasDir.c_str(), nullptr);
	if (atlas.get() == nullptr)return false;

	std::string skeletonBytes = ReadFileAsUtf8PathBytes(szSkelPath);
	if (skeletonBytes.empty()) return false;
	if (!bBinary) StripUtf8Bom(skeletonBytes);

	std::shared_ptr<spSkeletonData> skeletonData = bBinary ?
		spine_loader_c::ReadBinarySkeletonFromMemory(reinterpret_cast<const unsigned char*>(skeletonBytes.data()), static_cast<int>(skeletonBytes.size()), atlas.get()) :
		spine_loader_c::ReadTextSkeletonFromMemory(skeletonBytes.c_str(), atlas.get());
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

size_t CSpinePlayerC::getNumberOfSpines() const noexcept
{
	return m_drawables.size();
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

	updatePosition();
}

void CSpinePlayerC::addOffset(int iX, int iY)
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

void CSpinePlayerC::shiftAnimation()
{
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

	for (const auto& pDrawable : m_drawables)
	{
		spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton()->data, szAnimationName);
		if (pAnimation != nullptr)
		{
			spAnimationState_setAnimationByName(pDrawable->animationState(), 0, pAnimation->name, loop ? -1 : 0);
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

	for (const auto& pDrawable : m_drawables)
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
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->premultiplyAlpha(!pDrawable->isAlphaPremultiplied());
	}
}

void CSpinePlayerC::toggleBlendModeAdoption()
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->forceBlendModeNormal(!pDrawable->isBlendModeNormalForced());
	}
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
	for (const auto& pDrawable : m_drawables)
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
	if (m_nSkinIndex < m_skinNames.size())
		return m_skinNames[m_nSkinIndex];
	return std::string();
}

void CSpinePlayerC::getCurrentAnimationTime(float* fTrack, float* fLast, float* fStart, float* fEnd)
{
	for (const auto& pDrawable : m_drawables)
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
	for (const auto& pDrawable : m_drawables)
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
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutList(vBuffer.data(), static_cast<int>(vBuffer.size()));
	}
}

void CSpinePlayerC::mixSkins(const std::vector<std::string>& skinNames)
{

#if defined(SPINE_38) || defined(SPINE_40) || defined(SPINE_41) || defined(SPINE_42)
	if (m_nSkinIndex >= m_skinNames.size())return;
	const auto& currentSkinName = m_skinNames[m_nSkinIndex];

	for (const auto& pDrawble : m_drawables)
	{
		spSkin* skinToSet = spSkeletonData_findSkin(pDrawble->skeleton()->data, currentSkinName.c_str());
		if (skinToSet == nullptr)continue;

		for (const auto& skinName : skinNames)
		{
			if (currentSkinName != skinName)
			{
				spSkin* skinToAdd = spSkeletonData_findSkin(pDrawble->skeleton()->data, skinName.c_str());
				if (skinToAdd != nullptr)
				{
					spSkin_addSkin(skinToSet, skinToAdd);
				}
			}
		}
		spSkeleton_setSkin(pDrawble->skeleton, skinToSet);
		spSkeleton_setToSetupPose(pDrawble->skeleton());

		if (pDrawble->animationState() != nullptr)
		{
			spAnimationState_apply(pDrawble->animationState(), pDrawble->skeleton());
#if defined(SPINE_42)
			spSkeleton_updateWorldTransform(pDrawble->skeleton(), spPhysics::SP_PHYSICS_UPDATE);
#else
			spSkeleton_updateWorldTransform(pDrawble->skeleton());
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

	for (const auto& pDrawble : m_drawables)
	{
		spAnimation* pFirstAnimation = spSkeletonData_findAnimation(pDrawble->skeleton()->data, firstAnimationName.c_str());
		if (pFirstAnimation == nullptr)continue;

		spAnimationState_setAnimationByName(pDrawble->animationState(), 0, pFirstAnimation->name, loop ? -1 : 0);

		int iTrack = 1;
		for (size_t i = 1; i < animationNames.size(); ++i)
		{
			const auto& animationName = animationNames[i];
			if (animationName != firstAnimationName)
			{
				spAnimation* pAnimationToAdd = spSkeletonData_findAnimation(pDrawble->skeleton()->data, animationName.c_str());
				if (pAnimationToAdd != nullptr)
				{
					spAnimationState_addAnimation(pDrawble->animationState(), iTrack, pAnimationToAdd, loop ? - 1: 0, 0.f);
					++iTrack;
				}
			}
		}
	}
}
void CSpinePlayerC::setSlotExcludeCallback(bool(*pFunc)(const char*, size_t))
{
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->setLeaveOutCallback(pFunc);
	}
}

std::unordered_map<std::string, std::vector<std::string>> CSpinePlayerC::getSlotNamesWithTheirAttachments()
{
	std::unordered_map<std::string, std::vector<std::string>> slotAttachmentMap;

	for (const auto& pSkeletonDatum : m_skeletonData)
	{
		spSkin* pSkin = pSkeletonDatum->defaultSkin;
		if (pSkin == nullptr)continue;

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

	for (const auto& pDrawable : m_drawables)
	{
		spSlot* pSlot = FindSlot(pDrawable->skeleton());
		if (pSlot == nullptr)continue;

		spAttachment* pAttachment = FindAttachment(pDrawable->skeleton()->data);
		if (pAttachment == nullptr)continue;


		if (pSlot->attachment != nullptr)
		{
			const char* animationName = m_animationNames[m_nAnimationIndex].c_str();
			spAnimation* pAnimation = spSkeletonData_findAnimation(pDrawable->skeleton()->data, animationName);
			if (pAnimation == nullptr)continue;

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
	return m_fOffset;
}

void CSpinePlayerC::setOffset(float fX, float fY) noexcept
{
	 m_fOffset.x = fX;
	 m_fOffset.y = fY;
	 updatePosition();
}

float CSpinePlayerC::getSkeletonScale() const noexcept
{
	return m_fSkeletonScale;
}

void CSpinePlayerC::setSkeletonScale(float fScale) noexcept
{
	m_fSkeletonScale = fScale;
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
	for (const auto& pDrawable : m_drawables)
	{
		pDrawable->animationState()->data->defaultMix = mixTime;
	}
}

bool CSpinePlayerC::isFlipX() const noexcept
{
	return m_bFlipX;
}

void CSpinePlayerC::toggleFlipX() noexcept
{
	m_bFlipX = !m_bFlipX;
}

int CSpinePlayerC::getRotationSteps() const noexcept
{
	return m_iRotationSteps;
}

void CSpinePlayerC::rotate90() noexcept
{
	m_iRotationSteps = (m_iRotationSteps + 1) % 4;
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
	auto pDrawable = std::make_unique<CSpineDrawableC>(pSkeletonData);
	if (pDrawable.get() == nullptr)false;

	pDrawable->skeleton()->x = m_fBaseSize.x / 2;
	pDrawable->skeleton()->y = m_fBaseSize.y / 2;
	pDrawable->update(0.f);

	m_drawables.push_back(std::move(pDrawable));

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

		for (size_t i = 0; i < pSkeletonDatum->animationsCount; ++i)
		{
			const char* szAnimationName = pSkeletonDatum->animations[i]->name;
			if (szAnimationName == nullptr)continue;

			const auto& iter = std::find(m_animationNames.begin(), m_animationNames.end(), szAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(szAnimationName);
		}

		for (size_t i = 0; i < pSkeletonDatum->skinsCount; ++i)
		{
			const char* szSkinName = pSkeletonDatum->skins[i]->name;
			if (szSkinName == nullptr)continue;

			const auto& iter = std::find(m_skinNames.begin(), m_skinNames.end(), szSkinName);
			if (iter == m_skinNames.cend())m_skinNames.push_back(szSkinName);
		}

		for (size_t i = 0; i < pSkeletonDatum->slotsCount; ++i)
		{
			const char* szSlotName = pSkeletonDatum->slots[i]->name;
			if (szSlotName == nullptr)continue;

			const auto& iter = std::find(m_slotNames.begin(), m_slotNames.end(), szSlotName);
			if (iter == m_slotNames.cend())m_slotNames.push_back(szSlotName);
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
	for (const auto& drawable : m_drawables)
	{
		drawable->skeleton()->x = m_fBaseSize.x / 2 - m_fOffset.x;
		drawable->skeleton()->y = m_fBaseSize.y / 2 - m_fOffset.y;
	}
}

void CSpinePlayerC::clearAnimationTracks()
{
	for (const auto& pDdrawble : m_drawables)
	{
		for (int iTrack = 1; iTrack < pDdrawble->animationState()->tracksCount; ++iTrack)
		{
#if defined(SPINE_21) || defined(SPINE_34)
			spAnimationState_clearTrack(pDdrawble->animationState(), iTrack);
#else
			spAnimationState_setEmptyAnimation(pDdrawble->animationState(), iTrack, 0.f);
#endif
		}
	}
}




