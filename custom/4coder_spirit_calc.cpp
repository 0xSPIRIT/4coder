////////////////////////////////
//// Simple calculator.
////////////////////////////////


////////////////////////////////
//// > Node Evaluation
////////////////////////////////

#define CALC_TOKEN_STRING_LENGTH 128

#include <string.h>
function void u8_strcpy(u8 *dest, const u8 *src) {
    u64 i = 0;
    while (true) {
        dest[i] = src[i];
        if (dest[i] == 0) break;
        ++i;
    }
}

enum Calc_Node_Type {
    CALC_NODE_ADD,
    CALC_NODE_SUB,
    CALC_NODE_MUL,
    CALC_NODE_DIV,
    CALC_NODE_MOD,
    CALC_NODE_CONST
};

struct Calc_Node {
    enum Calc_Node_Type type;
    double value;
    struct Calc_Node *childA;
    struct Calc_Node *childB;
};

function struct Calc_Node 
Calc_EvaluateNode(struct Calc_Node *node) {
    struct Calc_Node result = {};
    result.type = CALC_NODE_CONST;
    
    switch (node->type) {
        case CALC_NODE_CONST: {
            result.value = node->value;
            break;
        }
        case CALC_NODE_ADD: {
            result.value = Calc_EvaluateNode(node->childA).value + Calc_EvaluateNode(node->childB).value;
            break;
        }
        case CALC_NODE_SUB: {
            result.value = Calc_EvaluateNode(node->childA).value - Calc_EvaluateNode(node->childB).value;
            break;
        }
        case CALC_NODE_MUL: {
            result.value = Calc_EvaluateNode(node->childA).value * Calc_EvaluateNode(node->childB).value;
            break;
        }
        case CALC_NODE_DIV: {
            result.value = Calc_EvaluateNode(node->childA).value / Calc_EvaluateNode(node->childB).value;
            break;
        }
        case CALC_NODE_MOD: {
            result.value = fmod(Calc_EvaluateNode(node->childA).value, Calc_EvaluateNode(node->childB).value);
            break;
        }
    }
    
    return result;
}

////////////////////////////////
//// > Tokenizer
////////////////////////////////

struct Calc_Token {
    enum Calc_Node_Type type;
    u8 str[CALC_TOKEN_STRING_LENGTH];
    
    struct Calc_Token *next, *prev;
};

struct Calc_Tokenizer {
    struct Calc_Token *tok_start, *tok_curr;
    unsigned token_count;
};

function struct Calc_Node
Calc_TokenToNode(struct Calc_Token tok) {
    struct Calc_Node node = {};
    node.type = tok.type;
    node.value = atof((char*)tok.str);
    return node;
}

function struct Calc_Token*
Calc_PushToken(Arena *arena, 
               struct Calc_Tokenizer *tokenizer, 
               u8 str[CALC_TOKEN_STRING_LENGTH]) 
{
    struct Calc_Token *tok = push_array(arena, struct Calc_Token, 1);
    u8_strcpy(tok->str, str);
    
    switch (str[0]) {
        case '+': tok->type = CALC_NODE_ADD; break;
        case '-': tok->type = CALC_NODE_SUB; break;
        case '*': tok->type = CALC_NODE_MUL; break;
        case '/': tok->type = CALC_NODE_DIV; break;
        case '%': tok->type = CALC_NODE_MOD; break;
        default:  tok->type = CALC_NODE_CONST; break;
    }
    
    struct Calc_Token *previous_token = NULL;
    if (tokenizer->tok_curr) {
        previous_token = tokenizer->tok_curr;
    }
    
    tokenizer->tok_curr = tok;
    if (!tokenizer->tok_start) {
        tokenizer->tok_start = tok;
    }
    
    if (previous_token) {
        previous_token->next = tok;
        tok->prev = previous_token;
    }
    
    tokenizer->token_count++;
    
    return tok;
}

