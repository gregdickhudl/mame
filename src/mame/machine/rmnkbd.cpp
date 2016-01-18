// license:BSD-3-Clause
// copyright-holders:Carl
#include "machine/rmnkbd.h"

rmnimbus_keyboard_device::rmnimbus_keyboard_device(const machine_config& mconfig, std::string tag, device_t* owner, UINT32 clock) :
	serial_keyboard_device(mconfig, RMNIMBUS_KEYBOARD, "RM Nimbus Keyboard", tag, owner, 0, "rmnimbus_keyboard", __FILE__),
	m_io_kbd8(*this, "TERM_LINE8"),
	m_io_kbd9(*this, "TERM_LINE9"),
	m_io_kbda(*this, "TERM_LINEA")
{
}

UINT8 rmnimbus_keyboard_device::keyboard_handler(UINT8 last_code, UINT8 *scan_line)
{
	int i = *scan_line, j;
	UINT8 code = 0, change;

	if (i == 0) code = m_io_kbd0->read();
	else
	if (i == 1) code = m_io_kbd1->read();
	else
	if (i == 2) code = m_io_kbd2->read();
	else
	if (i == 3) code = m_io_kbd3->read();
	else
	if (i == 4) code = m_io_kbd4->read();
	else
	if (i == 5) code = m_io_kbd5->read();
	else
	if (i == 6) code = m_io_kbd6->read();
	else
	if (i == 7) code = m_io_kbd7->read();
	else
	if (i == 8) code = m_io_kbd8->read();
	else
	if (i == 9) code = m_io_kbd9->read();
	else
	if (i == 10) code = m_io_kbda->read();

	*scan_line = (*scan_line + 1) % 11;
	code = ~code;

	if(m_state[i] == code)
		return 0;

	change = (m_state[i] ^ code);

	for(j = 0; j < 8; j++)
		if((change >> j) == 1) break;

	change = (1 << j);
	m_state[i] ^= change;

	return ((change & code) ? 0 : 0x80) | ((i << 3) + j);
}

