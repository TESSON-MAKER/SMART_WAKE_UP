#ifndef FONTS_H_
#define FONTS_H_

#include <stdint.h>

typedef struct Font {
	const uint8_t *data;
	const uint8_t asciiBegin;
	const uint8_t asciiEnd;
	const uint8_t asciiOffset;
	const uint8_t datasize;
	const uint8_t length;
	const uint8_t height;
	const uint8_t bytesPerColums;
} Font;

//Font parameters
extern const Font Arial28x28;
extern const Font Arial12x12;

#endif /* FONTS_H */
