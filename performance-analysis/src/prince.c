//
//  prince.c
//  PRINCE-128
//
//  Created by Dare, Christopher E. (Assoc) on 6/1/16.
//  Copyright © 2016 Dare, Christopher E. (Assoc). All rights reserved.
//

#include "prince.h"


/*****************************************************************************/
/* Private variables:                                                        */
/*****************************************************************************/


// State in the form {10,32,54,76,98,ba,dc,fe} for bit ordering in array
static uint8_t* State;


static const uint8_t RoundConstant[96] = {
    /*i=0*/    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*i=1*/    0x31, 0x91, 0xa8, 0xe2, 0x30, 0x07, 0x37, 0x44,
    /*i=2*/    0x4a, 0x90, 0x83, 0x22, 0x92, 0xf9, 0x13, 0x0d,
    /*i=3*/    0x80, 0xe2, 0xaf, 0x89, 0xce, 0xe4, 0xc6, 0x98,
    /*i=4*/    0x54, 0x82, 0x12, 0x6e, 0x83, 0x0d, 0x31, 0x77,
    /*i=5*/    0xeb, 0x45, 0x66, 0xfc, 0x43, 0x9e, 0xc0, 0xc6,
    /*i=6*/    0xe7, 0x8f, 0xf4, 0x87, 0xdf, 0x59, 0xc5, 0x1b,
    /*i=7*/    0x58, 0x48, 0x80, 0x15, 0x1f, 0xca, 0x34, 0xaa,
    /*i=8*/    0x8c, 0x28, 0x3d, 0xf2, 0x52, 0x23, 0xc3, 0x45,
    /*i=9*/    0x46, 0x5a, 0x11, 0x59, 0x0e, 0x3e, 0x16, 0xd0,
    /*i=10*/   0x3d, 0x5b, 0x3a, 0x99, 0xac, 0xc0, 0x32, 0x99,
    /*i=11*/   0x0c, 0xca, 0x92, 0x7b, 0x9c, 0xc7, 0x05, 0xdd
};

static const uint8_t SBox[16] = {
    //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
    0x0B, 0x0F, 0x03, 0x02, 0x0A, 0x0C, 0x09, 0x01, 0x06, 0x07, 0x08, 0x00, 0x0E, 0x05, 0x0D, 0x04
};
static const uint8_t InvSBox[16] = {
    //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
    0x0B, 0x07, 0x03, 0x02, 0x0F, 0x0D, 0x08, 0x09, 0x0A, 0x06, 0x04, 0x00, 0x05, 0x0E, 0x0C, 0x01
};

/*****************************************************************************/
/* Function Declarations:                                                    */
/*****************************************************************************/
static void MPrimeLayer();
static void MLayer();
static void ShiftRow();
static void InvShiftRow();
static void InvMLayer();
static void addRoundKey(uint8_t* Key);
static void addRoundConstant(uint8_t round);
static void sBoxLayer();
static void InvSBoxLayer();


/*****************************************************************************/
/* Private functions:                                                        */
/*****************************************************************************/

/* The last 64 bits of the round key are added to the state */
static void addRoundKey(uint8_t* Key)
{
    

    State[0] ^= Key[8];
    State[1] ^= Key[9];
    State[2] ^= Key[10];
    State[3] ^= Key[11];
    State[4] ^= Key[12];
    State[5] ^= Key[13];
    State[6] ^= Key[14];
    State[7] ^= Key[15];
} // End addRoundKey()

/* Each nibble of the 64-bit state goes through a non-linear S-Box.
 In our algorithm we use a mask and bit shift to isolate each nibble
 separately.
 */
static void sBoxLayer()
{
    uint8_t lownib, highnib, i;
    
    for (i = 0; i < 8; i++)
    {
        lownib = State[i] & 0x0F;
        highnib = State[i] >> 4;
        lownib = SBox[lownib];
        highnib = SBox[highnib];
        highnib <<= 4;
        State[i] = lownib + highnib;
    } // End for-loop()
} // End sBoxLayer()


/* In the M and M0-layer the 64-bit state is multiplied with a 64 × 64 matrix M
 (resp. M0) defined below. We have different requirements for the two different
 linear layers. The M0-layer is only used in the middle round, thus M0 has to
 be an involution to ensure the α-reflection property. Each output bit
 of an Sbox has to influence 3 Sboxes in the next round and therefore the minimum
 number of ones per row and column is 3
 
 In our algorithm we use a variance of bitwise shifts and masks in order to mimic
 the bitwise matrix multiplication operation on the state. Since the even indexed
 elements of the state are changed before the odd, yet the odd relies on the even,
 we save the even indexed elements of the state in Read Only Memory.
 */
