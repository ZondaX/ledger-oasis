/*******************************************************************************
*   (c) 2019 Zondax GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "crypto.h"
#include "coin.h"
#include "zxmacros.h"
#include "apdu_codes.h"
#include "coin.h"
#include "sha512.h"

#include <bech32.h>

uint32_t hdPath[HDPATH_LEN_DEFAULT];

#if defined(TARGET_NANOS) || defined(TARGET_NANOX)
#include "cx.h"
#include "rslib.h"

void crypto_extractPublicKey(const uint32_t path[HDPATH_LEN_DEFAULT], uint8_t *pubKey, uint16_t pubKeyLen) {
    cx_ecfp_public_key_t cx_publicKey;
    cx_ecfp_private_key_t cx_privateKey;
    uint8_t privateKeyData[32];

    if (pubKeyLen < PK_LEN_ED25519) {
        return;
    }

    BEGIN_TRY
    {
        TRY {
            // Generate keys
            os_perso_derive_node_bip32_seed_key(
                    HDW_NORMAL,
                    CX_CURVE_Ed25519,
                    path,
                    HDPATH_LEN_DEFAULT,
                    privateKeyData,
                    NULL,
                    NULL,
                    0);

            cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, &cx_privateKey);
            cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, &cx_publicKey);
            cx_ecfp_generate_pair(CX_CURVE_Ed25519, &cx_publicKey, &cx_privateKey, 1);
        }
        FINALLY {
            MEMZERO(&cx_privateKey, sizeof(cx_privateKey));
            MEMZERO(privateKeyData, 32);
        }
    }
    END_TRY;

    // Format pubkey
    for (int i = 0; i < 32; i++) {
        pubKey[i] = cx_publicKey.W[64 - i];
    }

    if ((cx_publicKey.W[32] & 1) != 0) {
        pubKey[31] |= 0x80;
    }
}

uint16_t crypto_sign(uint8_t *signature,
                     uint16_t signatureMaxlen,
                     const uint8_t *message,
                     uint16_t messageLen) {
    uint8_t messageDigest[CX_SHA512_SIZE];
    SHA512_256(message, messageLen, messageDigest);

    cx_ecfp_private_key_t cx_privateKey;
    uint8_t privateKeyData[32];
    int signatureLength;
    unsigned int info = 0;

    BEGIN_TRY
    {
        TRY
        {
            // Generate keys
            os_perso_derive_node_bip32_seed_key(
                    HDW_NORMAL,
                    CX_CURVE_Ed25519,
                    hdPath,
                    HDPATH_LEN_DEFAULT,
                    privateKeyData,
                    NULL,
                    NULL,
                    0);
            cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, &cx_privateKey);

            // Sign
            signatureLength = cx_eddsa_sign(&cx_privateKey,
                                            CX_LAST,
                                            CX_SHA512,
                                            messageDigest,
                                            CX_SHA256_SIZE,
                                            NULL,
                                            0,
                                            signature,
                                            signatureMaxlen,
                                            &info);
        }
        FINALLY {
            MEMZERO(&cx_privateKey, sizeof(cx_privateKey));
            MEMZERO(privateKeyData, 32);
        }
    }
    END_TRY;

    return signatureLength;
}

#else

#define CX_SHA512_SIZE 64

void crypto_extractPublicKey(const uint32_t path[HDPATH_LEN_DEFAULT], uint8_t *pubKey, uint16_t pubKeyLen) {
    ///////////////////////////////////////
    // THIS IS ONLY USED FOR TEST PURPOSES
    ///////////////////////////////////////

    // Empty version for non-Ledger devices
    MEMZERO(pubKey, pubKeyLen);
}

uint16_t crypto_sign(uint8_t *signature,
                     uint16_t signatureMaxlen,
                     const uint8_t *message,
                     uint16_t messageLen) {
    // Empty version for non-Ledger devices
    return 0;
}

#endif

typedef union {
    //    1 byte <context-version> + first 20 bytes of SHA512-256(<context-identifier> || <pubkey>)
    uint8_t address[21];
    struct {
        uint8_t version;
        uint8_t address_truncated_hash[20];
    };
    struct {
        uint8_t padding;
        uint8_t pkHash[CX_SHA512_SIZE];
    };
} tmp_address_t;

uint16_t crypto_encodeAddress(char *addr_out, uint16_t addr_out_max, uint8_t *pubkey) {
    tmp_address_t tmp;
    tmp.version = COIN_ADDRESS_VERSION;

    SHA512_256_with_context (
            (uint8_t *) COIN_ADDRESS_CONTEXT, strlen(COIN_ADDRESS_CONTEXT),
            pubkey, PK_LEN_ED25519,
            tmp.pkHash
    );

    //  and encode as bech32
    const zxerr_t err = bech32EncodeFromBytes(
            addr_out, addr_out_max,
            COIN_HRP,
            tmp.address, sizeof_field(tmp_address_t, address), 1);

    if (err != zxerr_ok) {
        return 0;
    }

    return strlen(addr_out);
}

uint16_t crypto_fillAddress(uint8_t *buffer, uint16_t buffer_len) {
    if (buffer_len < PK_LEN_ED25519 + 50) {
        return 0;
    }
    crypto_extractPublicKey(hdPath, buffer, buffer_len);

    // format pubkey as oasis bech32 address
    char *addr_out = (char *) (buffer + PK_LEN_ED25519);
    const uint16_t addr_out_max =  buffer_len - PK_LEN_ED25519;
    const uint16_t addr_out_len = crypto_encodeAddress(addr_out, addr_out_max, buffer);

    return PK_LEN_ED25519 + addr_out_len;
}
