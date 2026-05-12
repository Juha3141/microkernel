static Pair<word,bool> convert_scancode_1_e0(byte scancode) {
    special_key_status.e0_received = false;
    switch(scancode) {
        case 0x1D: return {INPUT_KBD_SKEY_LCTRL , true};
        case 0x35: return {'/'|INPUT_KBD_DATA_KEYPAD , false};
        case 0x47: return {INPUT_KBD_SKEY_HOME , true};
        case 0x48: return {INPUT_KBD_SKEY_CURSOR_UP , true};
        case 0x49: return {INPUT_KBD_SKEY_PGUP , true};
        case 0x4f: return {INPUT_KBD_SKEY_END , true};
        case 0x50: return {INPUT_KBD_SKEY_CURSOR_DOWN , true};
        case 0x51: return {INPUT_KBD_SKEY_PGDOWN , true};
        case 0x52: return {INPUT_KBD_SKEY_INSERT , true};
        case 0x53: return {INPUT_KBD_SKEY_DELETE , true};
    }
    return {0xffff, true};
}

Pair<word,bool> convert_scancode_1(byte scancode) {
    bool shift = special_key_status.lshift_pressed|special_key_status.rshift_pressed;
    bool ctrl  = special_key_status.lctrl_pressed|special_key_status.rctrl_pressed;
    bool alt   = special_key_status.lalt_pressed|special_key_status.ralt_pressed;
    bool capslock = special_key_status.capslock;
    bool numlock  = special_key_status.numlock;
    scancode &= 0x7f;

    if(special_key_status.e0_received) return convert_scancode_1_e0(scancode);
    
    // Scancode interpretation that requires no translation from special keys
    switch(scancode) {
        case 0x01: return {INPUT_KBD_SKEY_ESC       , true};
        case 0x0E: return {(word)'\b'               , false};
        case 0x0F: return {(word)'\t'               , false};
        case 0x1C: return {(word)'\n'               , false};
        case 0x1D: return {INPUT_KBD_SKEY_LCTRL     , true};
        case 0x2A: return {INPUT_KBD_SKEY_LSHIFT    , true};
        case 0x36: return {INPUT_KBD_SKEY_RSHIFT    , true};
        case 0x37: return {(word)'*'                , false};
        case 0x38: return {INPUT_KBD_SKEY_LALT      , true};
        case 0x39: return {(word)' '                , false};
        case 0x3A: return {INPUT_KBD_SKEY_CAPSLOCK  , true};
        case 0x3B: return {INPUT_KBD_SKEY_F1        , true};
        case 0x3C: return {INPUT_KBD_SKEY_F2        , true};
        case 0x3D: return {INPUT_KBD_SKEY_F3        , true};
        case 0x3E: return {INPUT_KBD_SKEY_F4        , true};
        case 0x3F: return {INPUT_KBD_SKEY_F5        , true};
        case 0x40: return {INPUT_KBD_SKEY_F6        , true};
        case 0x41: return {INPUT_KBD_SKEY_F7        , true};
        case 0x42: return {INPUT_KBD_SKEY_F8        , true};
        case 0x43: return {INPUT_KBD_SKEY_F9        , true};
        case 0x44: return {INPUT_KBD_SKEY_F10       , true};
        case 0x45: return {INPUT_KBD_SKEY_NUMLOCK   , true};
        case 0x46: return {INPUT_KBD_SKEY_SCRLOCK   , true};
        case 0x57: return {INPUT_KBD_SKEY_F11       , true};
        case 0x58: return {INPUT_KBD_SKEY_F12       , true};
    }
    // Number keys
    if(scancode >= 0x02 && scancode <= 0x0D) {
        char dat[]       = "1234567890-=";
        char dat_shift[] = "!@#$%^&*()_+";
        return {(word)(shift ? dat_shift[scancode-0x02] : dat[scancode-0x02]) , false};
    }
    // Alphabets
    if(scancode >= 0x10 && scancode <= 0x19) {
        char dat[]       = "qwertyuiop";
        char dat_shift[] = "QWERTYUIOP";
        return {(word)((shift^capslock) ? dat_shift[scancode-0x10] : dat[scancode-0x10]) , false};
    }
    if(scancode == 0x1A) return {(word)(shift ? '{' : '[') , false};
    if(scancode == 0x1B) return {(word)(shift ? '}' : ']') , false};
    if(scancode >= 0x1E && scancode <= 0x26) {
        char dat[]       = "asdfghjkl";
        char dat_shift[] = "ASDFGHJKL";
        return {(word)((shift^capslock) ? dat_shift[scancode-0x1E] : dat[scancode-0x1E]) , false};
    }
    if(scancode == 0x27) return {(word)(shift ? ':' : ';')   , false};
    if(scancode == 0x28) return {(word)(shift ? '\"' : '\'') , false};
    if(scancode == 0x29) return {(word)(shift ? '~' : '`')   , false};
    if(scancode == 0x2B) return {(word)(shift ? '|' : '\\')  , false};
    if(scancode >= 0x2C && scancode <= 0x32) {
        char dat[]       = "zxcvbnm";
        char dat_shift[] = "ZXCVBNM";
        return {(word)((shift^capslock) ? dat_shift[scancode-0x2C] : dat[scancode-0x2C]) , false};
    }
    if(scancode == 0x33) return {(word)(shift ? '<' : ',') , false};
    if(scancode == 0x34) return {(word)(shift ? '>' : '.') , false};
    if(scancode == 0x35) return {(word)(shift ? '?' : '/') , false};

    if(scancode >= 0x47 && scancode <= 0x53) {
        char dat_numlock[] = "789-456+1230.";
        word dat[] = {
            INPUT_KBD_SKEY_HOME , INPUT_KBD_SKEY_CURSOR_UP , INPUT_KBD_SKEY_PGUP , 
            INPUT_KBD_SKEY_CURSOR_LEFT , 0xffff , INPUT_KBD_SKEY_CURSOR_RIGHT , 
            INPUT_KBD_SKEY_END , INPUT_KBD_SKEY_CURSOR_DOWN , INPUT_KBD_SKEY_PGDOWN , 
            INPUT_KBD_SKEY_DELETE
        };
        return (numlock|shift) ? make_pair((word)(dat_numlock[scancode-0x47]|INPUT_KBD_DATA_KEYPAD) , false)
            : make_pair((word)(dat[scancode-0x47]|INPUT_KBD_DATA_KEYPAD) , true);
    }
    return {0xffff , true};
}

Pair<word,bool> convert_scancode_2(byte scancode) {
    return {0xffff , true};
}