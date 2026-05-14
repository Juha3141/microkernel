/* 0xff : invalid */

#include <kernel/input/keyboard_codes.hpp>

const word scancode1_no_shift_map[] = {
    0xff , 
    INPUT_KBD_SKEY_ESC , 
    '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , '\b' , '\t' , 
    'q' , 'w' , 'e' , 'r' , 't' , 'y' , 'u' , 'i' , 'o' , 'p' , '[' , ']' , '\n' , INPUT_KBD_SKEY_LCTRL , 
    'a' , 's' , 'd' , 'f' , 'g' , 'h' , 'j' , 'k' , ';' , '\'' , '`' , INPUT_KBD_SKEY_LSHIFT , '\\' , 
    'z' , 'x' , 'c' , 'v' , 'b' , 'n' , 'm' , ',' , '.' , '/' , INPUT_KBD_SKEY_RSHIFT , '*'|INPUT_KBD_DATA_KEYPAD , 
    INPUT_KBD_SKEY_LALT , ' ' , INPUT_KBD_SKEY_CAPSLOCK , 
    INPUT_KBD_SKEY_F1 , INPUT_KBD_SKEY_F2 , INPUT_KBD_SKEY_F3 , INPUT_KBD_SKEY_F4 , INPUT_KBD_SKEY_F5 , 
    INPUT_KBD_SKEY_F6 , INPUT_KBD_SKEY_F7 , INPUT_KBD_SKEY_F8 , INPUT_KBD_SKEY_F9 , INPUT_KBD_SKEY_F10 , 
    INPUT_KBD_SKEY_NUMLOCK , INPUT_KBD_SKEY_SCRLOCK , 
    '7'|INPUT_KBD_DATA_KEYPAD , '8'|INPUT_KBD_DATA_KEYPAD , '9'|INPUT_KBD_DATA_KEYPAD , '-'|INPUT_KBD_DATA_KEYPAD , 
    '4'|INPUT_KBD_DATA_KEYPAD , '5'|INPUT_KBD_DATA_KEYPAD , '6'|INPUT_KBD_DATA_KEYPAD , '+'|INPUT_KBD_DATA_KEYPAD , 
    '1'|INPUT_KBD_DATA_KEYPAD , '2'|INPUT_KBD_DATA_KEYPAD , '3'|INPUT_KBD_DATA_KEYPAD , '0'|INPUT_KBD_DATA_KEYPAD , 
    '.'|INPUT_KBD_DATA_KEYPAD , 0xff , 0xff , 0xff , 
    INPUT_KBD_SKEY_F11 , INPUT_KBD_SKEY_F12 , 
    0xff , 0xff , 0xff , 0xff , 
};

