/*
 * Copyright(C) 2015 Simon Schmidt
 * 
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 */
#include <ppe/ip_ph.h>
#include <ppe/endianess.h>

/*
 *
 *
 *             Figure 1: IPv4 pseudo header
 *
 *   0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                       Source Address                          |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    Destination Address                        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |     Zeros     |    Protocol   |          TCP Length           |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 *
 *             Figure 2: IPv6 pseudo header
 *
 *   0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   |                                                               |
 *   |                                                               |
 *   |                       Source Address                          |
 *   |                                                               |
 *   |                                                               |
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                                                               |
 *   |                                                               |
 *   |                                                               |
 *   |                    Destination Address                        |
 *   |                                                               |
 *   |                                                               |
 *   |                                                               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                          TCP Length                           |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    Zeros                      |    Protocol   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */


uint64_t ppe_ipphChecksum(IPPH_Struct *ipph, uintptr_t size){
	uint64_t checksum = 0;
	uint16_t *ptr = 0;

	/*
	 * Check the IP Pseudoheader Type.
	 * The only accepted values are
	 *   IPPH_IPv4   and
	 *   IPPH_IPv6
	 * Everything else will be ignored, and 0 will be returned.
	 *
	 * As the checksum is just an addition of all 16-bit words,
	 * the order doesn't matter anymore. So, if the source and destination
	 * IP-addresses are swapped, the resulting checksum is still the same.
	 */
	switch(ipph->ipphType){
	case IPPH_IPv4:

		/*
		 * Calculating the checksum of the IPv4 Pseudoheader.
		 * See figure 1.
		 */
		checksum += (ipph->ipv4.remote & 0xffff) + ((ipph->ipv4.remote>>16) & 0xffff);
		checksum += (ipph->ipv4.local  & 0xffff) + ((ipph->ipv4.local>>16)  & 0xffff);
		checksum += encBE16(ipph->ipv4.protocol);
		checksum += encBE16(size&0xffff);
		break;
	case IPPH_IPv6:

		/*
		 * Calculating the checksum of the IPv6 Pseudoheader.
		 * See figure 2.
		 */
		ptr = (uint16_t*) ipph->ipv6.remote;
		checksum += ptr[0];
		checksum += ptr[1];
		checksum += ptr[2];
		checksum += ptr[3];
		checksum += ptr[4];
		checksum += ptr[5];
		checksum += ptr[6];
		checksum += ptr[7];
		ptr = (uint16_t*) ipph->ipv6.local;
		checksum += ptr[0];
		checksum += ptr[1];
		checksum += ptr[2];
		checksum += ptr[3];
		checksum += ptr[4];
		checksum += ptr[5];
		checksum += ptr[6];
		checksum += ptr[7];
		checksum += encBE16(size&0xffff);
		checksum += encBE16((size>>16)&0xffff);
		checksum += encBE16(ipph->ipv6.protocol);
		break;
	}

	/*
	 * When in doubt, produce the hash of an empty string.
	 */
	return checksum;
}


