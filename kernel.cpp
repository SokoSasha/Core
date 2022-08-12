__asm("jmp kmain");

#define VIDEO_BUF_PTR (0xb8000) 
#define IDT_TYPE_INTR (0x0E)
#define IDT_TYPE_TRAP (0x0F)

#define GDT_CS (0x8)
#define PIC1_PORT (0x20)
#define CURSOR_PORT (0x3D4)
#define VIDEO_WIDTH (80)
#define VIDEO_LENGTH (25)


struct idt_entry
{
	unsigned short base_lo;
	unsigned short segm_sel;
	unsigned char always0;
	unsigned char flags;
	unsigned short base_hi;
} __attribute__((packed));


struct idt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

typedef void (*intr_handler)();

struct idt_entry g_idt[256];
struct idt_ptr g_idtp;
unsigned int global_str = 0;
unsigned int global_pos = 0;
bool shift = false;
bool start = true;

void on_key(unsigned char scan_code);
void command_handler();
bool strcmp(unsigned char *str1, const char *str2);
int strlen_s(unsigned char *str);

void info();
void gcd(unsigned char* str);
void solve(unsigned char* str);
void lcm(unsigned char* str);
void div(unsigned char* str);
int char_to_int(unsigned char *str);
char *int_to_char(int integer);
void scroll();

void shutdown();
void shift_check(unsigned char scan_code);
void clean(bool chk);
void backspace();
void tab();
void enter();
void symbol(unsigned char scan_code);

void default_intr_handler();
void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr);
void intr_init();
void intr_start();
void intr_enable();
void intr_disable();
void out_str(int color, const char* ptr, unsigned int strnum);

void out_char(int color, unsigned char simbol);
void out_word(int color, const char* ptr);
void out_num(int num);
static inline void outw (unsigned int port, unsigned int data);

static inline unsigned char inb (unsigned short port);
static inline void outb (unsigned short port, unsigned char data);
void keyb_init();
void keyb_handler();
void keyb_process_keys();
void cursor_moveto(unsigned int strnum, unsigned int pos);

char scan_codes[] = 
{
    0, 
    0,
    '1','2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    0,
    0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    0,
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '<','>','+',
    0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,
    '*',
    0,
    ' ',
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0, 0,
    0,
    '+',
    0,
    0
};


char shift_char[] = 
{
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '=', '8',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '+', '*'
};

char currentColour = 0x07;
char colour = 0;
char colours[] = { 0x07, 0x0f, 0x0e, 0x0b, 0x09, 0x0a };

void clean(bool chk)
{	
	unsigned char *video_buf = (unsigned char*) VIDEO_BUF_PTR;
	for (int i = 0; i < VIDEO_WIDTH * VIDEO_LENGTH; i++){
		*(video_buf + i*2) = '\0';
	}
	global_str = 0;
	global_pos = 0;
	if(chk){
		out_str(currentColour, "# ", global_str);
		global_pos = 2;
	}
	cursor_moveto(global_str, global_pos);
}

void scroll(unsigned int strnm){
	unsigned char *video_buf = (unsigned char*) (VIDEO_BUF_PTR + VIDEO_WIDTH * 2);
	unsigned char *video_buf_new = (unsigned char*) VIDEO_BUF_PTR;
	for (int i = 0; i < VIDEO_WIDTH * VIDEO_LENGTH * 2; i++){
		*(video_buf_new + i) = *(video_buf + i);
	}
	for (int i = VIDEO_WIDTH * (VIDEO_LENGTH - 1) * 2; i < VIDEO_WIDTH * VIDEO_LENGTH * 2; i++){
		*(video_buf + i) = '\0';
	}
	global_str = VIDEO_LENGTH - 1;
	strnm = VIDEO_LENGTH - 1;
	cursor_moveto(global_str, global_pos);
}

