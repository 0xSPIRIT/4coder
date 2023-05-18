#include "spirit_search.h"

function void string_copy(u8 *dst, const char *src);

function void spirit_draw_search_highlight(Application_Links *app, View_ID view, Buffer_ID buffer, Text_Layout_ID text_layout_id, f32 roundness){
	//strcpy(spirit_search_query, "::");
	char pattern[1024] = {0};
	strcpy(pattern, spirit_search_query);
	u64 size = 1;
    
	if(size == 0){ return; }
	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
	//i64 buffer_size = buffer_get_size(app, buffer);
	i64 cur_pos = 0;
	while(cur_pos < spirit_stored_buffer_size){
		i64 new_pos = 0;
		String_Const_u8 s;
		s.str = (u8*)pattern;
		s.size = size;
		seek_string_forward(app, buffer, cur_pos, 0, s, &new_pos);
		if(new_pos == 0 || new_pos == spirit_stored_buffer_size){ break; }
		else{
			cur_pos = new_pos;
			Rect_f32 rect = text_layout_character_on_screen(app, text_layout_id, cur_pos);
			rect.x1 = rect.x0 + size*(rect_width(rect));
			draw_rectangle_fcolor(app, rect, roundness, fcolor_id(defcolor_highlight));
		}
	}
}

