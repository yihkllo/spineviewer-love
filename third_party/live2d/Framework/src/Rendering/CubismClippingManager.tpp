

#pragma once

template <class T_ClippingContext, class T_OffscreenSurface>
CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::CubismClippingManager() :
                                                                    _clippingMaskBufferSize(256, 256)
{
    CubismRenderer::CubismTextureColor* tmp = NULL;
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 1.0f;
    tmp->G = 0.0f;
    tmp->B = 0.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 1.0f;
    tmp->B = 0.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 0.0f;
    tmp->B = 1.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 0.0f;
    tmp->B = 0.0f;
    tmp->A = 1.0f;
    _channelColors.PushBack(tmp);
}

template <class T_ClippingContext, class T_OffscreenSurface>
CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::~CubismClippingManager()
{
    for (csmUint32 i = 0; i < _clippingContextListForMask.GetSize(); i++)
    {
        if (_clippingContextListForMask[i]) CSM_DELETE_SELF(T_ClippingContext, _clippingContextListForMask[i]);
        _clippingContextListForMask[i] = NULL;
    }

    for (csmUint32 i = 0; i < _clippingContextListForDraw.GetSize(); i++)
    {
        _clippingContextListForDraw[i] = NULL;
    }

    for (csmUint32 i = 0; i < _channelColors.GetSize(); i++)
    {
        if (_channelColors[i]) CSM_DELETE(_channelColors[i]);
        _channelColors[i] = NULL;
    }

    if (_clearedMaskBufferFlags.GetSize() != 0)
    {
        _clearedMaskBufferFlags.Clear();
        _clearedMaskBufferFlags = NULL;
    }
}

template <class T_ClippingContext, class T_OffscreenSurface>
void CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::Initialize(CubismModel& model, const csmInt32 maskBufferCount)
{
    _renderTextureCount = maskBufferCount;

    for (csmInt32 i = 0; i < _renderTextureCount; ++i)
    {
        _clearedMaskBufferFlags.PushBack(false);
    }

    for (csmInt32 i = 0; i < model.GetDrawableCount(); i++)
    {
        if (model.GetDrawableMaskCounts()[i] <= 0)
        {
            _clippingContextListForDraw.PushBack(NULL);
            continue;
        }

        T_ClippingContext* cc = FindSameClip(model.GetDrawableMasks()[i], model.GetDrawableMaskCounts()[i]);
        if (cc == NULL)
        {
            cc = CSM_NEW T_ClippingContext(this, model, model.GetDrawableMasks()[i], model.GetDrawableMaskCounts()[i]);
            _clippingContextListForMask.PushBack(cc);
        }

        cc->AddClippedDrawable(i);

        _clippingContextListForDraw.PushBack(cc);
    }
}

template <class T_ClippingContext, class T_OffscreenSurface>
T_ClippingContext* CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::FindSameClip(const csmInt32* drawableMasks, csmInt32 drawableMaskCounts) const
{
    for (csmUint32 i = 0; i < _clippingContextListForMask.GetSize(); i++)
    {
        T_ClippingContext* cc = _clippingContextListForMask[i];
        const csmInt32 count = cc->_clippingIdCount;
        if (count != drawableMaskCounts) continue;
        csmInt32 samecount = 0;

        for (csmInt32 j = 0; j < count; j++)
        {
            const csmInt32 clipId = cc->_clippingIdList[j];
            for (csmInt32 k = 0; k < count; k++)
            {
                if (drawableMasks[k] == clipId)
                {
                    samecount++;
                    break;
                }
            }
        }
        if (samecount == count)
        {
            return cc;
        }
    }
    return NULL;
}