void ChooseColour(){
	clean(0);
	int defColour = 0x08;
	global_pos = 1;
	global_str = 0;
	out_str(currentColour, "Choose colour:", global_str);
	global_pos = 1;
	out_str((currentColour == 0x07) ? 0x07 : defColour, (currentColour == 0x07) ? "  Gray" : "Gray", ++global_str);
	global_pos = 1;
	out_str((currentColour == 0x0f) ? 0x0f : defColour, (currentColour == 0x0f) ? "  White" : "White", ++global_str);
	global_pos = 1;
	out_str((currentColour == 0x0e) ? 0x0e : defColour, (currentColour == 0x0e) ? "  Yellow" : "Yellow", ++global_str);
	global_pos = 1;
	out_str((currentColour == 0x0b) ? 0x0b : defColour, (currentColour == 0x0b) ? "  Cian" : "Cian", ++global_str);
	global_pos = 1;
	out_str((currentColour == 0x09) ? 0x09 : defColour, (currentColour == 0x09) ? "  Magenta" : "Magenta", ++global_str);
	global_pos = 1;
	out_str((currentColour == 0x0a) ? 0x0a : defColour, (currentColour == 0x0a) ? "  Green" : "Green", ++global_str);
	cursor_moveto(VIDEO_WIDTH, VIDEO_LENGTH);
}


extern "C" int kmain()
{
	start = true;
	ChooseColour();

	intr_disable();
	intr_init();
	keyb_init();
	intr_start();
	intr_enable();
	
	while(1){
		asm("hlt");
	}

	return 0;
}

// Пустой обработчик прерываний. Другие обработчики могут быть реализованы по этому шаблону
void default_intr_handler()
{
	asm("pusha");
	// ... (реализация обработки)
	asm("popa; leave; iret");
}


void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr)
{
	unsigned int hndlr_addr = (unsigned int) hndlr;

	g_idt[num].base_lo = (unsigned short) (hndlr_addr & 0xFFFF);
	g_idt[num].segm_sel = segm_sel;
	g_idt[num].always0 = 0;
	g_idt[num].flags = flags;
	g_idt[num].base_hi = (unsigned short) (hndlr_addr >> 16);
}


void intr_init()
{
	int i;
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);

	for(i = 0; i < idt_count; i++)
		intr_reg_handler(i, GDT_CS, 0x80 | IDT_TYPE_INTR,
			default_intr_handler);
}


void intr_start()
{
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);

	g_idtp.base = (unsigned int) (&g_idt[0]);
	g_idtp.limit = (sizeof (struct idt_entry) * idt_count) - 1;

	asm("lidt %0" : : "m" (g_idtp) );
}

void intr_enable()
{
	asm("sti");
}

void intr_disable()
{
	asm("cli");
}


static inline unsigned char inb (unsigned short port) 
{
	unsigned char data;
	asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
	return data;
}


