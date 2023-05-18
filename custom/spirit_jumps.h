#pragma once

struct Spirit_Jump_Point {
    ID_Pos_Jump_Location loc;
    View_ID view;
    Buffer_Scroll scroll;
};

#define MAX_JUMPS 8192

struct Spirit_Jump_Stack {
    Spirit_Jump_Point points[MAX_JUMPS];
    int curr;
};

global Spirit_Jump_Stack jump_stack = {0};

function void spirit_push_jump(Application_Links *app, View_ID view);