function b32 is_letter(u8 c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

function b32 is_whitespace(u8 c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

function struct Calc_Tokenizer
Calc_Tokenize(Arena *arena, String_Const_u8 str) {
    struct Calc_Tokenizer tokenizer = {};
    
    u8 cur_tok_str[CALC_TOKEN_STRING_LENGTH] = {0};
    u64 cur_tok_len = 0;
    
    u8 *cstr = str.str;
    
    for (u64 i = 0; i < str.size; i++) {
        if (is_whitespace(cstr[i])) {
            if (cur_tok_len > 0) {
                // We assume that the type must be a constant.
                Calc_PushToken(arena, &tokenizer, cur_tok_str);
                
                memset(cur_tok_str, 0, CALC_TOKEN_STRING_LENGTH);
                cur_tok_len = 0;
            }
            continue;
        }
        
        if (cstr[i] == '+' || cstr[i] == '-' || cstr[i] == '*' || 
            cstr[i] == '/') {
            if (cur_tok_len > 0) {
                // We assume that the type must be a constant.
                Calc_PushToken(arena, &tokenizer, cur_tok_str);
                
                memset(cur_tok_str, 0, CALC_TOKEN_STRING_LENGTH);
                cur_tok_len = 0;
            }
            
            u8 char_str[CALC_TOKEN_STRING_LENGTH] = {cstr[i], 0};
            Calc_PushToken(arena, &tokenizer, char_str);
            
            memset(cur_tok_str, 0, CALC_TOKEN_STRING_LENGTH);
            cur_tok_len = 0;
        } else if (!is_letter(cstr[i])) {
            cur_tok_str[cur_tok_len++] = cstr[i];
        }
    }

    if (cur_tok_len > 0) {
        // We assume that the type must be a constant.
        Calc_PushToken(arena, &tokenizer, cur_tok_str);

        memset(cur_tok_str, 0, CALC_TOKEN_STRING_LENGTH);
        cur_tok_len = 0;
    
    }
    return tokenizer;
}

function struct Calc_Node
Calc_TokensToNodes(Arena *arena, struct Calc_Tokenizer tokenizer) {
    struct Calc_Token *tok = tokenizer.tok_start;
    struct Calc_Node previous_result = {};
    
    if (tokenizer.token_count % 2 == 0 || tokenizer.token_count == 0) {
        return {};
    }
    
    //
    // The way this works is: You take the first three nodes,
    //   evaluate them, then compress them into one node.
    //   Then, you repeat with the following three nodes
    //   until you run out. We don't do order-of-operations
    //   here.
    //
    
#define next_tok() tok = tok->next; if (!tok){break;}
    
    while (tok) {
        struct Calc_Node a = {}, b = {}, op = {};

        if (!tok->next || !tok->next->next) break;
        
        if (tok == tokenizer.tok_start) {
            a = Calc_TokenToNode(*tok);
        } else {
            a = previous_result;
        }
        
        b = Calc_TokenToNode(*tok->next->next);
        op = { tok->next->type, 0, &a, &b };
        
        previous_result = Calc_EvaluateNode(&op);
        
        next_tok();
        next_tok();
    }
    
#undef next_tok
    
    return previous_result;
}


CUSTOM_COMMAND_SIG(spirit_quick_calc)
CUSTOM_DOC("Executes a quick calc operation on text within the mark")
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    
    Range_i64 range = Ii64(view_get_cursor_pos(app, view),
                           view_get_mark_pos(app, view));
    
    Arena arena = make_arena_system();
    
    u8 out[1024] = {0};
    b32 ok = buffer_read_range(app, buffer, range, out);
    
    if (range.max - range.min == 0) return;

    Assert(ok);
    
    struct Calc_Tokenizer tokenizer = 
        Calc_Tokenize(&arena, SCu8(out, range.max-range.min));
    
    struct Calc_Node result = Calc_TokensToNodes(&arena, tokenizer);
    
    char result_text[256] = {0};
    u64 length = sprintf(result_text, "%f", result.value);
    
    String_Const_u8 str = SCu8(result_text, length);
    
    buffer_replace_range(app, buffer, range, str);
}