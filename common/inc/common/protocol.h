/*
 * protocol.h
 *
 *  Created on: 22.10.2018
 *      Author: jarko
 */

#ifndef COMMON_INC_COMMON_PROTOCOL_H_
#define COMMON_INC_COMMON_PROTOCOL_H_


#define PROTO_RC_OK             0
#define PROTO_RC_INVALID_CMD    1
#define PROTO_RC_BUSY           2
#define PROTO_RC_INVALID_VAL    3
#define PROTO_RC_TIMEOUT        4

/*
 * No operation command
 *  index: ignored
 *  value: ignored
 *
 * response format [RC]
 */
#define PROTO_CMD_NOP                 0x00

/*
 * Get firmware version
 *  index: ignored
 *  value: ignored
 *
 * response format [RC][V_MAJ][V_MIN]
 */
#define PROTO_CMD_GET_VERSION         0x01

/*
 * Resets device.
 *  index: ignored
 *  value: ignored
 *
 * response - no response
 */
#define PROTO_CMD_RESET               0x02

/*
 * Controls coil state
 *  index: ignored
 *  value: 0 - disable, 1 - enable
 */
#define PROTO_CMD_COIL_ENABLE         0x03

/*
 * Reads pulse vector values
 *  index: ignored
 *  value: ignored
 */
#define PROTO_CMD_PULSE_VECTOR_READ   0x04

/*
 * Writes pulse vector values
 *  index: ignored
 *  value: ignored
 */
#define PROTO_CMD_PULSE_VECTOR_WRITE  0x05

/*
 * Reads data buffer
 *  index: ignored
 *  value: ignored
 */
#define PROTO_CMD_SAMPLE_VECTOR_READ  0x06

/*
 * Writes data buffer
 *  index: ignored
 *  value: ignored
 */
#define PROTO_CMD_SAMPLE_VECTOR_WRITE 0x07

/*
 * Returns sample/pulse vector item size and number.
 *  index: ignored
 *  value: ignored
 *
 *  response format [RC][PULSE_SIZE][PULSE_COUNT][SAMPLE_SIZE][SAMPLE_COUNT]
 *
 *  Where:
 *    - PULSE_SIZE        (1B) - size of one pulse entry (bits)
 *    - SAMPLE_SIZE       (1B) - size of one sample entry (bits)
 *    - DATA_BUFFER_SIZE  (2B) - size of data buffer in bytes (shared between samples and pulses)
 *    - PULSE_VECTOR_SIZE (2B) - pulse vector length in bytes
 */
#define PROTO_CMD_GET_BUFFER_SIZE     0x08

/*
 * index: [8b - flags][8b - prescaler]
 * value: timeout
 *
 * response format [RC][ID]
 */
#define PROTO_TRANSFER_FLAG_FIRST_ON_START 0x8000
#define PROTO_TRANSFER_FLAG_START_ON_EDGE  0x4000
#define PROTO_TRANSFER_FLAG_FALLING_EDGE   0x2000
#define PROTO_TRANSFER_FLAG_TX_MODE        0x1000

#define PROTO_CMD_TRANSFER_START      0x09

/*
 * index: [ID]
 * value: ignored
 *
 * response format [1B - RC][1B - STATUS][2B - samplesCount]
 */

#define PROTO_TRANSFER_STATUS_UNKNOWN     0x00
#define PROTO_TRANSFER_STATUS_OK          0x01
#define PROTO_TRANSFER_STATUS_TIMEOUT     0x02
#define PROTO_TRANSFER_STATUS_IN_PROGRESS 0x03

#define PROTO_CMD_TRANSFER_STATUS     0x0a

#endif /* COMMON_INC_COMMON_PROTOCOL_H_ */
