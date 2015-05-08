/*-
 *  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
 *  code or tables extracted from it, as desired without restriction.
 */

uint32_t crc32(uint32_t crc, const void *buf, size_t size);
void crc32_serialize(uint32_t crc, const void *buf);
uint32_t crc32_deserialize(const void *buf);
