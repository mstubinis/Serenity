#include <serenity/renderer/text/TextRenderer.h>
#include <serenity/renderer/text/TextRendererIncludes.h>
#include <serenity/system/Engine.h>

namespace {
    //constexpr const glm::u8vec4 WHITE = glm::u8vec4(255_uc, 255_uc, 255_uc, 255_uc);
}


namespace {

    struct LineInfo {
        int index      = 0;
        int lineHeight = 0;
    };

    std::vector<LineInfo>      BUFFER_NEW_LINE_INDICES;
    std::vector<glm::u8vec4>   BUFFER_COLOR_STACK;
    std::string                HEX_ACCUMULATOR;

    uint8_t internal_hex_to_color(const std::string& hex) {
        uint8_t res = 0;
        for (const char character : hex) {
            int number;
            if (character >= 'A' && character <= 'F') {
                number = (character - 'A' + 10);
            } else if (character >= 'a' && character <= 'f') {
                number = (character - 'a' + 10);
            } else {
                number = character - '0';
            }
            res = (res << 4) | number;
        }
        return res;
    }
    void internal_process_newline_chars(std::string_view& text, const Font& font, float& xAdvancement) {
        xAdvancement = 0.0f;
        bool skipColorChars = false;
        int digitCount = 0;
        if (text.back() == '\n') { //do not count the very last char if it is a newline
            text.remove_suffix(1);
        }
        BUFFER_NEW_LINE_INDICES.clear();
        for (uint32_t j = 0; j < text.size(); ++j) {
            if (text[j] == char(TextStringBuilderColorToken::Start)) {
                skipColorChars = true;
            } else if (text[j] == '\n') {
                BUFFER_NEW_LINE_INDICES.emplace_back(j, uint16_t(xAdvancement));
                xAdvancement = 0.0f;
            } else if (!skipColorChars && text[j] != '\0') {
                const CharGlyph& chr = font.getGlyphData(text[j]);
                xAdvancement += float(chr.xadvance);
            }
            if (digitCount == 9) {
                skipColorChars = false;
                digitCount = 0;
            }
            if (skipColorChars) {
                ++digitCount;
            }
        }
        //add the last block
        BUFFER_NEW_LINE_INDICES.emplace_back(uint32_t(text.size()), uint16_t(xAdvancement));
        xAdvancement = 0.0f;
    }
    void internal_add_quad(Engine::priv::TextRenderer::IndiceBuffer& indices, uint32_t accumulator) {
        indices.push(accumulator + 0);
        indices.push(accumulator + 1);
        indices.push(accumulator + 2);
        indices.push(accumulator + 3);
        indices.push(accumulator + 1);
        indices.push(accumulator + 0);
    }
    void internal_add_positions(Engine::priv::TextRenderer::PositionBuffer& points, float x, float y, float z, const CharGlyph& glyph) {
        for (uint32_t k = 0; k < 4; ++k) {
            points.emplace_push(x + glyph.pts[k].x, y + glyph.pts[k].y, z);
        }
    }
    void internal_add_uvs(Engine::priv::TextRenderer::UVBuffer& uvs, const CharGlyph& glyph) {
        for (uint32_t k = 0; k < 4; ++k) {
            uvs.emplace_push(glyph.uvs[k]);
        }
    }
    void internal_add_colors(Engine::priv::TextRenderer::ColorBuffer& colors, const glm::u8vec4& color) {
        for (uint32_t k = 0; k < 4; ++k) {
            colors.emplace_push(color);
        }
    }
}

