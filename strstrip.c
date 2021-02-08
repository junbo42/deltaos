#include <string.h>

char *strstrip(char *s){
    int i;
    char *d = (char *)s;
    unsigned int len = strlen(s) - 1;
      
    for(i = 0; i < len; i++){
        if(s[i] == ' ' || s[i] == '\n')
            d++;
        else
            break;
    } 
      
    s = d;
    len = strlen(s) - 1;
      
    for(i = len; i >= 0; i++){
        if(s[i] == ' ' || s[i] == '\n')
            s[i] = 0;
        else
            break;
    } 
      
    return s;
}
