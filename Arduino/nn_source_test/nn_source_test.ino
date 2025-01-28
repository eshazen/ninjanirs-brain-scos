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
}

byte ram1[] = { 255, 0, 176, 208, 7, 0, 0};
byte ram2[] = { 255, 1, 176, 184, 11, 1, 0};
byte ram3[] = { 255, 2, 176, 0, 0, 16, 128};

byte ramr[] = { 255, 0, 48, 0, 0, 0, 0};
byte ramd[10];

int nr;

void loop() {
  Serial2.write( ram1, sizeof(ram1));
  Serial2.write( ram2, sizeof(ram2));
  Serial2.write( ram3, sizeof(ram3));

  // read back RAM
  for( int i=0; i<3; i++) {
    Serial.print("buffer ");
    Serial.println( i);
    ramr[1] = i;
    Serial2.write( ramr, 7);
    nr = Serial2.readBytes( ramd, 10);
    Serial.print( nr);
    Serial.println(" bytes read");
    if( nr) {
      for( int n=0; n<nr; n++) {
	Serial.print( ramd[n]);
	Serial.print(" ");
      }
      Serial.println();
    }
  }
  delay(1000);
}
