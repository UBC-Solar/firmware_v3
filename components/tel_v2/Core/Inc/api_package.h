#ifndef __API_PACKAGE_H__
#define __API_PACKAGE_H__

#ifdef __cplusplus
extern "C" {
#endif


 uint16_t apiPackage(uint8_t raw_outbox[], uint8_t end_position, uint8_t unsized_packet[], uint8_t MessageType);

#ifdef __cplusplus
}
#endif

#endif
