function Buffer_Kill_Result spirit_buffer_kill(Application_Links* app,
                                               Buffer_ID buffer_id,
                                               Buffer_Kill_Flag flags)
{
    for (int i = 0; i < spirit_switcher_count; i++) {
        if (spirit_switcher[i].current_buffer == buffer_id) {
            spirit_switcher[i].current_buffer = 0;
        }
        if (spirit_switcher[i].previous_buffer == buffer_id) {
            spirit_switcher[i].previous_buffer = 0;
        }
    }
    return buffer_kill(app, buffer_id, flags);
}