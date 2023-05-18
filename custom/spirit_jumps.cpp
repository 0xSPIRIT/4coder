// Redoing the jumps that the vim layer provides since that's dogshit.

#include "spirit_jumps.h"

function void spirit_go_to_jump(Application_Links *app, Spirit_Jump_Point point) {
    view_set_active(app, point.view);
    view_set_buffer(app, point.view, point.loc.buffer_id, 0);
    
    Buffer_Seek seek;
    seek.type = buffer_seek_pos;
    seek.pos = point.loc.pos;
    
    view_set_buffer_scroll(app, point.view, point.scroll, SetBufferScroll_NoCursorChange);
    
    view_set_cursor(app, point.view, seek);
}

function void spirit_push_jump(Application_Links *app, View_ID view) {
    Spirit_Jump_Point point;
    
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    i64 cursor = view_get_cursor_pos(app, view);
    
    point.loc = {buffer, cursor};
    point.view = view;
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view);   
    point.scroll = scroll;
    
    jump_stack.points[jump_stack.curr++] = point;
}

CUSTOM_UI_COMMAND_SIG(spirit_pop_jump) {
    Spirit_Jump_Point point;
    
    if (jump_stack.curr == 0) return;
    
    point = jump_stack.points[--jump_stack.curr];
    
    spirit_go_to_jump(app, point);
}

CUSTOM_UI_COMMAND_SIG(spirit_undo_pop_jump) {
    if (jump_stack.points[jump_stack.curr].view == 0) return;
    if (jump_stack.curr == MAX_JUMPS) return;
    
    spirit_go_to_jump(app, jump_stack.points[jump_stack.curr]);
    jump_stack.curr++;
}