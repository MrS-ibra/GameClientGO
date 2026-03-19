#include "EmojiRenderer.h"
#include "render2d.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

EmojiRenderer* EmojiRenderer::s_instance = nullptr;

EmojiRenderer* EmojiRenderer::Get_Instance()
{
    if (!s_instance)
        s_instance = new EmojiRenderer();
    return s_instance;
}

void EmojiRenderer::Init(const char* emoji_folder)
{
    if (!m_folder.empty())
        return;

    m_folder = emoji_folder;
    if (!m_folder.empty() && m_folder.back() != '\\' && m_folder.back() != '/')
        m_folder += '\\';
}

void EmojiRenderer::Shutdown()
{
    for (auto& pair : m_textures)
    {
        if (pair.second)
            pair.second->Release_Ref();
    }
    m_textures.clear();
    m_missing.clear();
}

std::string EmojiRenderer::Get_Emoji_Path(uint32_t codepoint) const
{
    char buf[64];
    sprintf(buf, "%semoji_u%x.png", m_folder.c_str(), codepoint);
    return buf;
}

bool EmojiRenderer::Has_Emoji(uint32_t codepoint) const
{
    if (codepoint < 0x1F000) return false;
    if (m_missing.count(codepoint)) return false;
    if (m_textures.count(codepoint)) return true;
    std::string path = Get_Emoji_Path(codepoint);
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES);
}

TextureClass* EmojiRenderer::Load_PNG(const char* path)
{
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 4);
    if (!data)
        return nullptr;

    TextureClass* tex = W3DNEW TextureClass(w, h, WW3D_FORMAT_A8R8G8B8, MIP_LEVELS_1);
    if (!tex)
    {
        stbi_image_free(data);
        return nullptr;
    }

    tex->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    tex->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
    tex->Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_BEST);
    tex->Get_Filter().Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_BEST);
    tex->Get_Filter().Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);

    SurfaceClass* surface = tex->Get_Surface_Level();
    if (surface)
    {
        int stride = 0;
        uint32_t* dst = (uint32_t*)surface->Lock(&stride);
        if (dst)
        {
            int dst_pitch = stride / 4;
            for (int row = 0; row < h; row++)
            {
                for (int col = 0; col < w; col++)
                {
                    int src_idx = (row * w + col) * 4;
                    uint8 r = data[src_idx + 0];
                    uint8 g = data[src_idx + 1];
                    uint8 b = data[src_idx + 2];
                    uint8 a = data[src_idx + 3];
                    dst[row * dst_pitch + col] = (a << 24) | (r << 16) | (g << 8) | b;
                }
            }
            surface->Unlock();
        }
        surface->Release_Ref();
    }

    stbi_image_free(data);
    return tex;
}

TextureClass* EmojiRenderer::Get_Texture(uint32_t codepoint)
{
    auto it = m_textures.find(codepoint);
    if (it != m_textures.end())
        return it->second;

    if (m_missing.count(codepoint))
        return nullptr;

    std::string path = Get_Emoji_Path(codepoint);
    TextureClass* tex = Load_PNG(path.c_str());

    if (!tex)
    {
        m_missing[codepoint] = true;
        return nullptr;
    }

    m_textures[codepoint] = tex;
    return tex;
}

void EmojiRenderer::Draw_Emoji(uint32_t codepoint, float x, float y, float size, Render2DClass* renderer)
{
    TextureClass* tex = Get_Texture(codepoint);
    if (!tex) return;

    RectClass screen_rect(x, y, x + size, y + size);
    RectClass uv_rect(0.0f, 0.0f, 1.0f, 1.0f);

    ShaderClass shader = Render2DClass::Get_Default_Shader();
    shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);
    shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_SRC_ALPHA);
    shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);

    renderer->Set_Texture(tex);
    ShaderClass* curr_shader = renderer->Get_Shader();
    *curr_shader = shader;
    renderer->Add_Quad(screen_rect, uv_rect, 0xFFFFFFFF);
}
