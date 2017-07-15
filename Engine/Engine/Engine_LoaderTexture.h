#pragma once
#ifndef ENGINE_LOADER_TEXTURE_H
#define ENGINE_LAODER_TEXTURE_H

class Texture;

namespace Engine{
    namespace Resources{
        namespace Load{

            void load(Texture*);
            void unload(Texture*);

            class LoaderTexture{
                friend class ::Texture;
                public:
                    static void load(Texture*);
                    static void unload(Texture*);
            };
        };
    };
};

#endif