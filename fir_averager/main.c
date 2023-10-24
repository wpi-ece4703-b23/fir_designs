#include "xlaudio.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "xlaudio_armdsp.h"
#include <stdlib.h>

uint16_t processSample(uint16_t x) {
    static float32_t taps[9];

    // white noise
    float32_t input = xlaudio_adc14_to_f32(0x1800 + rand() % 0x1000);

    uint32_t i;
    for (i=0; i<8; i++)
        taps[8-i] = taps[7-i];
    taps[0] = input;

    // the filter. We're adding a scale factor to avoid overflow.
    float32_t r = 0.125f * (taps[1] + taps[2] + taps[3] + taps[4] + taps[5] + taps[6] + taps[7] + taps[8]);

    return xlaudio_f32_to_dac14(r);
}

#include <stdio.h>

int main(void) {
    WDT_A_hold(WDT_A_BASE);

    xlaudio_init_intr(FS_32000_HZ, XLAUDIO_J1_2_IN, processSample);
    xlaudio_run();

    return 1;
}
