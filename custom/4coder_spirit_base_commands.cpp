function void string_copy(u8 *dst, const char *src) {
    while (1) {
        *dst++ = *src++;
        if (*src == 0) return;
    }
}

CUSTOM_COMMAND_SIG(spirit_toggle_poscontext) {
    global_show_poscontext = !global_show_poscontext;
}

CUSTOM_COMMAND_SIG(vim_page_up){
    vim_push_jump(app, get_active_view(app, Access_ReadVisible));
    page_up(app);
}

CUSTOM_COMMAND_SIG(vim_page_down){
    vim_push_jump(app, get_active_view(app, Access_ReadVisible));
    page_down(app);
}

CUSTOM_COMMAND_SIG(spirit_swap_insert_cursor_type)
CUSTOM_DOC("Queries for a system command, runs the system command as a CLI and prints the output to the a new buffer."){
    if (spirit_insert_cursor_type == SPIRIT_INSERT_CURSOR_LINE) {
        spirit_insert_cursor_type = SPIRIT_INSERT_CURSOR_BLOCK;
    } else {
        spirit_insert_cursor_type = SPIRIT_INSERT_CURSOR_LINE;
    }
}

CUSTOM_COMMAND_SIG(spirit_execute_cli)
CUSTOM_DOC("Queries for a system command, runs the system command as a CLI and prints the output to the a new buffer."){
    Scratch_Block scratch(app);
    Query_Bar_Group group(app);
    
    memset(out_buffer_space, 0, sizeof(out_buffer_space));
    
    string_copy(out_buffer_space, "*out*");
    
    Query_Bar bar_cmd = {};
    bar_cmd.prompt = string_u8_litexpr("Command: ");
    bar_cmd.string = SCu8(command_space, (u64)0);
    bar_cmd.string_capacity = sizeof(command_space);
    if (!query_user_string(app, &bar_cmd)) return;
    bar_cmd.string.size = clamp_top(bar_cmd.string.size, sizeof(command_space) - 1);
    command_space[bar_cmd.string.size] = 0;
    
    String_Const_u8 hot = push_hot_directory(app, scratch);
    {
        u64 size = clamp_top(hot.size, sizeof(hot_directory_space));
        block_copy(hot_directory_space, hot.str, size);
        hot_directory_space[hot.size] = 0;
    }
    
    execute_previous_cli(app);
}

CUSTOM_COMMAND_SIG(spirit_home)
{
    seek_beginning_of_line(app);
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view);
    scroll.target.pixel_shift.x = 0;
    view_set_buffer_scroll(app, view, scroll, SetBufferScroll_NoCursorChange);
}

CUSTOM_COMMAND_SIG(spirit_close_compilation)
CUSTOM_DOC("Closes fleury's compilation view. Use Ctrl+Shift+M to toggle back!")
{
    view_set_split_pixel_size(app, global_compilation_view, 0);
    global_compilation_view_expanded = 1;
}

CUSTOM_COMMAND_SIG(spirit_transpose_chars)
CUSTOM_DOC("Transpose two characters at cursor.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    
    i64 cursor = view_get_cursor_pos(app, view);
    
    Range_i64 range = {cursor-1, cursor+1};
    
    u8 out[3] = {0};
    buffer_read_range(app, buffer, range, out);
    
    u8 temp = out[1];
    out[1] = out[0];
    out[0] = temp;
    
    buffer_replace_range(app, buffer, range, SCu8(out));
}

CUSTOM_COMMAND_SIG(spirit_open_goodies)
CUSTOM_DOC("Open goodies file.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    String_Const_u8 goodies_file = string_u8_litexpr("D:/Notes/goodies.txt");
    set_hot_directory(app, goodies_file);
    Buffer_ID buffer = create_buffer(app, goodies_file, 0);
    if (buffer != 0){
        view_set_buffer(app, view, buffer, 0);
    }
}

CUSTOM_COMMAND_SIG(spirit_open_url_line)
CUSTOM_DOC("Open URL at line.")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID prev_buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 cursor = view_get_cursor_pos(app, view);
    i64 cursor_line = get_line_number_from_pos(app, prev_buffer, cursor);
    
    String_Const_u8 out_buffer = SCu8(out_buffer_space);
    String_Const_u8 hot_directory = SCu8(hot_directory_space);
    
    Scratch_Block scratch(app);
    String_Const_u8 linestr = push_buffer_line(app, scratch, prev_buffer, cursor_line);
    
    Arena arena = make_arena_system();
    String_Const_u8 command = push_u8_stringf(&arena, "%s -incognito \"%s\"",
                                              "d:/dev/useful/chrome.bat",
                                              linestr.str);
    
    for (u64 i = 0; i < command.size; i++) {
        if (command.str[i] == '\n' || command.str[i] == '\r' || command.str[i] == '\t') {
            command.str[i] = 0;
            
            command.size--;
            for (u64 j = i; j < command.size; j++) {
                command.str[j] = command.str[j+1];
            }
            command.str[command.size] = 0;
        }
    }
    
    for (u64 i = 0; i < command.size; i++) {
        command_space[i] = command.str[i];
    }

    exec_system_command(app, view, buffer_identifier(string_u8_litexpr("*out*")),
                        string_u8_litexpr("D:/Notes/"), command, 
                        CLI_OverlapWithConflict|CLI_CursorAtEnd|CLI_SendEndSignal);
    
    view_set_buffer(app, view, prev_buffer, 0);
}

CUSTOM_UI_COMMAND_SIG(spirit_open_search)
CUSTOM_DOC("Switches to the search buffer in another panel")
{
    // ... 
}

CUSTOM_UI_COMMAND_SIG(spirit_open_project)
CUSTOM_DOC("Open a project by navigating to the project file.")
{
    for(;;)
    {
        Scratch_Block scratch(app);
        View_ID view = get_this_ctx_view(app, Access_Always);
        File_Name_Result result = get_file_name_from_user(app, scratch, "Open Project:", view);
        if (result.canceled) break;
        
        String_Const_u8 file_name = result.file_name_activated;
        if (file_name.size == 0)
        {
            file_name = result.file_name_in_text_field;
        }
        if (file_name.size == 0) break;
        
        String_Const_u8 path = result.path_in_text_field;
        String_Const_u8 full_file_name = push_u8_stringf(scratch, "%.*s/%.*s",
                                                         string_expand(path), string_expand(file_name));
        
        if (result.is_folder)
        {
            set_hot_directory(app, full_file_name);
            continue;
        }
        
        if(character_is_slash(file_name.str[file_name.size - 1]))
        {
            File_Attributes attribs = system_quick_file_attributes(scratch, full_file_name);
            if (HasFlag(attribs.flags, FileAttribute_IsDirectory)){
                set_hot_directory(app, full_file_name);
                continue;
            }
            if (string_looks_like_drive_letter(file_name)){
                set_hot_directory(app, file_name);
                continue;
            }
            if (query_create_folder(app, file_name)){
                set_hot_directory(app, full_file_name);
                continue;
            }
            break;
        }
        
        set_hot_directory(app, full_file_name);
        load_project(app);
        break;
    }
}

CUSTOM_UI_COMMAND_SIG(close_other_panel)
CUSTOM_DOC("Closes the other panel.")
{
    change_active_panel(app);
    close_panel(app);
}
