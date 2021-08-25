#pragma once

#ifndef MAGICBITBOARDS_H_
#define MAGICBITBOARDS_H_

void initMagics();
U64 magicBishopAttacks(U64 occ, int sq);
U64 magicRookAttacks(U64 occ, int sq);
int popcount(U64 bb);
U64 randomULL();
int bitscanForward(U64 bb);

#endif

