#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

namespace Debug {
    void Initialize(void);
    void printf(const char *Format , ...);
    void PrintString(const char *String);

    void SetPosition(int X , int Y);
    void MovePosition(int RelativeX , int RelativeY);
    void GetScreenInformation(int *X , int *Y , unsigned char *BackgroundColor , unsigned char *ForegroundColor);
    unsigned char GetCharacter(int X , int Y);
}

#endif