/*******************************************************************************
*  (c) 2019 Zondax GmbH
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
#pragma once

#define CBOR_PARSER_MAX_RECURSIONS 4

#include <coin.h>
#include <zxtypes.h>
#include <validator/vote.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct {
    const uint8_t *ptr;
    uint8_t len;
    const uint8_t *suffixPtr;
    uint8_t suffixLen;
} context_t;

typedef enum {
    unknownType,
    txType,
    entityType,
    nodeType,
    consensusType
} oasis_blob_type_e;

typedef struct {
    context_t context;

    union {
        oasis_tx_vote_t voteTx;
    } oasis;

    oasis_blob_type_e type;
} parser_tx_t;

#ifdef __cplusplus
}
#endif
