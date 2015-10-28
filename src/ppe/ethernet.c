/*
 * Copyright(C) 2015 Simon Schmidt
 * 
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 */
#include <ppe/ethernet.h>
#include <ppe/packing.h>
#include <ppe/stdint.h>
#include <ppe/errornum.h>
#include <ppe/endianess.h>
#include <ppe/_lib_crc.h>


/*
 *    EthernetFrameHeader
 * @brief Ethernet-Frame Header
 */
typedef net_struct_begin{
	uint8_t dstMacAddr[6];
	uint8_t srcMacAddr[6];
	uint16_t type;
} net_struct_end EthernetFrameHeader;

/*
 *    EthernetFrameFooter
 * @brief Ethernet-Frame Footer
 */
typedef net_struct_begin{
	uint32_t crc;
} net_struct_end EthernetFrameFooter;


/*
 *    MacAddr
 * @brief Mac Address
 * This structure is designed, to efficiently assign a mac address.
 */
typedef net_struct_begin{
	uint8_t content[6];
} net_struct_end MacAddr;

typedef void* Pointer;

int ppe_createPacket_eth(ppeBuffer *packet, Eth_FrameInfo *info) {
	Pointer beginHeader, endHeader, beginFooter, endFooter;
	EthernetFrameHeader *header;
	EthernetFrameFooter *footer;

	/*
	 * Unpack memory address of the start and end of the packet.
	 */
	endHeader    =   packet->position;
	beginFooter  =   packet->limit;

	/*
	 * Calculating the outer frame boundaries.
	 */
	beginHeader  =   endHeader - sizeof(EthernetFrameHeader);
	endFooter    =   beginFooter + sizeof(EthernetFrameFooter);

	/*
	 * Check, wether the new frame exceeds the buffer boundaries.
	 */
	if(beginHeader < packet->begin) return ERROR_BUFFER_OVERFLOW;
	if(endFooter > packet->end) return ERROR_BUFFER_OVERFLOW;

	/*
	 * Assign the header and footer struct-pointers.
	 */
	header  =  beginHeader;
	footer  =  beginFooter;

	/*
	 * Construct the header.
	 */
	*((MacAddr*)header->srcMacAddr)   =   *((MacAddr*)info->local);
	*((MacAddr*)header->dstMacAddr)   =   *((MacAddr*)info->remote);
	header->type                      =   encBE16( info->type );

	/*
	 * Calculate the CRC32 sum and construct the footer.
	 */
	footer->crc  =  encBE32( crc32(0,beginHeader,beginFooter-beginHeader) );
	return 0;
}

int ppe_parsePacket_eth(ppeBuffer *packet, Eth_FrameInfo *info){
	Pointer beginHeader, endHeader, beginFooter;
	EthernetFrameHeader *header;
	EthernetFrameFooter *footer;

	/*
	 * Unpack memory address of the start of the packet.
	 */
	beginHeader =   packet->position;

	/*
	 * Calculating the inner frame boundaries.
	 */
	endHeader   =   beginHeader + sizeof(EthernetFrameHeader);
	beginFooter =   packet->limit - sizeof(EthernetFrameFooter);

	/*
	 * Check, wether the header and footer size exceed the packet size.
	 */
	if(endHeader > beginFooter) return ERROR_BUFFER_OVERFLOW;

	/*
	 * Extract Header and Footer Informations.
	 */
	header                     =    beginHeader;
	*((MacAddr*)info->remote)  =    *((MacAddr*)header->srcMacAddr);
	*((MacAddr*)info->local)   =    *((MacAddr*)header->dstMacAddr);
	info->type                 =    decBE16( header->type );
	info->length               =    (uint16_t)(beginFooter-endHeader);
	footer                     =    beginFooter;
	info->crcsum               =    decBE32( footer->crc );

	/*
	 * Check for corruption of the frame.
	 */
	if(info->crcsum != crc32(0,beginHeader,beginFooter-beginHeader))
		return ERROR_CHECKSUM_MISMATCH;

	/*
	 * Assign the new boundaries to the packet.
	 */
	packet->position   =   endHeader;
	packet->limit      =   beginFooter;
	return 0;
}