template <class T_ClippingContext, class T_OffscreenSurface>
void CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::SetupMatrixForHighPrecision(CubismModel& model, csmBool isRightHanded)
{
    csmInt32 usingClipCount = 0;
    for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
    {
        T_ClippingContext* cc = _clippingContextListForMask[clipIndex];

        CalcClippedDrawTotalBounds(model, cc);

        if (cc->_isUsing)
        {
            usingClipCount++;
        }
    }

    if (usingClipCount <= 0) {
        return;
    }
    SetupLayoutBounds(0);

    if (_clearedMaskBufferFlags.GetSize() != _renderTextureCount)
    {
        _clearedMaskBufferFlags.Clear();

        for (csmInt32 i = 0; i < _renderTextureCount; ++i)
        {
            _clearedMaskBufferFlags.PushBack(false);
        }
    }
    else
    {
        for (csmInt32 i = 0; i < _renderTextureCount; ++i)
        {
            _clearedMaskBufferFlags[i] = false;
        }
    }

    for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
    {
        T_ClippingContext* clipContext = _clippingContextListForMask[clipIndex];
        csmRectF* allClippedDrawRect = clipContext->_allClippedDrawRect;
        csmRectF* layoutBoundsOnTex01 = clipContext->_layoutBounds;
        const csmFloat32 MARGIN = 0.05f;
        csmFloat32 scaleX = 0.0f;
        csmFloat32 scaleY = 0.0f;
        const csmFloat32 ppu = model.GetPixelsPerUnit();
        const csmFloat32 maskPixelWidth = clipContext->GetClippingManager()->_clippingMaskBufferSize.X;
        const csmFloat32 maskPixelHeight = clipContext->GetClippingManager()->_clippingMaskBufferSize.Y;
        const csmFloat32 physicalMaskWidth = layoutBoundsOnTex01->Width * maskPixelWidth;
        const csmFloat32 physicalMaskHeight = layoutBoundsOnTex01->Height * maskPixelHeight;

        _tmpBoundsOnModel.SetRect(allClippedDrawRect);
        if (_tmpBoundsOnModel.Width * ppu > physicalMaskWidth)
        {
            _tmpBoundsOnModel.Expand(allClippedDrawRect->Width * MARGIN, 0.0f);
            scaleX = layoutBoundsOnTex01->Width / _tmpBoundsOnModel.Width;
        }
        else
        {
            scaleX = ppu / physicalMaskWidth;
        }

        if (_tmpBoundsOnModel.Height * ppu > physicalMaskHeight)
        {
            _tmpBoundsOnModel.Expand(0.0f, allClippedDrawRect->Height * MARGIN);
            scaleY = layoutBoundsOnTex01->Height / _tmpBoundsOnModel.Height;
        }
        else
        {
            scaleY = ppu / physicalMaskHeight;
        }


        createMatrixForMask(isRightHanded, layoutBoundsOnTex01, scaleX, scaleY);

        clipContext->_matrixForMask.SetMatrix(_tmpMatrixForMask.GetArray());
        clipContext->_matrixForDraw.SetMatrix(_tmpMatrixForDraw.GetArray());
    }
}

template <class T_ClippingContext, class T_OffscreenSurface>
void CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::createMatrixForMask(csmBool isRightHanded, csmRectF* layoutBoundsOnTex01, csmFloat32 scaleX, csmFloat32 scaleY)
{
    _tmpMatrix.LoadIdentity();
    {
        _tmpMatrix.TranslateRelative(-1.0f, -1.0f);
        _tmpMatrix.ScaleRelative(2.0f, 2.0f);
    }
    {
        _tmpMatrix.TranslateRelative(layoutBoundsOnTex01->X, layoutBoundsOnTex01->Y);
        _tmpMatrix.ScaleRelative(scaleX, scaleY);
        _tmpMatrix.TranslateRelative(-_tmpBoundsOnModel.X, -_tmpBoundsOnModel.Y);
     }
     _tmpMatrixForMask.SetMatrix(_tmpMatrix.GetArray());

    _tmpMatrix.LoadIdentity();
    {
        _tmpMatrix.TranslateRelative(layoutBoundsOnTex01->X, layoutBoundsOnTex01->Y * ((isRightHanded) ? -1.0f : 1.0f));
        _tmpMatrix.ScaleRelative(scaleX, scaleY * ((isRightHanded) ? -1.0f : 1.0f));
        _tmpMatrix.TranslateRelative(-_tmpBoundsOnModel.X, -_tmpBoundsOnModel.Y);
    }

    _tmpMatrixForDraw.SetMatrix(_tmpMatrix.GetArray());
}

