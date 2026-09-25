

#ifndef LIVE2D_CUBISM_CORE_H
#define LIVE2D_CUBISM_CORE_H

#if defined(__cplusplus)
extern "C"
{
#endif


#if !defined(csmApi)
#define csmApi
#endif


    typedef struct csmMoc csmMoc;

    typedef struct csmModel csmModel;

    typedef unsigned int csmVersion;

    enum
    {
        csmAlignofMoc = 64,

        csmAlignofModel = 16
    };

    enum
    {
        csmBlendAdditive = 1 << 0,

        csmBlendMultiplicative = 1 << 1,

        csmIsDoubleSided = 1 << 2,

        csmIsInvertedMask = 1 << 3
    };

    enum
    {
        csmIsVisible = 1 << 0,
        csmVisibilityDidChange = 1 << 1,
        csmOpacityDidChange = 1 << 2,
        csmDrawOrderDidChange = 1 << 3,
        csmRenderOrderDidChange = 1 << 4,
        csmVertexPositionsDidChange = 1 << 5,
        csmBlendColorDidChange = 1 << 6
    };

    typedef unsigned char csmFlags;

    enum
    {
        csmMocVersion_Unknown = 0,
        csmMocVersion_30 = 1,
        csmMocVersion_33 = 2,
        csmMocVersion_40 = 3,
        csmMocVersion_42 = 4,
        csmMocVersion_50 = 5
    };

    typedef unsigned int csmMocVersion;

    enum
    {
        csmParameterType_Normal = 0,

        csmParameterType_BlendShape = 1
    };

    typedef int csmParameterType;

    typedef struct
    {
        float X;

        float Y;
    } csmVector2;

    typedef struct
    {
        float X;

        float Y;

        float Z;

        float W;
    } csmVector4;

    typedef void (*csmLogFunction)(const char* message);

#if CSM_CORE_WIN32_DLL
#define csmCallingConvention __stdcall
#else
#define csmCallingConvention
#endif


    csmApi csmVersion csmCallingConvention csmGetVersion();

    csmApi csmMocVersion csmCallingConvention csmGetLatestMocVersion();

    csmApi csmMocVersion csmCallingConvention csmGetMocVersion(const void* address, const unsigned int size);


    csmApi int csmCallingConvention csmHasMocConsistency(void* address, const unsigned int size);


    csmApi csmLogFunction csmCallingConvention csmGetLogFunction();

    csmApi void csmCallingConvention csmSetLogFunction(csmLogFunction handler);


    csmApi csmMoc* csmCallingConvention csmReviveMocInPlace(void* address, const unsigned int size);


    csmApi unsigned int csmCallingConvention csmGetSizeofModel(const csmMoc* moc);

    csmApi csmModel* csmCallingConvention csmInitializeModelInPlace(const csmMoc* moc,
                                                                    void* address,
                                                                    const unsigned int size);

    csmApi void csmCallingConvention csmUpdateModel(csmModel* model);


    csmApi void csmCallingConvention csmReadCanvasInfo(const csmModel* model,
                                                       csmVector2* outSizeInPixels,
                                                       csmVector2* outOriginInPixels,
                                                       float* outPixelsPerUnit);


    csmApi int csmCallingConvention csmGetParameterCount(const csmModel* model);

    csmApi const char** csmCallingConvention csmGetParameterIds(const csmModel* model);


    csmApi const csmParameterType* csmCallingConvention csmGetParameterTypes(const csmModel* model);

    csmApi const float* csmCallingConvention csmGetParameterMinimumValues(const csmModel* model);

    csmApi const float* csmCallingConvention csmGetParameterMaximumValues(const csmModel* model);

    csmApi const float* csmCallingConvention csmGetParameterDefaultValues(const csmModel* model);

    csmApi float* csmCallingConvention csmGetParameterValues(csmModel* model);

    csmApi const int* csmCallingConvention csmGetParameterKeyCounts(const csmModel* model);

    csmApi const float** csmCallingConvention csmGetParameterKeyValues(const csmModel* model);



    csmApi int csmCallingConvention csmGetPartCount(const csmModel* model);

    csmApi const char** csmCallingConvention csmGetPartIds(const csmModel* model);

    csmApi float* csmCallingConvention csmGetPartOpacities(csmModel* model);

    csmApi const int* csmCallingConvention csmGetPartParentPartIndices(const csmModel* model);


    csmApi int csmCallingConvention csmGetDrawableCount(const csmModel* model);

    csmApi const char** csmCallingConvention csmGetDrawableIds(const csmModel* model);

    csmApi const csmFlags* csmCallingConvention csmGetDrawableConstantFlags(const csmModel* model);

    csmApi const csmFlags* csmCallingConvention csmGetDrawableDynamicFlags(const csmModel* model);

    csmApi const int* csmCallingConvention csmGetDrawableTextureIndices(const csmModel* model);

    csmApi const int* csmCallingConvention csmGetDrawableDrawOrders(const csmModel* model);

    csmApi const int* csmCallingConvention csmGetDrawableRenderOrders(const csmModel* model);

    csmApi const float* csmCallingConvention csmGetDrawableOpacities(const csmModel* model);

    csmApi const int* csmCallingConvention csmGetDrawableMaskCounts(const csmModel* model);

    csmApi const int** csmCallingConvention csmGetDrawableMasks(const csmModel* model);

    csmApi const int* csmCallingConvention csmGetDrawableVertexCounts(const csmModel* model);

    csmApi const csmVector2** csmCallingConvention csmGetDrawableVertexPositions(const csmModel* model);

    csmApi const csmVector2** csmCallingConvention csmGetDrawableVertexUvs(const csmModel* model);

    csmApi const int* csmCallingConvention csmGetDrawableIndexCounts(const csmModel* model);

    csmApi const unsigned short** csmCallingConvention csmGetDrawableIndices(const csmModel* model);

    csmApi const csmVector4* csmCallingConvention csmGetDrawableMultiplyColors(const csmModel* model);

    csmApi const csmVector4* csmCallingConvention csmGetDrawableScreenColors(const csmModel* model);

    csmApi const int* csmCallingConvention csmGetDrawableParentPartIndices(const csmModel* model);

    csmApi void csmCallingConvention csmResetDrawableDynamicFlags(csmModel* model);

#if defined(__cplusplus)
}
#endif

#endif
