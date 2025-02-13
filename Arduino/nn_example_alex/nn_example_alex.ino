//
// test the NN source card plugged in to J17 on a Rev2 SCOS board
// example code for Alex
//

// #define MEMORY_DUMP_DEBUG

// Pins on NN source card
#define SRCB_RUN 22
#define LED_EN 24
#define LED_STEP 25
#define FLOW_CTL 27

// Uart channel select
#define UART_MUX0 41
#define UART_MUX1 40

// SCOS board LEDs
#define LED1 A2
#define LED2 A3

byte* ram_wr( int addr, int led, unsigned dac, int en, int end);
int ram_rd( int addr, int& led, unsigned& dac, int& en, int& end);

void setup() {
  
  //---- setup pins and default values ----
  pinMode( LED1, OUTPUT);	// SCOS LED

  pinMode( SRCB_RUN, OUTPUT);
  digitalWrite( SRCB_RUN, HIGH);// SRCB Run - zero to reset MCU

  pinMode( LED_EN, OUTPUT);
  digitalWrite( LED_EN, LOW);	// default to UART mode

  pinMode( LED_STEP, OUTPUT);
  digitalWrite( LED_STEP, LOW);	// step pin default low

  pinMode( FLOW_CTL, OUTPUT);
  digitalWrite( FLOW_CTL, LOW);	// always allow data TX

  // set UART mux to port 2
  pinMode( UART_MUX0, OUTPUT);
  pinMode( UART_MUX1, OUTPUT);

  digitalWrite( UART_MUX0, LOW);
  digitalWrite( UART_MUX1, HIGH);

  Serial.begin( 115200);	// serial monitor
  Serial2.begin( 250000);	// UART for NN board

  delay(100);			// wait for NN board to boot up

  // write memory to turn each of 16 LEDs on, then off in sequence
#define NSTEPS 32

  for( int i=0; i<NSTEPS; i++) {	// loop over all LEDs
    ram_wr( 2*i,   i, 0xffff, 1, 0); // LED on
    ram_wr( 2*i+1, i, 0xffff, 0, 0); // LED off
    ram_wr( NSTEPS, 0, 0, 0, 1);	// end flag in memory
  }
  // WHY? does this have to be an additional word?
  
#ifdef MEMORY_DUMP_DEBUG
  // try to read and dump the memory
  digitalWrite( LED_EN, HIGH);
  digitalWrite( LED_EN, LOW);	// reset to state 0 before reading
#endif

  digitalWrite( LED_EN, 1);	// enable LEDs
}

void loop() {
  // pulse at 10Hz
  digitalWrite( LED_STEP, HIGH);
  digitalWrite( LED_STEP, LOW);
  delay(25);
//  digitalWrite( LED1, HIGH);
//  delay(100);
//  digitalWrite( LED1, LOW);
}


//
// write to RAM on NN source adapter
//   addr - memory address 0-1023
//   led  - LED channel 0-15
//   dac  - DAC (intensity) 0-65535
//   en   - enable LED this step (1=enable, 0=disable)
//   end  - marks the last step (1)
//
// returns pointer to raw memory data
//
byte* ram_wr( int addr, int led, unsigned dac, int en, int end) {

  static byte ramb[10];
  // force values to be in range
  addr &= 1023;
  led &= 15;

  digitalWrite( LED_EN, LOW);	// set to UART mode

  memset( ramb, 0, sizeof(ramb)); // zero the memory buffer
  ramb[0] = 255;		  // command byte
  // 2 bytes address, LSB first
  ramb[1] = (addr & 0xff);	// LSB of address
  ramb[2] = (addr >> 8) | 0b10110000; // MSB of address plus 'w' and offset
  // 4 bytes data, LSB first
  ramb[3] = dac & 0xff;	// DAC low byte
  ramb[4] = dac >> 8;	// DAC high byte
  ramb[5] = led;	// LED number
  ramb[6] = 0;
  if( !en)
    ramb[5] |= 0x10;	// LED disable this step
  if( end)
    ramb[6] |= 0x80;	// set 'END'

#ifdef MEMORY_DUMP_DEBUG
  // print memory word for debug
  Serial.println("Write");
  for( int i=0; i<7; i++) {
    Serial.print(ramb[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif

  Serial2.write( ramb, 7);
  return ramb;
}


//
// read memory location
//
int ram_rd( int addr, int& led, unsigned& dac, int& en, int& end) {
  static byte ram[] = { 255, 0, 48, 0, 0, 0, 0};
  static byte ramb[10];
  int nr;
  
  digitalWrite( LED_EN, LOW);	// set to UART mode

  ram[1] = addr & 0xff;
  ram[2] = ((addr >> 8) & 3) | 0x30;

  Serial2.write( ram, 7);
  nr = Serial2.readBytes( ramb, 10);

  if( nr != 7)
    return 1;

  addr = ramb[1] | ((ramb[2] & 0x3ff) << 8);
  led = ramb[5] & 15;
  dac = ramb[3] | (ramb[4] << 8);
  if( ramb[5] & 0x10)		// disable?
    en = 0;
  else
    en = 1;
  if( ramb[6] & 0x80)		// end?
    end = 1;
  else
    end = 0;
  return 0;
}
