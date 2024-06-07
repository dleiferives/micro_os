#include "ansi_repl.h"


ANSI_String_t ANSI_FILE = {
	.fg = ANSI_Colors_Blue,
	.bg = ANSI_Colors_Default,
	.graphics = ANSI_Graphics_Default,
};

ANSI_String_t ANSI_FOLDER = {
	.fg = ANSI_Colors_Green,
	.bg = ANSI_Colors_Default,
	.graphics = ANSI_Graphics_Bold,
};

ANSI_String_t ANSI_ERROR = {
	.fg = ANSI_Colors_White,
	.bg = ANSI_Colors_Red,
	.graphics = ANSI_Graphics_Bold | ANSI_Graphics_Underline,
};

ANSI_String_t ANSI_TEXT = {
	.fg = ANSI_Colors_Default,
	.bg = ANSI_Colors_Default,
	.graphics = ANSI_Graphics_Default,
};


////////////////////////////////////////////////////////////////////////////////
/// FUNCTION DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
void ANSI_Colors_Set_Fg(ANSI_Colors_k fg) {
  switch (fg) {
  case ANSI_Colors_Reset:
    EX_PRINT_STR("\x1b[0m");
    break;
  case ANSI_Colors_Black:
    EX_PRINT_STR("\x1b[30m");
    break;
  case ANSI_Colors_Red:
    EX_PRINT_STR("\x1b[31m");
    break;
  case ANSI_Colors_Green:
    EX_PRINT_STR("\x1b[32m");
    break;
  case ANSI_Colors_Yellow:
    EX_PRINT_STR("\x1b[33m");
    break;
  case ANSI_Colors_Blue:
    EX_PRINT_STR("\x1b[34m");
    break;
  case ANSI_Colors_Magenta:
    EX_PRINT_STR("\x1b[35m");
    break;
  case ANSI_Colors_Cyan:
    EX_PRINT_STR("\x1b[36m");
    break;
  case ANSI_Colors_White:
    EX_PRINT_STR("\x1b[37m");
    break;
  case ANSI_Colors_Default:
    EX_PRINT_STR("\x1b[39m");
    break;
  }
}

