#include "color.h"

const char* GetColor(COLOR _color)
{
	switch (_color) {
	case BLACK: return "\033[1;30m";
	case RED: return "\033[1;31m";
	case GREEN: return "\033[1;32m";
	case YELLOW: return "\033[1;33m";
	case BLUE: return "\033[1;34m";
	case PURPLE: return "\033[1;35m";
	case CYAN: return "\033[1;36m";
	case WHITE: return "\033[1;37m";
	}
}
