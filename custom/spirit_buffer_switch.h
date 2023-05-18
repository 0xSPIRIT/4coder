#pragma once

// We have two buffer IDs stored for each view.

struct Spirit_View_Buffer_Storage {
    View_ID view;
    Buffer_ID current_buffer, previous_buffer;
};

global Spirit_View_Buffer_Storage spirit_switcher[1000] = {0};
global i32 spirit_switcher_count = 0;

void spirit_push_buffer(Application_Links *app, Buffer_ID id);
function void spirit_generate_all_buffers_list(Application_Links *app, Lister *lister);
    