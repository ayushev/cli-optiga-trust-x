/**
* MIT License
*
* Copyright (c) 2019 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE

*/
/**
 * @defgroup ifx_i2c_config Infineon I2C Protocol Stack: Configuration
 * @{
 * @ingroup ifx_i2c
 *
 * @brief Module for the configuration of the Infineon I2C Protocol Stack library.
 */

#ifndef IFX_I2C_CONFIG_H__
#define IFX_I2C_CONFIG_H__

// IFX I2C Protocol Stack configuration

/** @brief I2C slave address of the Infineon device */
#define IFX_I2C_BASE_ADDR           0x30

/** @brief Physical Layer: polling interval in microseconds */
#define PL_POLLING_INVERVAL_US      10000
/** @brief Physical Layer: guard time interval in microseconds */
#define PL_GUARD_TIME_INTERVAL_US   50
/** @brief Physical layer: maximal attempts */
#define PL_POLLING_MAX_CNT          200

/** @brief Data link layer: maximum frame size */
#define DL_MAX_FRAME_SIZE           0x3B
/** @brief Data link layer: header size */
#define DL_HEADER_SIZE              5
/** @brief Data link layer: maximum number of retries in case of transmission error */
#define DL_MAX_RETRIES              3

// Transport Layer settings
/** @brief Transport layer: maximum fragment size */
#define TL_MAX_FRAGMENT_SIZE        (DL_MAX_FRAME_SIZE - DL_HEADER_SIZE)
/** @brief Transport layer: size of internal buffer
 *  @note Should be large enough to store an X.509 certificate
 */
#define TL_BUFFER_SIZE              2048

/** @brief Protocol Stack status codes for success */
#define IFX_I2C_STACK_SUCCESS       0x00
/** @brief Protocol Stack status codes for error */
#define IFX_I2C_STACK_ERROR         0x01

/** @brief Protocol Stack debug switch for physical layer (set to 0 or 1) */
#define IFX_I2C_LOG_PL              0
/** @brief Protocol Stack debug switch for data link layer (set to 0 or 1) */
#define IFX_I2C_LOG_DL              0
/** @brief Protocol Stack debug switch for transport layer (set to 0 or 1) */
#define IFX_I2C_LOG_TL              0
/** @brief Protocol Stack debug switch for hardware abstraction layer (set to 0 or 1) */
#define IFX_I2C_LOG_HAL             0

/** @brief Log ID number for physical layer */
#define IFX_I2C_LOG_ID_PL           0x00
/** @brief Log ID number for data link layer */
#define IFX_I2C_LOG_ID_DL           0x01
/** @brief Log ID number for transport layer */
#define IFX_I2C_LOG_ID_TL           0x02
/** @brief Log ID number for hardware abstraction layer */
#define IFX_I2C_LOG_ID_HAL          0x04

// Protocol Stack Includes
#include <stdint.h>

/** @brief Event handler function prototype */
typedef void (*ifx_i2c_event_handler_t)(uint8_t event, uint8_t* data, uint16_t data_len);

/**
 * @}
 **/

#endif /* IFX_I2C_CONFIG_H__ */
