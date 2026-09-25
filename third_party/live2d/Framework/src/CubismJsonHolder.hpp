

#pragma once

#include "Utils/CubismJson.hpp"

namespace Live2D { namespace Cubism { namespace Framework {
    class CubismJsonHolder
    {
    public:
        CubismJsonHolder()
            : _json(NULL)
        { }

        virtual ~CubismJsonHolder()
        { }

        csmBool IsValid()
        {
            return _json;
        }

    protected:

        void CreateCubismJson(const csmByte* buffer, csmSizeInt size)
        {
            _json = Utils::CubismJson::Create(buffer, size);

            if (!IsValid())
            {
                CubismLogError("[CubismJsonHolder] Invalid Json document.");
            }
        };

        void DeleteCubismJson()
        {
            Utils::CubismJson::Delete(_json);
            _json = NULL;
        }

        Utils::CubismJson* _json;
    };
}}}
