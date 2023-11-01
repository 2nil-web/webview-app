
#include <iostream>
#include <cstdio>
#include <ctype.h>
 
int main(void) {
   int ch;
 
   for (ch = 0; ch <= 0x7e; ch++) {
     std::cout << "The character code " << std::hex << ch << " has ";
    if (isascii(ch))
       std::cout << "the following ASCII representation : "<< (char)ch << std::endl;
    else
       std::cout << "no ASCII representation." << std::endl;
   }
   return 0;
}
