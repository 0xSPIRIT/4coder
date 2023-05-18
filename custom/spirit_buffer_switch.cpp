#include "spirit_buffer_switch.h"

Spirit_View_Buffer_Storage *find_switcher(View_ID view) {
    for (int i = 0; i < spirit_switcher_count; i++) {
        if (spirit_switcher[i].view == view)
            return &spirit_switcher[i];
    }
    return NULL;
}

void spirit_push_buffer(Application_Links *app, Buffer_ID id) {
    View_ID view = get_active_view(app, Access_Always);

    Spirit_View_Buffer_Storage *a = find_switcher(view);
    if (a == NULL) {
        a = &spirit_switcher[spirit_switcher_count++];
        a->view = view;
    }
    
    if (id == a->current_buffer) return;
    
    a->previous_buffer = a->current_buffer;
    a->current_buffer = id;
}

function void
spirit_generate_all_buffers_list(Application_Links *app, Lister *lister) {
    lister_begin_new_item_set(app, lister);
    
    View_ID view = get_active_view(app, Access_Always);
    Spirit_View_Buffer_Storage *a = find_switcher(view);
    
    if (a == NULL) {
        a = &spirit_switcher[spirit_switcher_count++];
        a->view = view;
    }

    if (a->previous_buffer)
        generate_all_buffers_list__output_buffer(app, lister, a->previous_buffer);
#if 0
    if (a->current_buffer)
        generate_all_buffers_list__output_buffer(app, lister, a->current_buffer);
#endif
    
    // Place all normal buffers into the list.
    for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
         buffer != 0;
         buffer = get_buffer_next(app, buffer, Access_Always))
    {
        if (!buffer_has_name_with_star(app, buffer) && buffer != a->previous_buffer)
            generate_all_buffers_list__output_buffer(app, lister, buffer);
    }
    
    // Buffers starting with *
    for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
         buffer != 0;
         buffer = get_buffer_next(app, buffer, Access_Always))
    {
        if (buffer_has_name_with_star(app, buffer) && buffer != a->previous_buffer)
            generate_all_buffers_list__output_buffer(app, lister, buffer);
    }
}