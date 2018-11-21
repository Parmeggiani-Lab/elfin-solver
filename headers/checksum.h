#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdint.h>
#include <stddef.h>

namespace elfin
{

typedef uint32_t Crc32;

Crc32 checksum_new(void const *__restrict__ p_data, size_t size);

void checksum_cascade(Crc32 *__restrict__ crc, void const *__restrict__ p_data, size_t size);

} // namespace elfin

#endif /* include guard */