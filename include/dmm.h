#ifndef DMM_H
#define DMM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" { 
#endif
    struct DmmResult {
        float voltage;
        float current;
        uint32_t timestamp;
    };

    void dmmInit(struct VM *vm);
    void dmmRead(struct VM *vm, struct DmmResult *result, uint8_t channel);
#ifdef __cplusplus
}
#endif

#endif // DMM_H