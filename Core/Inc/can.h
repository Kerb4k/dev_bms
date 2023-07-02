/*
 * can.h
 *
 *  Created on: Jun 14, 2023
 *      Author: dovlati
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "conf.h"

#define GLOBAL_MAILBOX_MASK           0x000000ff
#define CAN_DISABLE_ALL_INTERRUPT_MASK 0xffffffff

/** Define the mailbox mode. */
#define CAN_MB_DISABLE_MODE           0
#define CAN_MB_RX_MODE                1
#define CAN_MB_RX_OVER_WR_MODE        2
#define CAN_MB_TX_MODE                3
#define CAN_MB_CONSUMER_MODE          4
#define CAN_MB_PRODUCER_MODE          5


/** Define CAN mailbox transfer status code. */
#define CAN_MAILBOX_TRANSFER_OK       0     /**< Read from or write into mailbox
	                                           successfully. */
#define CAN_MAILBOX_NOT_READY         0x01  /**< Receiver is empty or
	                                           transmitter is busy. */
#define CAN_MAILBOX_RX_OVER           0x02  /**< Message overwriting happens or
	                                           there're messages lost in
	                                           different receive modes. */
#define CAN_MAILBOX_RX_NEED_RD_AGAIN  0x04  /**< Application needs to re-read
	                                           the data register in Receive with
	                                           Overwrite mode. */

/** Define the struct for CAN message mailbox. */
typedef struct {
	uint32_t ul_mb_idx;
	uint8_t uc_obj_type;  /**< Mailbox object type, one of the six different
	                         objects. */
	uint8_t uc_id_ver;    /**< 0 stands for standard frame, 1 stands for
	                         extended frame. */
	uint8_t uc_length;    /**< Received data length or transmitted data
	                         length. */
	uint8_t uc_tx_prio;   /**< Mailbox priority, no effect in receive mode. */
	uint32_t ul_status;   /**< Mailbox status register value. */
	uint32_t ul_id_msk;   /**< No effect in transmit mode. */
	uint32_t ul_id;       /**< Received frame ID or the frame ID to be
	                         transmitted. */
	uint32_t ul_fid;      /**< Family ID. */
	uint32_t ul_datal;
	uint32_t ul_datah;
} can_mb_conf_t;


int ReadCANBusMessage(uint32_t messageIdentifier, uint8_t* RxData1);

uint8_t CheckCanError( void );

void CanSend(uint8_t *TxData[], uint8_t identifier );

uint8_t Send_cell_data(cell_data_t cell_data[][CELL_NUM]);

void Send_temp_data(temp_data_t temp_data[][GPIO_NUM]);

void Send_Soc(status_data_t *status_data);

#endif /* INC_CAN_H_ */