static void MPrimeLayer()
{
    // temporary storage variables stored in read only memory
    uint8_t s0 = State[0];
    uint8_t s2 = State[2];
    uint8_t s4 = State[4];
    uint8_t s6 = State[6];
    
    //M0
    State[0] = (s0 & 0xD7) ^ (State[1] & 0x7D) ^ (s0 >> 4 & 0x0B) ^ (State[1] >> 4 & 0x0E) ^ (s0 << 4 & 0xB0) ^ (State[1] << 4 & 0xE0);
    State[1] = (s0 & 0x7D) ^ (State[1] & 0xD7) ^ (s0 >> 4 & 0x0E) ^ (State[1] >> 4 & 0x0B) ^ (s0 << 4 & 0xE0) ^ (State[1] << 4 & 0xB0);
    //M1
    State[2] = (s2 & 0xEB) ^ (State[3] & 0xBE) ^ (s2 >> 4 & 0x0D) ^ (State[3] >> 4 & 0x07) ^ (s2 << 4 & 0xD0) ^ (State[3] << 4 & 0x70);
    State[3] = (s2 & 0xBE) ^ (State[3] & 0xEB) ^ (s2 >> 4 & 0x07) ^ (State[3] >> 4 & 0x0D) ^ (s2 << 4 & 0x70) ^ (State[3] << 4 & 0xD0);
    //M1
    State[4] = (s4 & 0xEB) ^ (State[5] & 0xBE) ^ (s4 >> 4 & 0x0D) ^ (State[5] >> 4 & 0x07) ^ (s4 << 4 & 0xD0) ^ (State[5] << 4 & 0x70);
    State[5] = (s4 & 0xBE) ^ (State[5] & 0xEB) ^ (s4 >> 4 & 0x07) ^ (State[5] >> 4 & 0x0D) ^ (s4 << 4 & 0x70) ^ (State[5] << 4 & 0xD0);
    //M0
    State[6] = (s6 & 0xD7) ^ (State[7] & 0x7D) ^ (s6 >> 4 & 0x0B) ^ (State[7] >> 4 & 0x0E) ^ (s6 << 4 & 0xB0) ^ (State[7] << 4 & 0xE0);
    State[7] = (s6 & 0x7D) ^ (State[7] & 0xD7) ^ (s6 >> 4 & 0x0E) ^ (State[7] >> 4 & 0x0B) ^ (s6 << 4 & 0xE0) ^ (State[7] << 4 & 0xB0);
    
} // End MPrimeLayer()


/* The MLayer is simply the M-Prime layer with a Row Shift operation
 to shift each nibble. For this reason, the MLayer no longer has the
 α-reflection property mentioned above in the MPrimeLayer. Due to the α-reflection property,
 the MPrimeLayer is its own inverse (since addition and subtraction are identical in the Galois
 Field, matrix multiplication must be invertible as well); however, the SR prevents the
 MLayer from being its own inverse, such that we require an Inverse MLayer protocol as well.
 */
static void MLayer()
{
    MPrimeLayer();
    ShiftRow();
} // End MLayer

/* Our ShiftRow() operation performs similarly to the AES Shift Row
 and permutes the 16 nibbles in the following way:
 
 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 −→ 0 5 10 15 4 9 14 3 8 13 2 7 12 1 6 11
 */
static void ShiftRow()
{
    uint8_t temp[8];
    
    temp[0] = (State[0] & 0x0F) | (State[2] & 0xF0);
    temp[1] = (State[5] & 0x0F) | (State[7] & 0xF0);
    temp[2] = (State[2] & 0x0F) | (State[4] & 0xF0);
    temp[3] = (State[7] & 0x0F) | (State[1] & 0xF0);
    temp[4] = (State[4] & 0x0F) | (State[6] & 0xF0);
    temp[5] = (State[1] & 0x0F) | (State[3] & 0xF0);
    temp[6] = (State[6] & 0x0F) | (State[0] & 0xF0);
    temp[7] = (State[3] & 0x0F) | (State[5] & 0xF0);
    
    for (uint8_t i = 0; i < 8; i++) {
        State[i] = temp[i];
    } // End for-loop
} // End ShiftRow()

