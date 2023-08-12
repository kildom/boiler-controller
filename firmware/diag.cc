#include "diag.hh"
#include "menu.hh"


static Diag::Mode mode = Diag::LOG;
static int lost = 0;

static void writeBuf(const char* text, int size)
{
	const char* end = text + size;
	while (text < end) {
		diag_append(*text++);
	}
}

void Diag::log(const char* text)
{
	if (mode != LOG) {
		lost++;
		return;
	}
	update();
	int free = diag_free();
	int len = strlen(text);
	if (free < len + 2 || lost > 0) {
		if (free > 5 && lost == 0) {
			writeBuf(text, free - 5);
			writeBuf("...\r\n", 5);
		}
		lost++;
	} else {
		writeBuf(text, len);
		writeBuf("\r\n", 2);
	}
	diag_send();
}

void Diag::write(const char* data, int size, Mode requestMode)
{
	if (size < 0) {
		size = strlen(data);
	}
	if (mode == requestMode && diag_free() >= size ) {
		writeBuf(data, size);
		diag_send();
	}
}

void Diag::write(uint8_t data, Mode requestMode)
{
	if (mode == requestMode && diag_free() > 1) {
		diag_append(data);
		diag_send();
	}
}


void Diag::update()
{
	if (mode == LOG && lost > 0 && diag_free() > 12 + 13) {
		static char lostStr[12 + 13 + 1];
		sprintf(lostStr, "\r\n[lost %d]\r\n", lost);
		lost = 0;
		log(lostStr);
	}
}


void diag_event(uint8_t* data, int size)
{
	uint8_t* end = data + size;

	while (data < end) {
		if ((*data == '\r' || *data == '\n') && mode != Diag::MENU) {
			mode = Diag::MENU;
		} else if (*data == '#' && mode != Diag::COMM) {
			mode = Diag::COMM;
			continue;
		} else if (*data == '!' && mode != Diag::LOG) {
			mode = Diag::LOG;
			Diag::write("\r\nLOG\r\n", -1, Diag::LOG);
			continue;
		}

		if (mode == Diag::MENU) {
			Menu::write(*data);
		} else if (mode == Diag::COMM) {
			//TODO: pass to comm decoder: Comm::write();
		}
		data++;
	}
}
