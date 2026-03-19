#pragma once
#include <unordered_map>
#include <cstdint>
#include <string>

// Forward declares
class TextureClass;
class Render2DClass;

class EmojiRenderer
{
public:
    static EmojiRenderer* Get_Instance();

    void Init(const char* emoji_folder);
    void Shutdown();

    // Returns true if we have a texture for this codepoint
    bool Has_Emoji(uint32_t codepoint) const;

    // Get texture for codepoint (loads on demand if not loaded yet)
    TextureClass* Get_Texture(uint32_t codepoint);

    // Render an emoji at screen position x,y with given size
    void Draw_Emoji(uint32_t codepoint, float x, float y, float size, Render2DClass* renderer);

private:
    EmojiRenderer() {}
    ~EmojiRenderer() { Shutdown(); }

    TextureClass* Load_PNG(const char* path);
    std::string Get_Emoji_Path(uint32_t codepoint) const;

    std::string m_folder;
    std::unordered_map<uint32_t, TextureClass*> m_textures;
    // Cache of codepoints we already tried and failed to load
    std::unordered_map<uint32_t, bool> m_missing;

    static EmojiRenderer* s_instance;
};