/* In the RCi-add step a 64-bit round constant is xored with the state.
 We define the constants used below (in hex notation):
 RC0 | 0000000000000000
 RC1 | 13198a2e03707344
 RC2 | a4093822299f31d0
 RC3 | 082efa98ec4e6c89
 RC4 | 452821e638d01377
 RC5 | be5466cf34e90c6c
 RC6 | 7ef84f78fd955cb1
 RC7 | 85840851f1ac43aa
 RC8 | c882d32f25323c54
 RC9 | 64a51195e0e3610d
 RC10| d3b5a399ca0c2399
 RC11| c0ac29b7c97c50dd
 
 Note that, for all 0 ≤ i ≤ 11, RCi⊕RC11−i
 is the constant α = c0ac29b7c97c50dd,
 RC0 = 0 and that RC1, . . . , RC5 and α are derived from the fraction part of
 π = 3.141....
 
 In our algorithm we treat each round constant as a byte array of length 8,
 stored in little endian order such that we can add the round constant to the
 state simply by state[i] += RoundConstant[8 * round + i];
 */
static void addRoundConstant(uint8_t round)
{
    State[0] ^= RoundConstant[8 * round];
    State[1] ^= RoundConstant[8 * round + 1];
    State[2] ^= RoundConstant[8 * round + 2];
    State[3] ^= RoundConstant[8 * round + 3];
    State[4] ^= RoundConstant[8 * round + 4];
    State[5] ^= RoundConstant[8 * round + 5];
    State[6] ^= RoundConstant[8 * round + 6];
    State[7] ^= RoundConstant[8 * round + 7];
} // End addRoundConstant()

/* As noted above, the MBox in each MLayer prevents MLayer from being its own inverse.
 The MLayer is defined as M = SR ◦ M'. Because ◦ (function composition) composes the group
 operation on the set of all functions from GF(2) to GF(2), M must have an inverse, defined by
 M^-1 = M'^-1 ◦ SR^-1 = M' ◦ SR^-1 . Thus, we first take the inverse MBox and call MPrimeLayer
 (as it is its own inverse).
 */
static void InvMLayer()
{
    InvShiftRow();
    MPrimeLayer();
} // End InvMLayer()

/* We invert the SBox and apply the transformation to each nibble in the state
 such that SBox[InvSBox[x]] = x and InvSBox[SBox[x]] = x for all x in GF(2)
 */
static void InvSBoxLayer()
{
    uint8_t lownib, highnib, i;
    
    for (i = 0; i < 8; i++)
    {
        lownib = State[i] & 0x0F;
        highnib = State[i] >> 4;
        lownib = InvSBox[lownib];
        highnib = InvSBox[highnib];
        highnib <<= 4;
        State[i] = lownib + highnib;
    } // End for-loop
} // End InvSBoxLayer()

static void InvShiftRow()
{
    uint8_t temp[8];
    
    temp[0] = (State[0] & 0x0F) | (State[6] & 0xF0);
    temp[1] = (State[5] & 0x0F) | (State[3] & 0xF0);
    temp[2] = (State[2] & 0x0F) | (State[0] & 0xF0);
    temp[3] = (State[7] & 0x0F) | (State[5] & 0xF0);
    temp[4] = (State[4] & 0x0F) | (State[2] & 0xF0);
    temp[5] = (State[1] & 0x0F) | (State[7] & 0xF0);
    temp[6] = (State[6] & 0x0F) | (State[4] & 0xF0);
    temp[7] = (State[3] & 0x0F) | (State[1] & 0xF0);
    
    for (uint8_t i = 0; i < 8; i++) {
        State[i] = temp[i];
    }
}

/* In our cipher, we use the first 64 bits of the key as for whitening before proceeding
 to PRINCE_core. In prince_core, the state undergoes 5 rounds of an Sbox Layer followed
 by an MLayer, followed by adding the Round Constant and last 64 bits of the key,
 respectively. In the middle of the cipher, the SBox is applied, followed by the symmetric
 MPrimeLayer, and an Inverse SBox. For the consequent 5 rounds, the inverse of the first 5 rounds
 is applied. Lastly, the subkey is used for whitening after PRINCE_core has completed.
 
 
 From the fact that the round constants satisfy RCi ⊕ RC11−i = α and that M'
 is an involution, we deduce that the core cipher is such that the inverse
 of PRINCE_core parametrized with k is equal to PRINCE_core parametrized with
 (k⊕α). We call this property of PRINCE_core the α-reflection property. It follows
 that, for any expanded key (k0||k'0||k1),
 D(k0||k'0||k1)(·) = E(k'0||k0||k1⊕α)(·)
 */
