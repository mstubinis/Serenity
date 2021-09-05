#include <serenity/renderer/text/TextRenderer.h>
#include <serenity/system/Engine.h>

namespace {
    std::vector<std::pair<uint32_t, uint16_t>> BufferNewLineIndices;

    void internal_process_newline_chars(std::string_view& text, const Font& font, float& x) {
        x = 0.0f;
        if (text.back() == '\n') { //do not count the very last char if it is a newline
            text.remove_suffix(1);
        }
        BufferNewLineIndices.clear();
        for (uint32_t j = 0; j < text.size(); ++j) {
            if (text[j] == '\n') {
                BufferNewLineIndices.emplace_back(j, static_cast<uint16_t>(x));
                x = 0.0f;
            } else if (text[j] != '\0') {
                const CharGlyph& chr = font.getGlyphData(text[j]);
                x += static_cast<float>(chr.xadvance);
            }
        }
        //add the last block
        BufferNewLineIndices.emplace_back(text.size(), static_cast<uint16_t>(x));
        x = 0.0f;
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
}

void Engine::priv::TextRenderer::internal_render_text_left(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    uint32_t i = 0;
    for (const char character : text) {
        if (character == '\n') {
            y += newLineGlyphHeight;
            x = 0.0f;
        } else if (character != '\0') {
            const uint32_t accum   = i * 4;
            const CharGlyph& glyph = font.getGlyphData(character);
            const float startingY  = y - (glyph.height + glyph.yoffset);
            ++i;

            const float startingX  = x + glyph.xoffset;
            x += glyph.xadvance;
            internal_add_quad(m_Text_Indices, accum);
            internal_add_positions(m_Text_Points, startingX, startingY, z, glyph);
            internal_add_uvs(m_Text_UVs, glyph);
        }
    }
}

void Engine::priv::TextRenderer::internal_render_text_center(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    internal_process_newline_chars(text, font, x);
    x = 0.0f;
    uint32_t i = 0;
    auto process_line = [&text, this, &font, &i, &x, &y, z, newLineGlyphHeight](int startIndex, int endIndex, int lineSize) {
        for (int j = startIndex; j <= endIndex; ++j) {
            const char character = text[j];
            if (character != '\0') {
                const uint32_t accum   = i * 4;
                const CharGlyph& glyph = font.getGlyphData(character);
                const float startingY  = y - (glyph.height + glyph.yoffset);
                ++i;

                const float startingX  = x + glyph.xoffset;
                x += glyph.xadvance;
                internal_add_quad(m_Text_Indices, accum);
                internal_add_positions(m_Text_Points, startingX - lineSize, startingY, z, glyph);
                internal_add_uvs(m_Text_UVs, glyph);
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    };
    //first line
    process_line(0, int(BufferNewLineIndices[0].first) - 1, int(BufferNewLineIndices[0].second / 2));
    //rest lines
    for (int t = 1; t < BufferNewLineIndices.size(); ++t) {
        process_line(int(BufferNewLineIndices[t - 1].first) + 1, int(BufferNewLineIndices[t].first) - 1, int(BufferNewLineIndices[t].second / 2));
    }
}

void Engine::priv::TextRenderer::internal_render_text_right(std::string_view text, const Font& font, float newLineGlyphHeight, float& x, float& y, float z) {
    internal_process_newline_chars(text, font, x);
    int k = 0;
    uint32_t i = 0;
    auto process_line = [&text, this, &font , &i, &x, &y, z, &k, newLineGlyphHeight](int startIndex, int endIndex) {
        k = 0;
        for (int j = startIndex; j >= endIndex; --j) {
            const char character = text[j];
            if (character != '\0') {
                const uint32_t accum = i * 4;
                ++i;
                const CharGlyph& glyph = font.getGlyphData(character);
                const float startingY = y - (glyph.height + glyph.yoffset);

                if (k == 0) {
                    x -= glyph.width;
                }
                const float startingX = x + glyph.xoffset;
                x -= glyph.xadvance;
                internal_add_quad(m_Text_Indices, accum);
                internal_add_positions(m_Text_Points, startingX, startingY, z, glyph);
                internal_add_uvs(m_Text_UVs, glyph);
                ++k;
            }
        }
        y += newLineGlyphHeight;
        x = 0.0f;
    };
    //first line
    process_line(int(BufferNewLineIndices[0].first) - 1, 0);
    //rest lines
    for (int t = 1; t < BufferNewLineIndices.size(); ++t) {
        process_line(int(BufferNewLineIndices[t].first) - 1, int(BufferNewLineIndices[t-1].first) + 1);
    }
}
void Engine::priv::TextRenderer::renderText(Mesh& fontPlane, RenderModule& renderModule, const std::string& text, Font& font, TextAlignment textAlignment, float& x, float& y, float z) {
    m_Text_Points.clear();
    m_Text_UVs.clear();
    m_Text_Indices.clear();

    renderModule.bind(&fontPlane);

    const float newLineGlyphHeight = font.getMaxHeight() + font.getLineHeight();

    if (textAlignment == TextAlignment::Left) {
        internal_render_text_left(text, font, -newLineGlyphHeight, x, y, z);
    } else if (textAlignment == TextAlignment::Right) {
        internal_render_text_right(text, font, -newLineGlyphHeight, x, y, z);
    } else if (textAlignment == TextAlignment::Center) {
        internal_render_text_center(text, font, -newLineGlyphHeight, x, y, z);
    }
    fontPlane.modifyVertices(0, m_Text_Points.data(), m_Text_Points.size(), MeshModifyFlags::None); //prevent gpu upload until after all the data is collected
    fontPlane.modifyVertices(1, m_Text_UVs.data(), m_Text_UVs.size(), MeshModifyFlags::UploadToGPU);
    fontPlane.modifyIndices(m_Text_Indices.data(), m_Text_Indices.size(), MeshModifyFlags::UploadToGPU);
}