const word scancode1_shift_map[] = {
    0xff , 
    INPUT_KBD_SKEY_ESC , 
    '!' , '@' , '#' , '$' , '%' , '^' , '&' , '*' , '(' , ')' , '_' , '+' , '\b' , '\t' , 
    'Q' , 'W' , 'E' , 'R' , 'T' , 'Y' , 'U' , 'I' , 'O' , 'P' , '{' , '}' , '\n' , INPUT_KBD_SKEY_LCTRL , 
    'A' , 'S' , 'D' , 'F' , 'G' , 'H' , 'J' , 'K' , ':' , '\"' , '~' , INPUT_KBD_SKEY_LSHIFT , '|' , 
    'z' , 'x' , 'c' , 'v' , 'b' , 'n' , 'm' , ',' , '.' , '/' , INPUT_KBD_SKEY_RSHIFT , '*'|INPUT_KBD_DATA_KEYPAD , 
    INPUT_KBD_SKEY_LALT , ' ' , INPUT_KBD_SKEY_CAPSLOCK , 
    INPUT_KBD_SKEY_F1 , INPUT_KBD_SKEY_F2 , INPUT_KBD_SKEY_F3 , INPUT_KBD_SKEY_F4 , INPUT_KBD_SKEY_F5 , 
    INPUT_KBD_SKEY_F6 , INPUT_KBD_SKEY_F7 , INPUT_KBD_SKEY_F8 , INPUT_KBD_SKEY_F9 , INPUT_KBD_SKEY_F10 , 
    INPUT_KBD_SKEY_NUMLOCK , INPUT_KBD_SKEY_SCRLOCK , 
    '7'|INPUT_KBD_DATA_KEYPAD , '8'|INPUT_KBD_DATA_KEYPAD , '9'|INPUT_KBD_DATA_KEYPAD , '-'|INPUT_KBD_DATA_KEYPAD , 
    '4'|INPUT_KBD_DATA_KEYPAD , '5'|INPUT_KBD_DATA_KEYPAD , '6'|INPUT_KBD_DATA_KEYPAD , '+'|INPUT_KBD_DATA_KEYPAD , 
    '1'|INPUT_KBD_DATA_KEYPAD , '2'|INPUT_KBD_DATA_KEYPAD , '3'|INPUT_KBD_DATA_KEYPAD , '0'|INPUT_KBD_DATA_KEYPAD , 
    '.'|INPUT_KBD_DATA_KEYPAD , 0xff , 0xff , 0xff , 
    INPUT_KBD_SKEY_F11 , INPUT_KBD_SKEY_F12 , 
    0xff , 0xff , 0xff , 0xff , 
};

const word scancode1_ctrl_map[] = {
    0xff , 
    INPUT_KBD_SKEY_ESC , 
    '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , '\b' , '\t' , 
    'q' , 'w' , 'e' , 'r' , 't' , 'y' , 'u' , 'i' , 'o' , 'p' , '[' , ']' , '\n' , INPUT_KBD_SKEY_LCTRL , 
    'a' , 's' , 'd' , 'f' , 'g' , 'h' , 'j' , 'k' , ';' , '\'' , '`' , INPUT_KBD_SKEY_LSHIFT , '\\' , 
    'z' , 'x' , 'c' , 'v' , 'b' , 'n' , 'm' , ',' , '.' , '/' , INPUT_KBD_SKEY_RSHIFT , '*'|INPUT_KBD_DATA_KEYPAD , 
    INPUT_KBD_SKEY_LALT , ' ' , INPUT_KBD_SKEY_CAPSLOCK , 
    INPUT_KBD_SKEY_F1 , INPUT_KBD_SKEY_F2 , INPUT_KBD_SKEY_F3 , INPUT_KBD_SKEY_F4 , INPUT_KBD_SKEY_F5 , 
    INPUT_KBD_SKEY_F6 , INPUT_KBD_SKEY_F7 , INPUT_KBD_SKEY_F8 , INPUT_KBD_SKEY_F9 , INPUT_KBD_SKEY_F10 , 
    INPUT_KBD_SKEY_NUMLOCK , INPUT_KBD_SKEY_SCRLOCK , 
    '7'|INPUT_KBD_DATA_KEYPAD , '8'|INPUT_KBD_DATA_KEYPAD , '9'|INPUT_KBD_DATA_KEYPAD , '-'|INPUT_KBD_DATA_KEYPAD , 
    '4'|INPUT_KBD_DATA_KEYPAD , '5'|INPUT_KBD_DATA_KEYPAD , '6'|INPUT_KBD_DATA_KEYPAD , '+'|INPUT_KBD_DATA_KEYPAD , 
    '1'|INPUT_KBD_DATA_KEYPAD , '2'|INPUT_KBD_DATA_KEYPAD , '3'|INPUT_KBD_DATA_KEYPAD , '0'|INPUT_KBD_DATA_KEYPAD , 
    '.'|INPUT_KBD_DATA_KEYPAD , 0xff , 0xff , 0xff , 
    INPUT_KBD_SKEY_F11 , INPUT_KBD_SKEY_F12 , 
    0xff , 0xff , 0xff , 0xff , 
};