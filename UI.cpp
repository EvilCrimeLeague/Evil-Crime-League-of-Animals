#include "UI.hpp"
#include "TextTextureProgram.hpp"
#include "gl_errors.hpp"

void UI::draw_texture(unsigned int texture) {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glUseProgram(text_texture_program->program);

	glBindVertexArray(text_texture_program->VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	GL_ERRORS();
}

void UI::draw(){
    draw_texture(box_texture);
    draw_texture(text_texture);
    draw_texture(img_texture);
}

void UI::gen_box_texture(){
    // initialize image. Origin is the upper left corner
	glm::u8vec4 image[height][width];
    for (int i = 0; i < height; i++ ){
        for (int j = 0; j < width; j++ ) {
            image[i][j] = glm::u8vec4(0x00);
        }    
    }

	for(auto box : boxes){
		// fill box area with color
        for (int i = box->rect[0]; i < box->rect[2]; i++ ){
            for (int j = box->rect[1]; j < box->rect[3]; j++ ) {
                image[j][i] = box->color;
            }    
        }
	}

    // create texture
	glm::u8vec4* data = new glm::u8vec4[width*height];
    int k = 0;
    for (int i = height-1; i >= 0; i-- ){
        for (int j = 0; j < width; j++ )
            data[k++] = image[i][j];
    }

    glBindTexture(GL_TEXTURE_2D, box_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    GL_ERRORS();

    delete[] data;
}

void UI::gen_text_texture() {
    /* Initialize image. Origin is the upper left corner */
    glm::u8vec4 image[height][width];
    for (int i = 0; i < height; i++ ){
        for (int j = 0; j < width; j++ ) {
            image[i][j] = glm::u8vec4(0x00);
        }    
    }
    
    FT_GlyphSlot slot;

    auto draw_bitmap = [&image,this](FT_Bitmap*  bitmap, FT_Int x, FT_Int y, glm::u8vec3 color) {
        FT_Int  i, j, p, q;
        FT_Int  x_max = x + bitmap->width;
        FT_Int  y_max = y + bitmap->rows;
        // y = y - bitmap->rows;

        for ( i = x, p = 0; i < x_max; i++, p++ )
        {
            for ( j = y, q = 0; j < y_max; j++, q++ )
            {
                if ( i < 0      || j < 0       ||
                    i >= width || j >= height )
                    continue;

                image[j][i][3] = bitmap->buffer[q * bitmap->width + p];
                image[j][i] = glm::u8vec4(color, image[j][i][3]);
            }
        }
    };

    for(auto& text: texts) {
        slot = text->font->ft_face->glyph;
        hb_buffer_t *hb_buffer = text->font->hb_buffer;

        // break text by newlines
        std::vector<std::string> lines = wrapText(text->text, text->line_length);

        double current_x = text->start_pos.x;
        double current_y = text->start_pos.y;

        for(std::string& line: lines) {
            /* Create hb-buffer and populate. */
            text->font->load_text(line);

            /* Get glyph information and positions out of the buffer. */
            unsigned int len = hb_buffer_get_length(hb_buffer);
            hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
            hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

            /* Converted to absolute positions and draw the glyph to image array */
            {   
                for (unsigned int i = 0; i < len; i++) {   
                    // calculate position
                    hb_codepoint_t gid = info[i].codepoint;
                    // unsigned int cluster = info[i].cluster;
                    // char glyphname[32];
                    // hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));
                    text->font->load_glyph(gid);

                    double x_position = current_x + pos[i].x_offset / 64. + slot->bitmap_left;
                    double y_position = current_y + pos[i].y_offset / 64. - slot->bitmap_top;
                    // printf ("glyph='%s'	bitmap_top=%d	bitmap_left=%d  current_pos = (%f,%f)  y0=%f y1=%f rows=%d\n",
                    //     glyphname, slot->bitmap_top, slot->bitmap_left, current_x, current_y, 
                    //     y_position, 
                    //     current_y + pos[i].y_offset / 64. + slot->bitmap_top, 
                    //     slot->bitmap.rows);

                    current_x += pos[i].x_advance / 64.;
                    current_y += pos[i].y_advance / 64.;

                    draw_bitmap( &slot->bitmap,
                                x_position,
                                y_position,
                                text->color);
                    
                }
            }
            current_x = text->start_pos.x;
            current_y += text->font->line_height;
        }
    }
    

    // {
    //     // show image
    //     int  i, j;
    //     for ( i = 0; i < height; i++ )
    //     {
    //         for ( j = 0; j < width; j++ )
    //             putchar( image[i][j] == 0 ? ' ' : image[i][j] < 128 ? '+' : '*' );
    //         putchar( '\n' );
    //     }
    // }

    glm::u8vec4* data = new glm::u8vec4[width*height];
    int k = 0;
    for (int i = height-1; i >= 0; i-- ){
        for (int j = 0; j < width; j++ )
            data[k++] = image[i][j];
    }

    glBindTexture(GL_TEXTURE_2D, text_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    GL_ERRORS();

    delete[] data;
}

std::vector<std::string> UI::wrapText(const std::string& text, size_t line_length) {
    std::vector<std::string> lines;
    size_t start = 0;
    
    while (start < text.length()) {
        // Find the position to break the line
        size_t end = start + line_length;
        
        if (end >= text.length()) {
            lines.push_back(text.substr(start));
            break;
        }

        // Find the last space within the line length to avoid cutting words
        size_t spacePos = text.rfind(' ', end);

        if (spacePos != std::string::npos && spacePos > start) {
            lines.push_back(text.substr(start, spacePos - start));
            start = spacePos + 1; // Move to the next word
        } else {
            // If no space is found, break at the line length (potential word split)
            lines.push_back(text.substr(start, line_length));
            start += line_length;
        }
    }

    return lines;
}

void UI::gen_img_texture() {
    glm::u8vec4 image[height][width];
    for (int i = 0; i < height; i++ ){
        for (int j = 0; j < width; j++ ) {
            image[i][j] = glm::u8vec4(0x00);
        }    
    }
    
    for(auto img: imgs) {
        assert(img->size.x <= width && img->size.y <= height && "Image size exceeds texture size");

        for (int i = img->pos.y; i < img->size.y+img->pos.y && i < height; i++ ){
            for (int j = img->pos.x; j < img->size.x+img->pos.x && j < width; j++ ) {
                image[i][j] = img->data[i*img->size.x + j];
            }    
        }
    }

    glm::u8vec4* data = new glm::u8vec4[width*height];
    int k = 0;
    for (int i = height-1; i >= 0; i-- ){
        for (int j = 0; j < width; j++ )
            data[k++] = image[i][j];
    }

    glBindTexture(GL_TEXTURE_2D, img_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL_ERRORS();
}