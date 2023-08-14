
#include "menu.hh"
#include "diag.hh"

static const int MAX_LINE_LEN = 128;

static char line[MAX_LINE_LEN];
static int lineIndex = 0;


struct MenuItem {
	const char* name;
	bool (*func)(const MenuItem* item, char data);
	const void* sub;
	const char* (*nameFunc)(const MenuItem* item);
};

static bool menuFunc(const MenuItem* item, char data);

static bool nullFunc(const MenuItem* item, char data)
{
	return false;
}

static bool isen = false;
static int x = 1;

static bool toggleFunc(const MenuItem* item, char data)
{
	isen = !isen;
	return false;
}

static const char* myNameFunc(const MenuItem* item)
{
	return isen ? "yes" : "no";
}

static void showMenu();
static void showPath();

static bool inputFunc(const MenuItem* item, char data)
{
	if (data == 0) {
		sprintf(line, "%d", x);
		lineIndex = strlen(line);
		showPath();
		Diag::write(line, lineIndex, Diag::MENU);
		return true;
	} else if (data == '\r' || data == '\n') {
		if (lineIndex == 0) return false;
		line[lineIndex] = 0;
		x = atoi(line);
		return false;
	} else if (data >= 32 && data < 127 && lineIndex < MAX_LINE_LEN - 1) {
		line[lineIndex++] = data;
		Diag::write(data, Diag::MENU);
		return true;
	} else if ((data == 8) || (data == 127) && lineIndex > 0) {
		Diag::write("\x08 \x08", 3, Diag::MENU);
		lineIndex--;
		return true;
	} else {
		return true;
	}
}

static const char* inputNameFunc(const MenuItem* item)
{
	static char buf[12];
	sprintf(buf, "%d", x);
	return buf;
}


static const MenuItem optionList[] = {
	{ "Sub1: ", toggleFunc, NULL, myNameFunc },
	{ "Sub2", nullFunc },
	{ "Sub3", nullFunc },
	{ "Value: ", inputFunc, NULL, inputNameFunc },
	{}
};

static uint32_t write_state = 0;

static const char* writeFuncStage(const MenuItem* item)
{
	static char buf[12];
	sprintf(buf, "0x%08X", (unsigned int)write_state);
	return buf;
}

static uint8_t the_source[] = "This is the text that should be used to test programming of store pages. Is it big enough?";
static uint8_t the_temp[128];

static bool writeFunc(const MenuItem* item, char data)
{
	int size = strlen((char*)the_source) + 1;
	store_write(&write_state, 0, the_source, size);
	return false;
}

static bool readFunc(const MenuItem* item, char data)
{
	store_read(0, the_temp, strlen((char*)the_source) + 1);
	the_temp[strlen((char*)the_source)] = 0;
	Diag::write("\r\n", -1, Diag::MENU);
	Diag::write((char*)the_temp, -1, Diag::MENU);
	Diag::write("\r\n", -1, Diag::MENU);
	return false;
}

static const MenuItem rootList[] = {
	{ "Option 1", nullFunc },
	{ "Option 2", menuFunc, optionList },
	{ "Write: ", writeFunc, NULL, writeFuncStage },
	{ "Read", readFunc },
	{}
};

static const MenuItem root = { "root", menuFunc, rootList };

static const MenuItem* menuStack[20] = { &root };
static int menuStackLast = 0;

static char prevChar = 0;

static void showPath()
{
	const MenuItem *sub;
	Diag::write("\r\n > ", 5, Diag::MENU);
	for (int i = 1; i <= menuStackLast; i++) {
		sub = menuStack[i];
		Diag::write(sub->name, -1, Diag::MENU);
		if (sub->nameFunc) {
			Diag::write(sub->nameFunc(sub), -1, Diag::MENU);
		}
		Diag::write(" > ", 3, Diag::MENU);
	}
}

static void showMenu()
{
	const MenuItem *sub;
	if (menuStack[menuStackLast]->func != menuFunc) return;
	sub = (const MenuItem *)menuStack[menuStackLast]->sub;
	char index = '1';
	Diag::write("\r\n------------------------------------------------------\r\n\r\n  ", -1, Diag::MENU);
	while (sub->name) {
		Diag::write(index, Diag::MENU);
		Diag::write(". ", -1, Diag::MENU);
		Diag::write(sub->name, -1, Diag::MENU);
		if (sub->nameFunc) {
			Diag::write(sub->nameFunc(sub), -1, Diag::MENU);
		}
		if (sub->func != menuFunc) {
			Diag::write("\r\n  ", -1, Diag::MENU);
		} else {
			Diag::write(" >>\r\n  ", -1, Diag::MENU);
		}
		sub++;
		index++;
		if (index > '9') {
			index = 'A';
		}
	}
	showPath();
}

static const int KEY_NONE = -1;
static const int KEY_ENTER = -2;
static const int KEY_BACKSPACE = -3;

static int keyInput(char data)
{
	if (data == 0 || data == '\r' || (data == '\n' && prevChar != '\r')) {
		return KEY_ENTER;
	} else if ((data == 8) || (data == 127)) {
		return KEY_BACKSPACE;
	} else if (data >= '1' && data <= '9') {
		return data - '1';
	} else if (data >= 'A' && data <= 'Z') {
		return data - 'A' + 9;
	} else if (data >= 'a' && data <= 'z') {
		return data - 'a' + 9;
	} else {
		return KEY_NONE;
	}
}

static bool selectMenu(const MenuItem* item, int selected, char echoChar)
{
	int index = 0;
	const MenuItem* sub = (const MenuItem*)item->sub;
	while (sub->name) {
		if (index == selected) {
			menuStack[++menuStackLast] = sub;
			return sub->func(sub, 0);
		}
		sub++;
		index++;
	}
	showPath();
	return true;
}

static bool menuFunc(const MenuItem* item, char data)
{
	int key = keyInput(data);
	if (key == KEY_ENTER) {
		showMenu();
		return true;
	} else if (key == KEY_BACKSPACE) {
		return false;
	} else if (key == KEY_NONE) {
		return true;
	} else {
		Diag::write(data, Diag::MENU);
		return selectMenu(item, key, data);
	}
}


void Menu::write(uint8_t data)
{
	const MenuItem* top = menuStack[menuStackLast];
	bool stay = top->func(top, data);
	if (!stay && menuStackLast > 0) {
		menuStackLast--;
		menuStack[menuStackLast]->func(menuStack[menuStackLast], 0);
	}
	prevChar = data;
}

