/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Texture.cpp
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Textures

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Texture.h"

#include <gli/gli.hpp>

namespace SliceEngine {
    namespace SliceEngineTypes {
        void Texture::LoadTexture(Texture* t, std::string const& filepath) {

            gli::texture Texture = gli::load(filepath);
            if (Texture.empty())
                return;

            gli::gl GL(gli::gl::PROFILE_GL33);
            gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
            GLenum Target = GL.translate(Texture.target());

            GLuint TextureName = 0;
            glGenTextures(1, &TextureName);
            glBindTexture(Target, TextureName);
            glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
            glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
            glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
            glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
            glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);
            glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glm::tvec3<GLsizei> const Extent(Texture.extent());
            GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

            switch (Texture.target())
            {
            case gli::TARGET_1D:
                glTexStorage1D(
                    Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
                break;
            case gli::TARGET_1D_ARRAY:
            case gli::TARGET_2D:
            case gli::TARGET_CUBE:
                glTexStorage2D(
                    Target, static_cast<GLint>(Texture.levels()), Format.Internal,
                    Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
                break;
            case gli::TARGET_2D_ARRAY:
            case gli::TARGET_3D:
            case gli::TARGET_CUBE_ARRAY:
                glTexStorage3D(
                    Target, static_cast<GLint>(Texture.levels()), Format.Internal,
                    Extent.x, Extent.y,
                    Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
                break;
            default:
                assert(0);
                break;
            }

            for (std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
                for (std::size_t Face = 0; Face < Texture.faces(); ++Face)
                    for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
                    {
                        GLsizei const LayerGL = static_cast<GLsizei>(Layer);
                        glm::tvec3<GLsizei> Extents(Texture.extent(Level));
                        Target = gli::is_target_cube(Texture.target())
                            ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
                            : Target;

                        switch (Texture.target())
                        {
                        case gli::TARGET_1D:
                            if (gli::is_compressed(Texture.format()))
                                glCompressedTexSubImage1D(
                                    Target, static_cast<GLint>(Level), 0, Extents.x,
                                    Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                                    Texture.data(Layer, Face, Level));
                            else
                                glTexSubImage1D(
                                    Target, static_cast<GLint>(Level), 0, Extents.x,
                                    Format.External, Format.Type,
                                    Texture.data(Layer, Face, Level));
                            break;
                        case gli::TARGET_1D_ARRAY:
                        case gli::TARGET_2D:
                        case gli::TARGET_CUBE:
                            if (gli::is_compressed(Texture.format()))
                                glCompressedTexSubImage2D(
                                    Target, static_cast<GLint>(Level),
                                    0, 0,
                                    Extents.x,
                                    Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extents.y,
                                    Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                                    Texture.data(Layer, Face, Level));
                            else
                                glTexSubImage2D(
                                    Target, static_cast<GLint>(Level),
                                    0, 0,
                                    Extents.x,
                                    Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extents.y,
                                    Format.External, Format.Type,
                                    Texture.data(Layer, Face, Level));
                            break;
                        case gli::TARGET_2D_ARRAY:
                        case gli::TARGET_3D:
                        case gli::TARGET_CUBE_ARRAY:
                            if (gli::is_compressed(Texture.format()))
                                glCompressedTexSubImage3D(
                                    Target, static_cast<GLint>(Level),
                                    0, 0, 0,
                                    Extents.x, Extents.y,
                                    Texture.target() == gli::TARGET_3D ? Extents.z : LayerGL,
                                    Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
                                    Texture.data(Layer, Face, Level));
                            else
                                glTexSubImage3D(
                                    Target, static_cast<GLint>(Level),
                                    0, 0, 0,
                                    Extents.x, Extents.y,
                                    Texture.target() == gli::TARGET_3D ? Extents.z : LayerGL,
                                    Format.External, Format.Type,
                                    Texture.data(Layer, Face, Level));
                            break;
                        default: assert(0); break;
                        }
                    }

            t->texture_id = TextureName;
            t->bindless_id = glGetTextureHandleARB(TextureName);
            glMakeTextureHandleResidentARB(t->bindless_id);
        }

        Texture Texture::LoadColorTexture(float r, float g, float b, float a)
        {
            float col[4] = { r,g,b,a };
            texture_id = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
            glTextureStorage2D(texture_id, 1, GL_RGBA32F, 1, 1);
            glTextureSubImage2D(texture_id, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, col);
            glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            bindless_id = glGetTextureHandleARB(texture_id);
            glMakeTextureHandleResidentARB(bindless_id);
            return { texture_id, bindless_id };
        }

        void Texture::DestroyTexture() {
            glMakeTextureHandleNonResidentARB(bindless_id);
            glDeleteTextures(1, &texture_id);
            return;
        }
    }

}

