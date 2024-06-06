#ifndef ANSI_REPL_H_
#define ANSI_REPL_H_
#include <stdint.h>
#define ANSI_ESC "\x1b"
extern void EX_PRINT_STR(uint8_t *str);
extern void EX_PRINT_STR_LEN(uint8_t *str, uint32_t len);

typedef enum{
	ANSI_Colors_Default = 0,
	ANSI_Colors_Reset,
	ANSI_Colors_Black,
	ANSI_Colors_Red,
	ANSI_Colors_Green,
	ANSI_Colors_Yellow,
	ANSI_Colors_Blue,
	ANSI_Colors_Magenta,
	ANSI_Colors_Cyan,
	ANSI_Colors_White,
}ANSI_Colors_k;

typedef enum{
	ANSI_Graphics_Default = 0,
	ANSI_Graphics_Reset = 1,
	ANSI_Graphics_Bold = 2,
	ANSI_Graphics_Underline = 4,
	ANSI_Graphics_Blink = 8,
	ANSI_Graphics_Invert = 16,
	ANSI_Graphics_Invisible = 32,
	ANSI_Graphics_Strikethrough = 64,
	ANSI_Graphics_Italic = 128,
}ANSI_Graphics_k;

typedef struct{
	uint8_t *buf;
	uint32_t len;
	ANSI_Colors_k fg;
	ANSI_Colors_k bg;
	ANSI_Graphics_k graphics;
}ANSI_String_t;

extern ANSI_String_t ANSI_FILE;
extern ANSI_String_t ANSI_FOLDER;
extern ANSI_String_t ANSI_TEXT;
extern ANSI_String_t ANSI_ERROR;


void ANSI_Colors_Set_Fg(ANSI_Colors_k fg);
void ANSI_Colors_Set_Bg(ANSI_Colors_k bg);
void ANSI_Colors_Set(ANSI_Colors_k fg, ANSI_Colors_k bg);
void ANSI_Graphics_Set(ANSI_Graphics_k graphics);

void ANSI_String_Print(ANSI_String_t *str);

void ANSI_String_Set(ANSI_String_t *str);

void ANSI_Reset(void);

#endif // ANSI_REPL_H_
