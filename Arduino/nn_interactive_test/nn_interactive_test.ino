//
// test the NN source card plugged in to J17 on a Rev2 SCOS board
//

// Pins on BZ source card
#define SRCB_RUN 22
#define LED_EN 24
#define LED_STEP 25
#define FLOW_CTL 27

#define UART_MUX0 41
#define UART_MUX1 40

// maximum number of command arguments to parse
#define MAXARG 10

// these are global so they are included in the report of variable size use
static char buff[80];
static char* argv[MAXARG];
static unsigned iargv[MAXARG];
static int argc;

void pbuff( byte *buff, int nc);
void ram_wr( int addr, int led, unsigned dac, int en, int end);

void setup() {
  // SRCB Run - zero to reset MCU
  pinMode( SRCB_RUN, OUTPUT);
  digitalWrite( SRCB_RUN, HIGH);

  pinMode( LED_EN, OUTPUT);
  digitalWrite( LED_EN, LOW);	// default to UART mode

  pinMode( LED_STEP, OUTPUT);
  digitalWrite( LED_STEP, LOW);

  pinMode( FLOW_CTL, OUTPUT);
  digitalWrite( FLOW_CTL, LOW);	// always allow data TX

  // set UART mux to port 2
  pinMode( UART_MUX0, OUTPUT);
  pinMode( UART_MUX1, OUTPUT);

  digitalWrite( UART_MUX0, LOW);
  digitalWrite( UART_MUX1, HIGH);

  Serial.begin( 115200);
  Serial2.begin( 250000);

  Serial2.setTimeout( 100);
  Serial.setTimeout( 2000);
}

byte ramr[] = { 255, 0, 48, 0, 0, 0, 0};
byte ramd[10];

int nr;

void loop() {

  // send a prompt, wait for it to be transmitted
  Serial.write(">");    
  Serial.flush();
  
  // read a ststring into buff with editing
  my_gets( buff, sizeof(buff));
  
  // echo the string read
  Serial.println( buff);

  // parse into text and integer tokens (see parse.ino)
  argc = parse( buff, argv, iargv, MAXARG);

  // run command
  switch( toupper( *argv[0])) {
  case 'H':
    Serial.println("W <addr> <led> <dac> [E] [D]");
    Serial.println("  addr 0-1024 led 0-15 E=en D=end");
    Serial.println("R <addr> [<count>]");
    Serial.println("E <0/1>  set LED enable (1=en 0=disable for RAM access)");
    Serial.println("S        step to next memory lcn");
    break;
  case 'W':			// write to RAM all bytes
    if( argc > 3) {
      digitalWrite( LED_EN, LOW);	// set to UART mode
      memset( ramd, 0, sizeof(ramd));
      int addr = iargv[1] & 1023;
      int led = iargv[2] & 15;
      int dac = iargv[3];
      ramd[0] = 255;		// command byte
      // 2 bytes address, LSB first
      ramd[1] = (addr & 0xff);	// LSB of address
      ramd[2] = (addr >> 8) | 0b10110000; // MSB of address plus 'w' and offset
      // 4 bytes data, LSB first
      ramd[3] = dac & 0xff;	// DAC low byte
      ramd[4] = dac >> 8;	// DAC high byte
      ramd[5] = led;			  // LED 2
      ramd[6] = 0;
      if( argc > 4) {		// check for 'E' (enable) and 'N' (end)
	for( int i=4; i<argc; i++)
	  switch( toupper( *argv[i])) {
	  case 'D':
	    ramd[5] |= 0x10;	// LED disable this step
	    break;
	  case 'E':
	    ramd[6] |= 0x80;	// set 'END'
	    break;
	  }
      }
      pbuff( ramd, 7);
      Serial2.write( ramd, 7);
    }
    break;
  case 'R':
    digitalWrite( LED_EN, LOW);	// set to UART mode
    ramr[1] = iargv[1];
    Serial2.write( ramr, 7);
    nr = Serial2.readBytes( ramd, 10);
    if( nr)
      pbuff( ramd, nr);
    break;
  case 'E':			// enable LEDs 0=off 1=on
    if( iargv[1])
      Serial.println("LEDs enabled");
    else
      Serial.println("LEDs disabled");
    digitalWrite( LED_EN, iargv[1]);
    break;
  case 'S':
    Serial.println("Step memory");
    digitalWrite( LED_STEP, HIGH);
    digitalWrite( LED_STEP, LOW);
  }
}


void pbuff( byte *b, int nc) {
  for( int i=0; i<nc; i++) {
    snprintf( buff, sizeof(buff), " %02x", b[i]);
    Serial.print( buff);
  }
  Serial.println();
}
 

//
// write to RAM on NN source adapter
//   addr - memory address 0-1023
//   led  - LED channel 0-15
//   dac  - DAC (intensity) 0-65535
//   en   - enable LED this step (1=enable, 0=disable)
//   end  - marks the last step (1)
//
void ram_wr( int addr, int led, unsigned dac, int en, int end) {

  byte ramb[10];

  digitalWrite( LED_EN, LOW);	// set to UART mode
  memset( ramb, 0, sizeof(ramb));
  ramb[0] = 255;		// command byte
  // 2 bytes address, LSB first
  ramb[1] = (addr & 0xff);	// LSB of address
  ramb[2] = (addr >> 8) | 0b10110000; // MSB of address plus 'w' and offset
  // 4 bytes data, LSB first
  ramb[3] = dac & 0xff;	// DAC low byte
  ramb[4] = dac >> 8;	// DAC high byte
  ramb[5] = led;			  // LED 2
  ramb[6] = 0;
  if( argc > 4) {		// check for 'E' (enable) and 'N' (end)
    for( int i=4; i<argc; i++)
      switch( toupper( *argv[i])) {
      case 'D':
	ramb[5] |= 0x10;	// LED disable this step
	break;
      case 'E':
	ramb[6] |= 0x80;	// set 'END'
	break;
      }
  }
  pbuff( ramb, 7);
  Serial2.write( ramb, 7);
    
}