static INPUT_PORTS_START( rmnimbus_keyboard )
	PORT_START("TERM_LINE0") /* Key row 0 scancodes 00..07 */
	//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC")    PORT_CODE(KEYCODE_ESC)          PORT_CHAR(0x1B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")      PORT_CODE(KEYCODE_1)            PORT_CHAR('1')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")      PORT_CODE(KEYCODE_2)            PORT_CHAR('2')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")      PORT_CODE(KEYCODE_3)            PORT_CHAR('3')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")      PORT_CODE(KEYCODE_4)            PORT_CHAR('4')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")      PORT_CODE(KEYCODE_5)            PORT_CHAR('5')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")      PORT_CODE(KEYCODE_6)            PORT_CHAR('6')

	PORT_START("TERM_LINE1") /* Key row 1 scancodes 08..0F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")      PORT_CODE(KEYCODE_7)            PORT_CHAR('7')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")      PORT_CODE(KEYCODE_8)            PORT_CHAR('8')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")      PORT_CODE(KEYCODE_9)            PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")      PORT_CODE(KEYCODE_0)            PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")      PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")      PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BSPACE") PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(0x08)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB")    PORT_CODE(KEYCODE_TAB)          PORT_CHAR(0x09)

	PORT_START("TERM_LINE2") /* Key row 2 scancodes 10..17 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")      PORT_CODE(KEYCODE_Q)            PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")      PORT_CODE(KEYCODE_W)            PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")      PORT_CODE(KEYCODE_E)            PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")      PORT_CODE(KEYCODE_R)            PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")      PORT_CODE(KEYCODE_T)            PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")      PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")      PORT_CODE(KEYCODE_U)            PORT_CHAR('U')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")      PORT_CODE(KEYCODE_I)            PORT_CHAR('I')

	PORT_START("TERM_LINE3") /* Key row 3 scancodes 18..1F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")      PORT_CODE(KEYCODE_O)            PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")      PORT_CODE(KEYCODE_P)            PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")      PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]")      PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")  PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(0x0D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")   PORT_CODE(KEYCODE_LCONTROL)     PORT_CODE(KEYCODE_RCONTROL) // Ether control
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")      PORT_CODE(KEYCODE_A)            PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")      PORT_CODE(KEYCODE_S)            PORT_CHAR('S')

	PORT_START("TERM_LINE4") /* Key row 4 scancodes 20..27 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")      PORT_CODE(KEYCODE_D)            PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")      PORT_CODE(KEYCODE_F)            PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")      PORT_CODE(KEYCODE_G)            PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")      PORT_CODE(KEYCODE_H)            PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")      PORT_CODE(KEYCODE_J)            PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")      PORT_CODE(KEYCODE_K)            PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")      PORT_CODE(KEYCODE_L)            PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")      PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';')

	PORT_START("TERM_LINE5") /* Key row 5 scancodes 28..2F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("#")      PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BACKSLASH") PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('\\')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TILDE")  PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('`')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")      PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")      PORT_CODE(KEYCODE_X)            PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")      PORT_CODE(KEYCODE_C)            PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")      PORT_CODE(KEYCODE_V)            PORT_CHAR('V')


	PORT_START("TERM_LINE6") /* Key row 6 scancodes 30..37 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")      PORT_CODE(KEYCODE_B)            PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")      PORT_CODE(KEYCODE_N)            PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")      PORT_CODE(KEYCODE_M)            PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")      PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")      PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")      PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PRSCR")  PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR('*')

	PORT_START("TERM_LINE7") /* Key row 7 scancodes 38..3F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALT")    PORT_CODE(KEYCODE_LALT)         PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")  PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS")   PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")     PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")     PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")     PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")     PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")     PORT_CODE(KEYCODE_F5)


	PORT_START("TERM_LINE8") /* Key row 8 scancodes 40..47 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6")     PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7")     PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8")     PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F9")     PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F10")    PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NUMLK")  PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SCRLK")  PORT_CODE(KEYCODE_SCRLOCK)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP7")    PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR('7')

	PORT_START("TERM_LINE9") /* Key row 9 scancodes 48..4F */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP8")    PORT_CODE(KEYCODE_8_PAD)        //PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP9")    PORT_CODE(KEYCODE_9_PAD)        //PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-")    PORT_CODE(KEYCODE_MINUS_PAD)    //PORT_CHAR('-')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP4")    PORT_CODE(KEYCODE_4_PAD)        //PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP5")    PORT_CODE(KEYCODE_5_PAD)        //PORT_CHAR('5')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP6")    PORT_CODE(KEYCODE_6_PAD)        //PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP+")    PORT_CODE(KEYCODE_PLUS_PAD)     //PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP1")    PORT_CODE(KEYCODE_1_PAD)        //PORT_CHAR('1')

	PORT_START("TERM_LINEA") /* Key row 10 scancodes 50..57 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP2")    PORT_CODE(KEYCODE_2_PAD)        //PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP3")    PORT_CODE(KEYCODE_3_PAD)        //PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP0")    PORT_CODE(KEYCODE_0_PAD)        //PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP.")    PORT_CODE(KEYCODE_DEL_PAD)      //PORT_CHAR('.')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP5")    PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR('5')
	//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP6")    PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR('6')
	//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP+")    PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR('+')
	//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP1")    PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR('1')

	PORT_START("TERM_LINEC")

	PORT_START("RS232_TXBAUD")
	PORT_CONFNAME(0xff, RS232_BAUD_9600, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_BAUD_9600, "9600")

	PORT_START("RS232_STARTBITS")
	PORT_CONFNAME(0xff, RS232_STARTBITS_1, "Start Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_STARTBITS_1, "1")

	PORT_START("RS232_DATABITS")
	PORT_CONFNAME(0xff, RS232_DATABITS_8, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_DATABITS_8, "8")

	PORT_START("RS232_PARITY")
	PORT_CONFNAME(0xff, RS232_PARITY_NONE, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_PARITY_NONE, "None")

	PORT_START("RS232_STOPBITS")
	PORT_CONFNAME(0xff, RS232_STOPBITS_1, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_STOPBITS_1, "1")
INPUT_PORTS_END

ioport_constructor rmnimbus_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rmnimbus_keyboard);
}

void rmnimbus_keyboard_device::device_start()
{
	serial_keyboard_device::device_start();
	memset(m_state, '\0', sizeof(m_state));
}

const device_type RMNIMBUS_KEYBOARD = &device_creator<rmnimbus_keyboard_device>;