void cipher(uint8_t* state, uint8_t* Key, uint8_t* subkey)
{
    State = state;
    uint8_t i;
    
    for (i = 0; i < 8; i++) {
        State[i] ^= Key[i];
    } // End for-loop
    
    addRoundKey(Key);
    addRoundConstant(0);
    
    
    for (i = 1; i <= 5; i++)
    {
        sBoxLayer();
        MLayer();
        addRoundConstant(i);
        addRoundKey(Key);
    } // End for-loop
    
    sBoxLayer();
    MPrimeLayer();
    InvSBoxLayer();
    
    for (i = 6; i <= 10; i++)
    {
        addRoundKey(Key);
        addRoundConstant(i);
        InvMLayer();
        InvSBoxLayer();
    } // End for-loop
    
    addRoundConstant(11);
    addRoundKey(Key);
    
    for (i = 0; i < 8; i++) {
        State[i] ^= subkey[i];
    } // End for-loop
} // End cipher()

/*
 From the fact that the round constants satisfy RCi ⊕ RC11−i = α and that M'
 is an involution, we deduce that the core cipher is such that the inverse
 of PRINCE_core parametrized with k is equal to PRINCE_core parametrized with
 (k⊕α). We call this property of PRINCE_core the α-reflection property. It follows
 that, for any expanded key (k0||k'0||k1),
 D(k0||k'0||k1)(·) = E(k'0||k0||k1⊕α)(·)
 */
void decipher(uint8_t* state, uint8_t* Key, uint8_t* subkey)
{
    uint8_t temp[8];
    
    for (uint8_t i = 0; i < 8; i++)
    {
        temp[i] = subkey[i];
    } // End for-loop
    for (uint8_t i = 0; i < 8; i++)
    {
        subkey[i] = Key[i];
    } // End for-loop
    for (uint8_t i = 0; i < 8; i++)
    {
        Key[i] = temp[i];
    } // End for-loop
    
    Key[8] ^= 0x0c;
    Key[9] ^= 0xca;
    Key[10] ^= 0x92;
    Key[11] ^= 0x7b;
    Key[12] ^= 0x9c;
    Key[13] ^= 0xc7;
    Key[14] ^= 0x05;
    Key[15] ^= 0xdd;
    
    cipher(state, Key, subkey);
    
    Key[8] ^= 0x0c;
    Key[9] ^= 0xca;
    Key[10] ^= 0x92;
    Key[11] ^= 0x7b;
    Key[12] ^= 0x9c;
    Key[13] ^= 0xc7;
    Key[14] ^= 0x05;
    Key[15] ^= 0xdd;
} // End decipher

/* The 128-bit key (k0||k1) is extended to a 192-bit key (k0||k'0||k1) by a linear
 mapping of the form
 (k0||k1) -→ (k0||P(k0)||k1).
 Thus, a hardware-optimal choice for P such that both P and P ⊕ Id are
 permutations is
 P(x) = (x ≫> 1) ⊕ (x ≫ 63) ,
 i.e., P(x63, . . . , x0) = (x0, x63, . . . , x2, x1 ⊕ x63).
 
 In our algorithm we perform a simple bitwise shift on the first 64-bits of the
 key in order to produce our subkey, k'0 or P(k0) as denoted above. The subkey does
 not change by round, as in other algorithms.
 */
void keySchedule(uint8_t* Key, uint8_t* subkey)
{
    subkey[0] = (Key[0] >> 1) | (Key[1] << 7);
    subkey[1] = (Key[1] >> 1) | (Key[2] << 7);
    subkey[2] = (Key[2] >> 1) | (Key[3] << 7);
    subkey[3] = (Key[3] >> 1) | (Key[4] << 7);
    subkey[4] = (Key[4] >> 1) | (Key[5] << 7);
    subkey[5] = (Key[5] >> 1) | (Key[6] << 7);
    subkey[6] = (Key[6] >> 1) | (Key[7] << 7);
    subkey[7] = (Key[7] >> 1) | (Key[0] << 7);
    subkey[7] ^= (Key[7] & 0x10);
} // End keySchedule()

