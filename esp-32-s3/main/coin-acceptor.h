// (c) 2025, Christian Schüeler, hello@christianschueler.at

#ifndef COIN_ACCEPTOR_H
#define COIN_ACCEPTOR_H

//extern int coin_acceptor_amount_cents;

void coin_acceptor_init();
void coin_acceptor_loop();
void coin_acceptor_reset_amount();
int coin_acceptor_get_amount_cents();
void coin_acceptor_enable();
void coin_acceptor_disable();

#endif