#include "xlaudio.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "xlaudio_armdsp.h"
#include <stdlib.h>

// comment

typedef struct cascadestate {
    float32_t s[2];
    float32_t c[2];
} cascadestate_t;

float32_t cascadefir(float32_t x, cascadestate_t *p) {
    float32_t r = x + (p->s[0] * p->c[0]) + (p->s[1] * p->c[1]);
    p->s[1] = p->s[0];
    p->s[0] = x;
    return r;
}

void createcascade(float32_t c0, float32_t c1, cascadestate_t *p) {
    p->c[0] = c0;
    p->c[1] = c1;
    p->s[0] = p->s[1] = 0.0f;
}

cascadestate_t stage1;
cascadestate_t stage2;
cascadestate_t stage3;
cascadestate_t stage4;

#define M_SQRT2 1.41421356237309504880

void initcascade() {
    createcascade(0.0f, 1.0f, &stage1);
    createcascade(M_SQRT2, 1.0f, &stage2);
    createcascade(-M_SQRT2, 1.0f, &stage3);
    createcascade(1.0f, 0.0f, &stage4);
}

uint16_t processCascade(uint16_t x) {

    float32_t input = xlaudio_adc14_to_f32(0x1800 + rand() % 0x1000);
    float32_t v;
    static float32_t d;

    v = cascadefir(d, &stage1);
    v = cascadefir(v, &stage2);
    v = cascadefir(v, &stage3);
    v = cascadefir(v, &stage4);
    d = input;

    return xlaudio_f32_to_dac14(v * 0.125);
}

uint16_t processSample(uint16_t x)
{
    static float32_t taps[9];

    // white noise
    float32_t input = xlaudio_adc14_to_f32(0x1800 + rand() % 0x1000);

    // test signal
    // float32_t input = xlaudio_adc14_to_f32(x);

    uint32_t i;
    for (i = 0; i < 8; i++)
        taps[8 - i] = taps[7 - i];
    taps[0] = input;

    // the filter. We're adding a scale factor to avoid overflow.
    float32_t r = 0.125f
            * (taps[1] + taps[2] + taps[3] + taps[4] + taps[5] + taps[6]
                    + taps[7] + taps[8]);

    return xlaudio_f32_to_dac14(r);
}

int main(void) {
    WDT_A_hold(WDT_A_BASE);

    initcascade();

    xlaudio_init_intr(FS_32000_HZ, XLAUDIO_J1_2_IN, processCascade);
    xlaudio_run();

    return 1;
}
