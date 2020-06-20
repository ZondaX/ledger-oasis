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

#pragma once

#include <zxmacros.h>
#include "coin.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t hdPath[HDPATH_LEN_DEFAULT];

uint16_t crypto_encodeAddress(char *addr_out, uint16_t addr_out_max, uint8_t *pubkey);

uint16_t crypto_fillAddress(uint8_t *buffer, uint16_t bufferLen);

uint16_t crypto_sign(uint8_t *signature,
                     uint16_t signatureMaxlen,
                     const uint8_t *message,
                     uint16_t messageLen);

#ifdef __cplusplus
}
#endif