void Engine::priv::TextRenderer::internal_process_line(std::string_view text, int startIdx, int endIdx, float& x, float& y, float z, const Font& font, float newLineGlyphHeight, uint32_t& w, int lineSize) {
    int j = startIdx;
    while (j < endIdx) {
        if (text[j] == '\n') {
            y += newLineGlyphHeight;
            x = 0.0f;
        } else if (text[j] == char(TextStringBuilderColorToken::Start)) {
            int l = j + 1;
            int colorCount = 0;
            int digitCount = 0;
            std::array<uint8_t, 4> colors;
            HEX_ACCUMULATOR.clear();
            HEX_ACCUMULATOR.reserve(2);
            while (colorCount < 4) {
                HEX_ACCUMULATOR += text[l];
                digitCount++;
                if (digitCount == 2) {
                    digitCount = 0;
                    colors[colorCount] = internal_hex_to_color(HEX_ACCUMULATOR);
                    HEX_ACCUMULATOR.clear();
                    ++colorCount;
                }
                ++l;
            }
            BUFFER_COLOR_STACK.emplace_back(colors[0], colors[1], colors[2], colors[3]);
            j = l - 1;
        } else if (text[j] == char(TextStringBuilderColorToken::End)) {
            BUFFER_COLOR_STACK.pop_back();
        } else if (text[j] != '\0') {
            const uint32_t accum   = w * 4;
            ++w;
            const CharGlyph& glyph = font.getGlyphData(text[j]);
            const float startingY  = y - (glyph.height + glyph.yoffset);
            const float startingX  = x + glyph.xoffset;
            x += float(int(glyph.xadvance));
            internal_add_quad(m_Text_Indices, accum);
            internal_add_uvs(m_Text_UVs, glyph);
            internal_add_colors(m_Text_Colors, BUFFER_COLOR_STACK.back());
            internal_add_positions(m_Text_Points, startingX + lineSize, startingY, z, glyph);
        }
        ++j;
    }
    y += newLineGlyphHeight;
    x = 0.0f;
}
void Engine::priv::TextRenderer::internal_render_text_left(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    uint32_t w = 0;
    internal_process_line(text, 0, int(text.size()), x, y, z, font, newLineGlyphHeight, w, 0);
}
void Engine::priv::TextRenderer::internal_render_text_center(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    internal_process_newline_chars(text, font, x);
    uint32_t w = 0;
    //first line
    internal_process_line(text, 0, BUFFER_NEW_LINE_INDICES[0].index, x, y, z, font, newLineGlyphHeight, w, -BUFFER_NEW_LINE_INDICES[0].lineHeight / 2);
    //rest lines
    for (int t = 1; t < BUFFER_NEW_LINE_INDICES.size(); ++t) {
        internal_process_line(text, BUFFER_NEW_LINE_INDICES[t - 1].index + 1, BUFFER_NEW_LINE_INDICES[t].index, x, y, z, font, newLineGlyphHeight, w, -BUFFER_NEW_LINE_INDICES[t].lineHeight / 2);
    }
}
void Engine::priv::TextRenderer::internal_render_text_right(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    internal_process_newline_chars(text, font, x);
    uint32_t w = 0;
    //first line
    internal_process_line(text, 0, BUFFER_NEW_LINE_INDICES[0].index, x, y, z, font, newLineGlyphHeight, w, -BUFFER_NEW_LINE_INDICES[0].lineHeight);
    //rest lines
    for (int t = 1; t < BUFFER_NEW_LINE_INDICES.size(); ++t) {
        internal_process_line(text, BUFFER_NEW_LINE_INDICES[t - 1].index + 1, BUFFER_NEW_LINE_INDICES[t].index, x, y, z, font, newLineGlyphHeight, w, -BUFFER_NEW_LINE_INDICES[t].lineHeight);
    }
}
void Engine::priv::TextRenderer::renderText(Mesh& fontPlane, RenderModule& renderModule, const std::string& text, Font& font, const glm::vec4& color, TextAlignment textAlignment, float& x, float& y, float z) {
    m_Text_Points.clear();
    m_Text_UVs.clear();
    m_Text_Colors.clear();
    m_Text_Indices.clear();

    BUFFER_COLOR_STACK.emplace_back(color * 255.0f);

    renderModule.bind(&fontPlane);

    const float newLineGlyphHeight = font.getMaxHeight() + font.getLineHeight();

    if (textAlignment == TextAlignment::Left) {
        internal_render_text_left(text, font, -newLineGlyphHeight, x, y, z);
    } else if (textAlignment == TextAlignment::Right) {
        internal_render_text_right(text, font, -newLineGlyphHeight, x, y, z);
    } else if (textAlignment == TextAlignment::Center) {
        internal_render_text_center(text, font, -newLineGlyphHeight, x, y, z);
    }
    fontPlane.modifyVertices(0, m_Text_Points.data(), m_Text_Points.size(),  MeshModifyFlags::None); //prevent gpu upload until after all the data is collected
    fontPlane.modifyVertices(1, m_Text_UVs.data(),    m_Text_UVs.size(),     MeshModifyFlags::None); //prevent gpu upload until after all the data is collected
    fontPlane.modifyVertices(2, m_Text_Colors.data(), m_Text_Colors.size(),  MeshModifyFlags::UploadToGPU);
    fontPlane.modifyIndices(   m_Text_Indices.data(), m_Text_Indices.size(), MeshModifyFlags::UploadToGPU);

    BUFFER_COLOR_STACK.pop_back();
}