static inline void outb (unsigned short port, unsigned char data)
{
	asm volatile ("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline void outw (unsigned int port, unsigned int data)
{
	asm volatile ("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

void keyb_init()
{
	intr_reg_handler(0x09, GDT_CS, 0x80 | IDT_TYPE_INTR, keyb_handler);
	outb(PIC1_PORT + 1, 0xFF ^ 0x02);
}

void keyb_handler()
{
	asm("pusha");
	keyb_process_keys();
	outb(PIC1_PORT, 0x20);
	asm("popa; leave; iret");
}


void keyb_process_keys()
{
	if (inb(0x64) & 0x01)
	{
		unsigned char scan_code;
		unsigned char state;
		scan_code = inb(0x60);
		if (scan_code < 128)
			on_key(scan_code);
		else shift_check(scan_code);
	}
}


void cursor_moveto(unsigned int strnum, unsigned int pos)
{
	unsigned short new_pos = (strnum * VIDEO_WIDTH) + pos;
	outb(CURSOR_PORT, 0x0F);
	outb(CURSOR_PORT + 1, (unsigned char)(new_pos & 0xFF));
	outb(CURSOR_PORT, 0x0E);
	outb(CURSOR_PORT + 1, (unsigned char)( (new_pos >> 8) & 0xFF));
}

void out_str(int color, const char* ptr, unsigned int strnum)
{
	if (strnum >= VIDEO_LENGTH){
		char nm = VIDEO_LENGTH - strnum + 1;
		for (int i = 0; i < nm; i++) scroll(strnum);
		strnum = VIDEO_LENGTH - 1;
	}
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += VIDEO_WIDTH * 2 * strnum;
	while (*ptr)
	{
		video_buf[0] = (unsigned char) *ptr;
		video_buf[1] = color;
		video_buf += 2;
		ptr++;
	}
}

void out_word(int color, const char* ptr)
{
	if (global_str >= VIDEO_LENGTH){
		char nm = VIDEO_LENGTH - global_str + 1;
		for (int i = 0; i < nm; i++) scroll(global_str);
	}
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 2*(VIDEO_WIDTH * global_str + global_pos);
	while (*ptr)
	{
		video_buf[0] = (unsigned char) *ptr;
		video_buf[1] = color;
		video_buf += 2;
		global_pos++;
		ptr++;
	}
}

void out_char(int color, unsigned char simbol)
{
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 2*(global_str * VIDEO_WIDTH + global_pos);
	video_buf[0] = simbol;
	video_buf[1] = color;
	cursor_moveto(global_str, ++global_pos);
}

void shift_check(unsigned char scan_code)
{
	if (scan_code == 170 || scan_code == 182)
		shift = false;
}

void on_key(unsigned char scan_code)
{
	if (start){
		if (scan_code == 80) {
			colour++;
			if (colour == 6) colour = 0;
			currentColour = colours[colour];
			ChooseColour();
		}
		if (scan_code == 72) {
			colour--;
			if (colour == -1) colour = 5;
			currentColour = colours[colour];
			ChooseColour();
		}
	}
	if (start && scan_code != 72 && scan_code != 80) 
	{
		start = false;
		clean(1);
		return;			
	}
	if (scan_code == 14) backspace();
	else if (scan_code == 15) tab();
	else if (scan_code == 28) enter();
	else if (scan_code == 42 || scan_code == 54)
		shift = true;
	else if (global_pos < 42 && scan_codes[scan_code] != 0) symbol(scan_code);
}


void backspace()
{
	if (global_pos > 2){
		unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
		video_buf += 2*(global_str*VIDEO_WIDTH + global_pos - 1);
		video_buf[0] = '\0';
		cursor_moveto(global_str, --global_pos);
	}
}


void tab()
{
	if (global_pos < 38)
	{
		global_pos += 4;
		cursor_moveto(global_str, global_pos);	
	}
}


void enter()
{
	command_handler();
	out_str(currentColour, "# ", ++global_str);
	global_pos = 2;
	cursor_moveto(global_str, global_pos);
}


void symbol(unsigned char scan_code)
{
	char c = scan_codes[(unsigned int)scan_code];
	if (shift == false) out_char(currentColour, c);
	else
	{
		int i;
		for (i = 0; i < 28; i++)
			if (c == shift_char[i])
			break;
		out_char(currentColour, shift_char[i + 28]);
	}
}


void command_handler()
{
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += 2 * (global_str * VIDEO_WIDTH + 2);

	if (strcmp(video_buf, "info")) info();
	else if (strcmp(video_buf, "gcd ")) gcd(video_buf + 8);
	else if (strcmp(video_buf, "solve ")) solve(video_buf + 12);
	else if (strcmp(video_buf, "lcm ")) lcm(video_buf + 8);
	else if (strcmp(video_buf, "div ")) div(video_buf + 8);
	else if (strcmp(video_buf, "shutdown")) outw(0x604, 0x2000);
	else out_str(0x0c, "Wrong command", ++global_str);
}

bool strcmp(unsigned char *str1, const char *str2)
{
    while (*str1 != '\0' && *str1 != ' ' && *str2 != '\0' && *str1 == *str2) 
    {
      str1+=2;
      str2++;
    }

   if (*str1 == *str2) return true; 
   return false;
}

int strlen_s(unsigned char *str)
{
	int i = 0;
	while(*str != '\0') { str++; i++;}
	return i;
}


void info()
{
	global_pos = 0;
	out_str(currentColour, "SolverOS", ++global_str);
	out_str(currentColour, "YASM (AT&T)", ++global_str);
	out_str(currentColour, "OS: Linux", ++global_str);
	switch(currentColour)
	{
		case 0x08: { out_str(currentColour, "Colour: gray", ++global_str); break; }
		case 0x0f: { out_str(currentColour, "Colour: white", ++global_str); break; }
		case 0x0e: { out_str(currentColour, "Colour: yellow", ++global_str); break; }
		case 0x0b: { out_str(currentColour, "Colour: cian", ++global_str); break; }
		case 0x09: { out_str(currentColour, "Colour: magenta", ++global_str); break; }
		case 0x07: { out_str(currentColour, "Colour: green", ++global_str); break; }
	}
	out_str(currentColour, "Made by Sokolova Alexandra, gr. 4851001/00001", ++global_str);
}

int char_to_int(unsigned char *str){
	int res = 0;
	int sz = strlen_s(str);
	if (str[0]=='-'){
		if (sz == 1) res = 1;
		for (int i = 1; i < sz; i ++){
			res = res * 10 + (str[i] - '0');
		}
		res = 0 - res;
	}
	else if (str[0]=='+'){
		if (sz == 1) res = 1;
		for (int i = 1; i < sz; i ++){
			res = res * 10 + (str[i] - '0');
		}
	}
	else{
		if (sz == 0) res = 1;
		for (int i = 0; i < sz; i ++){
			res = res * 10 + (str[i] - '0');
		}
	}
	return res;
}

char *int_to_char(int integer){
	int add = 1;
	static char ch[40] = {'\0'};

	if (integer < 0) {
		integer = 0 - integer;
		add = 0;
	}

	if (!integer) return "0";

	int i = 0;
	int temp = integer;
	while (temp) {
		i++;
		temp /= 10;
	}
	if (!add) i++;
	ch[i--] = '\0';

	while (i >= 0) {
		ch[i--] = (integer % 10) + '0';
		integer /= 10;
	}
	if (!add) ch[0] = '-';
	return ch;
}

void solve(unsigned char *str){
	int i = 0, j = 0;
	unsigned char num1[40], num2[40], num3[40];
	for (int k = 0; k < 40; k++){
		num1[k] = '\0';
		num2[k] = '\0';
		num3[k] = '\0';
	}
	while (str[i*2] != 'x' && str[i*2]){	
		num1[i] = str[i*2];
		i++;
	}
	i++;
	while(str[i*2] && str[i*2] != '='){
		num2[j] = str[i*2];
		i++;
		j++;
	}
	i++;
	j = 0;
	while(str[i*2]){
		num3[j] = str[i*2];
		i++;
		j++;
	}
	int dig1 = 1;
	dig1 = char_to_int(num1);
	if (dig1 != 0 && (*num2 != '\0' || *num3 != '\0')){
		int dig2 = 0;
		if (*num2 != '\0') dig2 = char_to_int(num2);
		int dig3 = 0;
		if (*num3 != '\0') dig3 = char_to_int(num3);
		int res = (dig3 - dig2) /dig1;
		if (dig1 * res + dig2 != dig3){
			float resfl = ((float)dig3 - (float)dig2) / (float)dig1;
			const char *chr1 = int_to_char((int)resfl);
			global_pos = 0;
			global_str++;
			out_word(currentColour, "Result: x = ");
			if (resfl < 0 && resfl > -1) out_word(currentColour, "-");
			out_word(currentColour, chr1);
			out_word(currentColour, ".");
			resfl = (resfl - (int)resfl)*1000000;
			if (resfl < 0) resfl = resfl*(-1);
			if ((int)resfl % 10 >= 5) resfl = resfl + 10;
			resfl = resfl / 10;
			const char *chr2 = int_to_char((int)resfl);
			for (int k = 5; k > strlen_s((unsigned char *)chr2); k--)
				out_word(currentColour, "0");
			out_word(currentColour, chr2);
		}
		else {
		const char *chr = int_to_char(res);
		global_pos = 0;
		global_str++;
		out_word(currentColour, "Result: x = ");
		out_word(currentColour, chr);
		}
	}
	else out_str(0x0c, "Wrong command", ++global_str);
}

int nod(int m, int n)
{
    return n ? nod(n, m % n) : m;
}

void gcd(unsigned char *str){
	int i = 0, j = 0;
	unsigned char gcd1[40], gcd2[40];
	for (int k = 0; k < 40; k++){
		gcd1[k] = '\0';
		gcd2[k] = '\0';
	}
	while (str[i*2] != ' ' && str[i*2]){	
		gcd1[i] = str[i*2];
		i++;
	}
	i++;
	while(str[i*2]){
		gcd2[j] = str[i*2];
		i++;
		j++;
	}
	if (*gcd1 != '\0' && *gcd2 != '\0' && *gcd1 != '0' && *gcd2 != '0'){
		int dig1 = 1;
		dig1 = char_to_int(gcd1);
		int dig2 = 1;
		dig2 = char_to_int(gcd2);
		int gcd_res = nod(dig1, dig2);
		const char *chr = int_to_char(gcd_res);
		global_pos = 0;
		global_str++;
		out_word(currentColour, "Result: ");
		out_word(currentColour, chr);
	}
	else out_str(0x0c, "Wrong command", ++global_str);
}

void lcm(unsigned char *str){
	int i = 0, j = 0;
	unsigned char lcm1[40], lcm2[40];
	for (int k = 0; k < 40; k++){
		lcm1[k] = '\0';
		lcm2[k] = '\0';
	}
	while (str[i*2] != ' ' && str[i*2]){	
		lcm1[i] = str[i*2];
		i++;
	}
	i++;
	while(str[i*2]){
		lcm2[j] = str[i*2];
		i++;
		j++;
	}
	if (*lcm1 != '\0' && *lcm2 != '\0' && *lcm1 != '0' && *lcm2 != '0'){
		int dig1 = 1;
		dig1 = char_to_int(lcm1);
		int dig2 = 1;
		dig2 = char_to_int(lcm2);
		int lcm_res = (dig1 * dig2) / nod(dig1, dig2);
		const char *chr = int_to_char(lcm_res);
		global_pos = 0;
		global_str++;
		out_word(currentColour, "Result: ");
		out_word(currentColour, chr);
	}
	else out_str(0x0c, "Wrong command", ++global_str);
}

void div(unsigned char *str){
	int i = 0, j = 0;
	unsigned char div1[40], div2[40];
	for (int k = 0; k < 40; k++){
		div1[k] = '\0';
		div2[k] = '\0';
	}
	while (str[i*2] != ' ' && str[i*2]){	
		div1[i] = str[i*2];
		i++;
	}
	i++;
	while(str[i*2]){
		div2[j] = str[i*2];
		i++;
		j++;
	}
	if (*div1 != '\0' && *div2 != '\0' && *div1 != '0' && *div2 != '0'){
		int dig1 = 1;
		dig1 = char_to_int(div1);
		int dig2 = 1;
		dig2 = char_to_int(div2);
		float div_resfl = (float)dig1 / (float)dig2;
		const char *chr1 = int_to_char((int)div_resfl);
		global_pos = 0;
		global_str++;
		out_word(currentColour, "Result: ");
		out_word(currentColour, chr1);
		out_word(currentColour, ".");
		div_resfl = (div_resfl - (int)div_resfl)*1000000;
		if ((int)div_resfl % 10 >= 5) div_resfl = div_resfl + 10;
		div_resfl = div_resfl / 10;
		const char *chr2 = int_to_char((int)div_resfl);
		for (int k = 5; k > strlen_s((unsigned char *)chr2); k--)
			out_word(currentColour, "0");
		out_word(currentColour, chr2);
	}
	else out_str(0x0c, "Wrong command", ++global_str);
}
