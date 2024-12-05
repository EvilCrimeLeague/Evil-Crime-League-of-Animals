#include "UI.hpp"
#include "TextTextureProgram.hpp"
#include "gl_errors.hpp"
#include "Sound.hpp"


Load< Sound::Sample > pop2_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("pop2.wav"));
});

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
    draw_texture(img_texture);
    draw_texture(text_texture);
}

void UI::gen_box_texture(){
    // initialize image. Origin is the upper left corner
    std::vector<std::vector<glm::u8vec4>> image(height, std::vector<glm::u8vec4>(width));
	// glm::u8vec4 image[height][width];
    for (int i = 0; i < height; i++ ){
        for (int j = 0; j < width; j++ ) {
            image[i][j] = glm::u8vec4(0x00);
        }    
    }

	for(auto box : boxes){
        if(box->hide) continue;
		// fill box area with color
        for (uint32_t i = (uint32_t)box->rect[0]; i < (uint32_t)box->rect[2]; i++ ){
            for (uint32_t j = (uint32_t)box->rect[1]; j < (uint32_t)box->rect[3]; j++ ) {
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
    std::vector<std::vector<glm::u8vec4> > image(height, std::vector<glm::u8vec4>(width));
    // glm::u8vec4 image[height][width];
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
        if(text->hide) continue;
        slot = text->font->ft_face->glyph;
        hb_buffer_t *hb_buffer = text->font->hb_buffer;

        // break text by newlines
        std::vector<std::string> lines = wrapText(text->text, (size_t)text->line_length);

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
                                (FT_Int) x_position,
                                (FT_Int) y_position,
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
    if(showing_highres_img) {
        width *= scale;
        height *= scale;
    }
    std::vector<std::vector<glm::u8vec4>> image(height, std::vector<glm::u8vec4>(width));
    // glm::u8vec4 image[height][width];
    for (int i = 0; i < height; i++ ){
        for (int j = 0; j < width; j++ ) {
            image[i][j] = glm::u8vec4(0x00);
        }    
    }

    for(auto img: imgs) {
        if(img->hide) continue;
        for (uint32_t i = 0; i < img->size.y; i++ ){
            for (uint32_t j = 0; j < img->size.x; j++ ) {
                if((*img->data)[i*img->size.x + j] == glm::u8vec4(0x00))    
                    continue;
                float y = i+img->pos.y;
                float x = j+img->pos.x;
                if(y < 0 || y >= height || x < 0 || x >= width) continue;
                image[(uint32_t)y][(uint32_t)x] = ((*img->data)[i*((uint32_t)img->size.x) + j]);
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

    if(showing_highres_img) {
        width /= scale;
        height /= scale;
    }
}

void UI::update_choice(bool left) {
    if(!showing_description) return;
    // switch to the next choice
    if(left) {
        choice_id = uint32_t(std::max(0, (int)choice_id - 1));
    } else {
        choice_id = uint32_t(std::min((int)choice_id+1, (int)(choice_pos.size()-1)));
    }
    enter_bt_img->pos = choice_pos[choice_id];
    need_update_texture = true;
}

void UI::update_texture() {
    if(need_update_texture) {
        gen_text_texture();
        gen_box_texture();
        gen_img_texture();
        need_update_texture = false;
    }
}

void UI::show_description(std::string description, std::string choice1, std::string choice2) {
    hide_all();
    showing_description = true;

    description_text->hide = false;
    description_text->text = description;
    description_box->hide = false;
    
    manual_text->hide = false;
    if(choice1 != "" && choice2 != "") {
        enter_bt_img->hide = false;
        slot_left_img->hide = false;
        slot_right_img->hide = false;
        choice1_text->hide = false;
        choice1_text->text = choice1;
        choice2_text->hide = false;
        choice2_text->text = choice2;
        showing_choices = true;
        manual_text->text = "Press <- or -> to select, press 'return' to continue";
        manual_text->start_pos = glm::vec2(680, 690);
    } else {
        showing_choices = false;
        manual_text->text = "Press 'return' to continue";
        manual_text->start_pos = glm::vec2(880, 690);
    }

    need_update_texture = true;
}

void UI::hide_description() {
    set_restart_button(/*hide=*/false);
    showing_description = false;

    description_text->hide = true;
    description_box->hide = true;
    enter_bt_img->hide = true;
    slot_left_img->hide = true;
    slot_right_img->hide = true;
    choice1_text->hide = true;
    choice2_text->hide = true;
    manual_text->hide = true;

    set_menu_button(false);
    set_inventory_button(false);
    set_restart_button(false);
    title_text->hide = false;

    need_update_texture = true;
}

void UI::show_game_over(bool won) {
    hide_all();
    // show game over screen
    if(won) {
        game_over_text->text = "You won!";
        game_over_text->color = glm::u8vec3(0, 255, 0);
    } else {
        game_over_text->text = "Game over";
        game_over_text->color = glm::u8vec3(255, 0, 0);
    }
    manual_text->text = "Press Enter to continue";
    manual_text->start_pos.x = 900;
    game_over_text->hide = false;
    game_over_box->hide = false;
    manual_text->hide = false;
    showing_game_over = true;
    need_update_texture = true;
}

void UI::hide_all() {
    for(auto text: texts) {
        text->hide = true;
    }

    for(auto box: boxes) {
        box->hide = true;
    }
    
    for(auto img: imgs) {
        img->hide = true;
    } 

    showing_description = false;
    showing_choices = false;
    showing_inventory = false;
    showing_interactable_button = false;
    showing_notification = false;
    showing_alarm = false;
    showing_inventory_description = false;
    showing_menu = false;
    showing_image = false;
    showing_game_over = false;
    showing_highres_img = false;
}

void UI::reset() {
    // reset UI when game restart
    hide_all();

    choice_id = 0;
    enter_bt_img->pos = choice_pos[choice_id];
    inventory_slot_selected_id = 0;
    manual_text->start_pos.x = 680;
    for(auto img: extra_imgs) {
        imgs.erase(std::remove(imgs.begin(), imgs.end(), img), imgs.end());
    }
    for(auto box: extra_boxes) {
        boxes.erase(std::remove(boxes.begin(), boxes.end(), box), boxes.end());
    }
    for(auto text: extra_texts) {
        texts.erase(std::remove(texts.begin(), texts.end(), text), texts.end());
    }
    inventory_items.clear();
    manual_text->text = "Press <- or -> to select, press 'return' to continue";

    set_inventory_button(/*hide=*/false);
    set_restart_button(/*hide=*/false);
    set_menu_button(/*hide=*/false);

    title_text->hide = false;

    need_update_texture = true;
}

void UI::set_interactable_button(bool hide) {
    // show interactable button if within range of an interactable object
    interact_bt_img->hide = hide;
    showing_interactable_button = !hide;
    need_update_texture = true;
}

void UI::set_inventory_button(bool hide) {
    inventory_bt_manual_text->hide = hide;
    inventory_bt_img->hide = hide;
    need_update_texture = true;
}

void UI::set_inventory(bool hide){
    set_inventory_button(!hide);
    inventory_img->hide = hide;
    slot_selected_img->hide = hide;
    slot_selected_img->pos = inventory_item_pos[inventory_slot_selected_id];
    inventory_manual_text->hide = hide;
    for(uint32_t i = inventory_slot_id_start; i <inventory_slot_id_start+inventory_slot_num; ++i) {
        imgs[i]->hide = hide;
    }
    for(auto& item: inventory_items) {
        item.img->hide = hide;
    }
    showing_inventory = !hide;
    if(showing_inventory) {
        set_inventory_button(true);
    }
    need_update_texture = true;
}

void UI::update_inventory_selection(bool left) {
    if(!showing_inventory) return;
    if(left) {
        inventory_slot_selected_id = (inventory_slot_selected_id - 1 + inventory_slot_num) % inventory_slot_num;
    } else {
        inventory_slot_selected_id = (inventory_slot_selected_id + 1) % inventory_slot_num;
    }
    slot_selected_img->pos = inventory_item_pos[inventory_slot_selected_id];
    need_update_texture = true;
}

void UI::arrow_key_callback(bool left) {
    if(showing_description) {
        update_choice(left);
        Sound::play(*pop2_sample, 0.1f, 0.0f);
    } else if(showing_inventory) {
        update_inventory_selection(left);
        Sound::play(*pop2_sample, 0.1f, 0.0f);
    } else if (showing_menu){
        update_menu_selection(left);
        Sound::play(*pop2_sample, 0.1f, 0.0f);
    }
}

void UI::add_inventory_item(std::string item_name, std::string img_path, std::string description_img_path) {
    auto img_ptr = std::make_shared<Img>(inventory_item_pos[inventory_items.size()], img_path);
    imgs.push_back(img_ptr);
    extra_imgs.push_back(img_ptr);
    InventoryItem item = {item_name, img_ptr, static_cast<uint32_t>(inventory_items.size()), nullptr};
    if(description_img_path != "") {
        item.description_img = std::make_shared<Img>(glm::vec2(0,0), description_img_path);
        item.description_img->pos = glm::vec2(width/2-item.description_img->size.x/2, height/2-item.description_img->size.y/2);
        imgs.push_back(item.description_img);
        extra_imgs.push_back(item.description_img);
    }
    inventory_items.push_back(item);
    img_ptr->hide = inventory_img->hide;
    inventory_slot_selected_id = (uint32_t)inventory_items.size()-1;
    set_inventory(false);
    need_update_texture = true;
}

void UI::remove_inventory_item() {
    std::string item_name = get_selected_inventory_item_name();

    imgs.erase(std::remove(imgs.begin(), imgs.end(), inventory_items[inventory_slot_selected_id].img), imgs.end());

    inventory_items.erase(std::remove_if(inventory_items.begin(), inventory_items.end(), [&item_name](InventoryItem& item){return item.name == item_name;}), inventory_items.end());

    // Reassign inventory slot
    for (uint32_t i = 0; i < inventory_items.size(); i++) {
        inventory_items[i].inventory_slot_id = i;
        inventory_items[i].img->pos = inventory_item_pos[i];
    }
}

std::string UI::get_selected_inventory_item_name() {
    if(inventory_slot_selected_id > inventory_items.size()) return "";
    return inventory_items[inventory_slot_selected_id].name;
}

void UI::show_inventory_description_img(uint32_t slot_id){
    hide_all();
    inventory_items[slot_id].description_img->hide = false;
    description_img_box->hide = false;
    showing_inventory_description = true;

    need_update_texture = true;
}

void UI::hide_inventory_description_img() {
    showing_inventory_description = false;
    showing_highres_img = false;
    description_img_box->hide = true;
    for(auto& item: inventory_items) {
        if(item.description_img != nullptr) {
            item.description_img->hide = true;
        }
    }
    set_menu_button(false);
    set_inventory_button(false);
    set_restart_button(false);
    title_text->hide = false;
    need_update_texture = true;
}

uint32_t UI::get_inventory_item_id(std::string item_name) {
    for(auto& item: inventory_items) {
        if(item.name == item_name) {
            return item.inventory_slot_id;
        }
    }
    throw std::runtime_error("Item "+item_name+" not found in inventory");
}

void UI::handle_inventory_selection(std::string description,std::vector<std::string> choices) {
    if(description != "") {
        if(choices.size() == 0) {
            show_description(description);
        } else {
            show_description(description, choices[0], choices[1]);
        }
        showing_inventory_description = true;
    } else {
        show_inventory_description_img(inventory_slot_selected_id);
    }
}

void UI::show_notification(std::string notification) {
    notification_text->text = notification;
    notification_text->hide = false;
    notification_box->hide = false;
    showing_notification = true;

    need_update_texture = true;
}

void UI::hide_notification() {
    notification_text->hide = true;
    notification_box->hide = true;
    showing_notification = false;

    need_update_texture = true;
}

void UI::set_alarm(bool hide) {
    alarm_img->hide = hide;
    showing_alarm = !hide;
    need_update_texture = true;
}

void UI::set_restart_button(bool hide) {
    restart_bt_img->hide = hide;
    restart_bt_manual_text->hide = hide;

    need_update_texture = true;
}

void UI::show_interact_bt_msg(std::string msg) {
    interact_bt_manual_text->text = msg;
    interact_bt_manual_text->hide = false;
    need_update_texture = true;
}

void UI::hide_interact_bt_msg() {
    interact_bt_manual_text->hide = true;
    need_update_texture = true;
}

void UI::set_menu(bool hide) {
    if(hide) {
        set_menu_button(false);
        set_restart_button(false);
        set_inventory_button(false);
        title_text->hide = false;
        showing_menu = false;
    } else {
        // show menu, hide other UI elements
        hide_all();
        showing_menu = true;
        slot_selected_img->pos = menu_item_pos[menu_slot_selected_id];
    }
    menu_img->hide = hide;
    slot_selected_img->hide = hide;
    for(uint32_t i = menu_slot_id_start; i <menu_slot_id_start+menu_slot_num; ++i) {
        imgs[i]->hide = hide;
    }

    need_update_texture = true;
}

void UI::update_menu_selection(bool left) {
    if(!showing_menu) return;
    if(left) {
        menu_slot_selected_id = uint32_t(std::max(0, (int)menu_slot_selected_id - 1));
    } else {
        menu_slot_selected_id = uint32_t(std::min((int)menu_slot_selected_id+1, (int)(menu_slot_num-1)));
    }
    slot_selected_img->pos = menu_item_pos[menu_slot_selected_id];

    need_update_texture = true;
}

void UI::set_menu_button(bool hide) {
    menu_bt_img->hide = hide;
    menu_bt_manual_text->hide = hide;
    need_update_texture = true;
}

void UI::set_title(std::string title) {
    title_text->text = title;
}

std::shared_ptr<UI::Img> UI::add_img(std::string path) {
    auto img_ptr = std::make_shared<Img>(glm::vec2(0,0), path);
    if(img_ptr->size.x > (float)width || img_ptr->size.y > (float)height) {
        // high res image
        img_ptr->pos = glm::vec2(width*scale/2-img_ptr->size.x/2, height*scale/2-img_ptr->size.y/2);
    } else {
        img_ptr->pos = glm::vec2(width/2-img_ptr->size.x/2, height/2-img_ptr->size.y/2);
    }
    
    imgs.push_back(img_ptr);
    extra_imgs.push_back(img_ptr);
    return img_ptr;
}

void UI::show_img(std::shared_ptr<Img> img, bool showing_highres) {
    hide_all();
    this->showing_highres_img = showing_highres;
    img->hide = false;
    if(img->size.x > img->size.y) {
        // landscape
        description_img_box->rect = glm::vec4(200, 100, 1080, 620);
    } else {
        // portrait
        description_img_box->rect = glm::vec4(400, 50, 880, 670);
    }
    manual_text->start_pos.x = 480;
    manual_text->text = "Press 'return' to continue";
    manual_text->hide = false;
    description_img_box->hide = false;
    showing_image = true;

    need_update_texture = true;
}

void UI::hide_img() {
    hide_all();
    set_menu_button(false);
    set_inventory_button(false);
    set_restart_button(false);
    manual_text->hide = true;
    title_text->hide = false;
    showing_image = false;

    need_update_texture = true;
}

bool UI::should_pause() {
    return showing_description || showing_image || showing_inventory_description || showing_menu;
}

std::shared_ptr<UI::Box> UI::add_box(glm::vec4 rect, glm::u8vec4 color) {
    auto box = std::make_shared<UI::Box>(rect, color);
    boxes.push_back(box);
    extra_boxes.push_back(box);
    return box;
}

std::shared_ptr<Text> UI::add_text(std::string text, glm::vec2 start_pos, std::shared_ptr<Font> font) {
    auto text_ptr = std::make_shared<Text>(text, /*line length*/85, start_pos, font);
    texts.push_back(text_ptr);
    extra_texts.push_back(text_ptr);
    return text_ptr;
}

void UI::show_game_start() {
    hide_all();
    game_start_img->hide = false;
    manual_text->hide = false;
    manual_text->text = "Press Enter to start";
    manual_text->start_pos = glm::vec2(960, 630);

    need_update_texture = true;
}