template <class T_ClippingContext, class T_OffscreenSurface>
void CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::SetupLayoutBounds(csmInt32 usingClipCount) const
{
    const csmInt32 useClippingMaskMaxCount = _renderTextureCount <= 1
        ? ClippingMaskMaxCountOnDefault
        : ClippingMaskMaxCountOnMultiRenderTexture * _renderTextureCount;

    if (usingClipCount <= 0 || usingClipCount > useClippingMaskMaxCount)
    {
        if (usingClipCount > useClippingMaskMaxCount)
        {
            csmInt32 count = usingClipCount - useClippingMaskMaxCount;
            CubismLogError("not supported mask count : %d\n[Details] render texture count: %d\n, mask count : %d"
                , count, _renderTextureCount, usingClipCount);
        }

        for (csmUint32 index = 0; index < _clippingContextListForMask.GetSize(); index++)
        {
            T_ClippingContext* cc = _clippingContextListForMask[index];
            cc->_layoutChannelIndex = 0;
            cc->_layoutBounds->X = 0.0f;
            cc->_layoutBounds->Y = 0.0f;
            cc->_layoutBounds->Width = 1.0f;
            cc->_layoutBounds->Height = 1.0f;
            cc->_bufferIndex = 0;
        }
        return;
    }

    const csmInt32 layoutCountMaxValue = _renderTextureCount <= 1 ? 9 : 8;

    const csmInt32 countPerSheetDiv = (usingClipCount + _renderTextureCount - 1) / _renderTextureCount;
    const csmInt32 reduceLayoutTextureCount = usingClipCount % _renderTextureCount;

    const csmInt32 divCount = countPerSheetDiv / ColorChannelCount;
    const csmInt32 modCount = countPerSheetDiv % ColorChannelCount;

    csmInt32 curClipIndex = 0;

    for (csmInt32 renderTextureIndex = 0; renderTextureIndex < _renderTextureCount; renderTextureIndex++)
    {
        for (csmInt32 channelIndex = 0; channelIndex < ColorChannelCount; channelIndex++)
        {
            csmInt32 layoutCount = divCount + (channelIndex < modCount ? 1 : 0);

            const csmInt32 checkChannelIndex = modCount + (divCount < 1 ? -1 : 0);

            if (channelIndex == checkChannelIndex && reduceLayoutTextureCount > 0)
            {
                layoutCount -= !(renderTextureIndex < reduceLayoutTextureCount) ? 1 : 0;
            }

            if (layoutCount == 0)
            {
            }
            else if (layoutCount == 1)
            {
                T_ClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelIndex = channelIndex;
                cc->_layoutBounds->X = 0.0f;
                cc->_layoutBounds->Y = 0.0f;
                cc->_layoutBounds->Width = 1.0f;
                cc->_layoutBounds->Height = 1.0f;
                cc->_bufferIndex = renderTextureIndex;
            }
            else if (layoutCount == 2)
            {
                for (csmInt32 i = 0; i < layoutCount; i++)
                {
                    const csmInt32 xpos = i % 2;

                    T_ClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                    cc->_layoutChannelIndex = channelIndex;

                    cc->_layoutBounds->X = xpos * 0.5f;
                    cc->_layoutBounds->Y = 0.0f;
                    cc->_layoutBounds->Width = 0.5f;
                    cc->_layoutBounds->Height = 1.0f;
                    cc->_bufferIndex = renderTextureIndex;
                }
            }
            else if (layoutCount <= 4)
            {
                for (csmInt32 i = 0; i < layoutCount; i++)
                {
                    const csmInt32 xpos = i % 2;
                    const csmInt32 ypos = i / 2;

                    T_ClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                    cc->_layoutChannelIndex = channelIndex;

                    cc->_layoutBounds->X = xpos * 0.5f;
                    cc->_layoutBounds->Y = ypos * 0.5f;
                    cc->_layoutBounds->Width = 0.5f;
                    cc->_layoutBounds->Height = 0.5f;
                    cc->_bufferIndex = renderTextureIndex;
                }
            }
            else if (layoutCount <= layoutCountMaxValue)
            {
                for (csmInt32 i = 0; i < layoutCount; i++)
                {
                    const csmInt32 xpos = i % 3;
                    const csmInt32 ypos = i / 3;

                    T_ClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                    cc->_layoutChannelIndex = channelIndex;

                    cc->_layoutBounds->X = xpos / 3.0f;
                    cc->_layoutBounds->Y = ypos / 3.0f;
                    cc->_layoutBounds->Width = 1.0f / 3.0f;
                    cc->_layoutBounds->Height = 1.0f / 3.0f;
                    cc->_bufferIndex = renderTextureIndex;
                }
            }
            else
            {
                csmInt32 count = usingClipCount - useClippingMaskMaxCount;


                CubismLogError("not supported mask count : %d\n[Details] render texture count: %d\n, mask count : %d"
                    , count, _renderTextureCount, usingClipCount);

                CSM_ASSERT(0);

                for (csmInt32 i = 0; i < layoutCount; i++)
                {
                    T_ClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                    cc->_layoutChannelIndex = 0;
                    cc->_layoutBounds->X = 0.0f;
                    cc->_layoutBounds->Y = 0.0f;
                    cc->_layoutBounds->Width = 1.0f;
                    cc->_layoutBounds->Height = 1.0f;
                    cc->_bufferIndex = 0;
                }
            }
        }
    }
}

