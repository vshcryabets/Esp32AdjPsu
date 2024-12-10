#ifndef UI_H
#define UI_H

#include "vm.h"

#ifdef __cplusplus
extern "C" { 
#endif

void uiInit(struct VM *state);
void uiDraw(struct VM *state);

#ifdef __cplusplus
}
#endif


#endif // UI_H