// Reference: https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c and https://freetype.org/freetype2/docs/tutorial/step1.html
#include "Font.hpp"
#include "gl_errors.hpp"

Font::Font(std::string const &font_path, int font_size, float line_height) {
    this->font_size = font_size;
    this->line_height = line_height;

    const char *fontfile = font_path.c_str();
    FT_Error ft_error;

    /* Initialize FreeType and create FreeType font face. */
    if ((ft_error = FT_Init_FreeType (&ft_library))) {
        std::cerr << "Error initializing FreeType library" << std::endl;
        abort();
    }
    if ((ft_error = FT_New_Face (ft_library, fontfile, 0, &ft_face))) {
        std::cerr << "Error loading font file: " << font_path << std::endl;
        abort();
    }
    if ((ft_error = FT_Set_Char_Size (ft_face, font_size*64, font_size*64, 0, 0))) {
        std::cerr << "Error setting character size" << std::endl;
        abort();
    }

    /* Create hb-ft font. */
    hb_font = hb_ft_font_create (ft_face, NULL);
    hb_buffer = hb_buffer_create();
}

Font::~Font() {
    hb_font_destroy(hb_font);
    hb_buffer_destroy(hb_buffer);
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);
}

void Font::load_text(std::string const &text) {
    hb_buffer_clear_contents(hb_buffer);
    // hb_buffer_reset(hb_buffer);
    hb_buffer_add_utf8 (hb_buffer, text.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties (hb_buffer);
    // hb_buffer_set_direction(hb_buffer, HB_DIRECTION_LTR);
    // hb_buffer_set_script(hb_buffer, HB_SCRIPT_COMMON);
    // hb_buffer_set_language(hb_buffer, hb_language_from_string("en", -1));
    // hb_font_set_scale(hb_font, font_size * 64, font_size * 64);

    /* Shape it! */
    hb_shape (hb_font, hb_buffer, NULL, 0);
}

void Font::load_glyph(hb_codepoint_t gid) {
    /* load glyph image into the slot (erase previous one) */
    FT_Error ft_error;
    FT_GlyphSlot slot = ft_face->glyph;

    if((ft_error = FT_Load_Glyph(ft_face, 
    gid, // the glyph_index in the font file
    FT_LOAD_DEFAULT))) {
        return;
    }
    if((ft_error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL))) {
        return;
    }
}