template <class T_ClippingContext, class T_OffscreenSurface>
void CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::CalcClippedDrawTotalBounds(CubismModel& model, T_ClippingContext* clippingContext)
{
    csmFloat32 clippedDrawTotalMinX = FLT_MAX, clippedDrawTotalMinY = FLT_MAX;
    csmFloat32 clippedDrawTotalMaxX = -FLT_MAX, clippedDrawTotalMaxY = -FLT_MAX;


    const csmInt32 clippedDrawCount = clippingContext->_clippedDrawableIndexList->GetSize();
    for (csmInt32 clippedDrawableIndex = 0; clippedDrawableIndex < clippedDrawCount; clippedDrawableIndex++)
    {
        const csmInt32 drawableIndex = (*clippingContext->_clippedDrawableIndexList)[clippedDrawableIndex];

        csmInt32 drawableVertexCount = model.GetDrawableVertexCount(drawableIndex);
        csmFloat32* drawableVertexes = const_cast<csmFloat32*>(model.GetDrawableVertices(drawableIndex));

        csmFloat32 minX = FLT_MAX, minY = FLT_MAX;
        csmFloat32 maxX = -FLT_MAX, maxY = -FLT_MAX;

        csmInt32 loop = drawableVertexCount * Constant::VertexStep;
        for (csmInt32 pi = Constant::VertexOffset; pi < loop; pi += Constant::VertexStep)
        {
            csmFloat32 x = drawableVertexes[pi];
            csmFloat32 y = drawableVertexes[pi + 1];
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }

        if (minX == FLT_MAX) continue;

        if (minX < clippedDrawTotalMinX) clippedDrawTotalMinX = minX;
        if (minY < clippedDrawTotalMinY) clippedDrawTotalMinY = minY;
        if (maxX > clippedDrawTotalMaxX) clippedDrawTotalMaxX = maxX;
        if (maxY > clippedDrawTotalMaxY) clippedDrawTotalMaxY = maxY;
    }
    if (clippedDrawTotalMinX == FLT_MAX)
    {
        clippingContext->_allClippedDrawRect->X = 0.0f;
        clippingContext->_allClippedDrawRect->Y = 0.0f;
        clippingContext->_allClippedDrawRect->Width = 0.0f;
        clippingContext->_allClippedDrawRect->Height = 0.0f;
        clippingContext->_isUsing = false;
    }
    else
    {
        clippingContext->_isUsing = true;
        csmFloat32 w = clippedDrawTotalMaxX - clippedDrawTotalMinX;
        csmFloat32 h = clippedDrawTotalMaxY - clippedDrawTotalMinY;
        clippingContext->_allClippedDrawRect->X = clippedDrawTotalMinX;
        clippingContext->_allClippedDrawRect->Y = clippedDrawTotalMinY;
        clippingContext->_allClippedDrawRect->Width = w;
        clippingContext->_allClippedDrawRect->Height = h;
    }
}

template <class T_ClippingContext, class T_OffscreenSurface>
csmVector<T_ClippingContext*>* CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::GetClippingContextListForDraw()
{
    return &_clippingContextListForDraw;
}

template <class T_ClippingContext, class T_OffscreenSurface>
CubismVector2 CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::GetClippingMaskBufferSize() const
{
    return _clippingMaskBufferSize;
}

template <class T_ClippingContext, class T_OffscreenSurface>
csmInt32 CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::GetRenderTextureCount()
{
    return _renderTextureCount;
}

template <class T_ClippingContext, class T_OffscreenSurface>
CubismRenderer::CubismTextureColor* CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::GetChannelFlagAsColor(csmInt32 channelIndex)
{
    return _channelColors[channelIndex];
}

template <class T_ClippingContext, class T_OffscreenSurface>
void CubismClippingManager<T_ClippingContext, T_OffscreenSurface>::SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height)
{
    _clippingMaskBufferSize = CubismVector2(width, height);
}
