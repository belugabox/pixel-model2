// Minimal TGP stub for unit tests that don't need rendering
#include "tgp.h"

unsigned int tgp_read_register(TGP* tgp, unsigned int reg) {
    (void)tgp; (void)reg;
    return 0;
}

void tgp_write_register(TGP* tgp, unsigned int reg, unsigned int value) {
    (void)tgp; (void)reg; (void)value;
}

// Provide minimal functions used elsewhere
TGP* tgp_create() { return nullptr; }
void tgp_destroy(TGP* tgp) { (void)tgp; }