void ANSI_Colors_Set_Bg(ANSI_Colors_k bg) {
  switch (bg) {
  case ANSI_Colors_Reset:
	EX_PRINT_STR("\x1b[0m");
	break;
  case ANSI_Colors_Black:
	EX_PRINT_STR("\x1b[40m");
	break;
  case ANSI_Colors_Red:
	EX_PRINT_STR("\x1b[41m");
	break;
  case ANSI_Colors_Green:
	EX_PRINT_STR("\x1b[42m");
	break;
  case ANSI_Colors_Yellow:
	EX_PRINT_STR("\x1b[43m");
	break;
  case ANSI_Colors_Blue:
	EX_PRINT_STR("\x1b[44m");
	break;
  case ANSI_Colors_Magenta:
	EX_PRINT_STR("\x1b[45m");
	break;
  case ANSI_Colors_Cyan:
	EX_PRINT_STR("\x1b[46m");
	break;
  case ANSI_Colors_White:
	EX_PRINT_STR("\x1b[47m");
	break;
  case ANSI_Colors_Default:
	EX_PRINT_STR("\x1b[49m");
	break;
  }
}
void ANSI_Colors_Set(ANSI_Colors_k fg, ANSI_Colors_k bg) {
  if (fg == ANSI_Colors_Reset || bg == ANSI_Colors_Reset) {
    EX_PRINT_STR("\x1b[0m");
    return;
  }
  switch (fg) {
  case ANSI_Colors_Black:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[30;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[30;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[30;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[30;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[30;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[30;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[30;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[30;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[30m");
      break;
    }
    break;
  case ANSI_Colors_Red:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[31;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[31;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[31;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[31;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[31;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[31;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[31;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[31;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[31m");
      break;
    }
    break;
  case ANSI_Colors_Green:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[32;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[32;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[32;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[32;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[32;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[32;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[32;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[32;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[32m");
      break;
    }
    break;
  case ANSI_Colors_Yellow:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[33;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[33;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[33;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[33;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[33;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[33;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[33;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[33;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[33m");
      break;
    }
    break;
  case ANSI_Colors_Blue:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[34;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[34;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[34;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[34;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[34;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[34;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[34;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[34;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[34m");
      break;
    }
    break;
  case ANSI_Colors_Magenta:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[35;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[35;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[35;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[35;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[35;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[35;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[35;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[35;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[35m");
      break;
    }
    break;
  case ANSI_Colors_Cyan:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[36;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[36;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[36;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[36;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[36;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[36;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[36;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[36;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[36m");
      break;
    }
    break;
  case ANSI_Colors_White:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[37;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[37;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[37;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[37;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[37;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[37;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[37;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[37;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[37m");
      break;
    }
    break;
  case ANSI_Colors_Default:
    switch (bg) {
    case ANSI_Colors_Black:
      EX_PRINT_STR("\x1b[39;40m");
      break;
    case ANSI_Colors_Red:
      EX_PRINT_STR("\x1b[39;41m");
      break;
    case ANSI_Colors_Green:
      EX_PRINT_STR("\x1b[39;42m");
      break;
    case ANSI_Colors_Yellow:
      EX_PRINT_STR("\x1b[39;43m");
      break;
    case ANSI_Colors_Blue:
      EX_PRINT_STR("\x1b[39;44m");
      break;
    case ANSI_Colors_Magenta:
      EX_PRINT_STR("\x1b[39;45m");
      break;
    case ANSI_Colors_Cyan:
      EX_PRINT_STR("\x1b[39;46m");
      break;
    case ANSI_Colors_White:
      EX_PRINT_STR("\x1b[39;47m");
      break;
    default:
        EX_PRINT_STR("\x1b[39m");
      break;
    }
    break;
  default:
    break;
  }
}

void ANSI_Graphics_Set(ANSI_Graphics_k graphics) {
if(graphics == ANSI_Graphics_Default) {
	return;
}
  EX_PRINT_STR("\x1b[");
  if (graphics & ANSI_Graphics_Reset) {
    EX_PRINT_STR("0");
    EX_PRINT_STR("m");
    return;
  }
  uint8_t cont = 0;
  if (graphics & ANSI_Graphics_Bold) {
    EX_PRINT_STR("1");
    cont++;
  }
  if (graphics & ANSI_Graphics_Underline) {
    if (cont)
      EX_PRINT_STR(";");
    EX_PRINT_STR("4");
    cont++;
  }
  if (graphics & ANSI_Graphics_Blink) {
    if (cont)
      EX_PRINT_STR(";");
    EX_PRINT_STR("5");
    cont++;
  }
  if (graphics & ANSI_Graphics_Invert) {
    if (cont)
      EX_PRINT_STR(";");
    EX_PRINT_STR("7");
    cont++;
  }
  if (graphics & ANSI_Graphics_Invisible) {
    if (cont)
      EX_PRINT_STR(";");
    EX_PRINT_STR("8");
    cont++;
  }
  if (graphics & ANSI_Graphics_Strikethrough) {
    if (cont)
      EX_PRINT_STR(";");
    EX_PRINT_STR("9");
    cont++;
  }
  if (graphics & ANSI_Graphics_Italic) {
    if (cont)
      EX_PRINT_STR(";");
    EX_PRINT_STR("3");
    cont++;
  }
  EX_PRINT_STR("m");
}

void ANSI_String_Print(ANSI_String_t *str) {
  ANSI_Graphics_Set(str->graphics);
  ANSI_Colors_Set(str->fg, str->bg);
  EX_PRINT_STR_LEN(str->buf, str->len);
  EX_PRINT_STR("\x1b[0m");
}

void ANSI_String_Set(ANSI_String_t *str) {
  ANSI_Graphics_Set(str->graphics);
  ANSI_Colors_Set(str->fg, str->bg);
}

void ANSI_Reset(void) { EX_PRINT_STR("\x1b[0m"); }




