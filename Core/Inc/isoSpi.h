



#ifndef LTC681X_GLUE_H_
#define LTC681X_GLUE_H_

void delay_u(uint32_t us);

void delay_m(uint32_t ms);

uint8_t spi_write_read_byte(uint8_t wbyte);

uint32_t spi_write_array(uint8_t len, uint8_t *data);

uint32_t spi_write_then_read_array_ltc(uint8_t wlen, uint8_t *wbuffer, uint8_t rlen, uint8_t *rbuffer);

#endif /* LTC681X_GLUE_H_ */
