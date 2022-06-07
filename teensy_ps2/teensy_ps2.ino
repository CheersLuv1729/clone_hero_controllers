#include <DigitalIO.h>
#include <XInput.h>
#include <array>

constexpr int DAT = 12;//12; // MISO // black
constexpr int CMD = 11;//13; // MOSI // brown
constexpr int ATT = 10;//15; // ATT  // yellow
constexpr int CLK = 13;//14; // SCLK // green

DigitalPin<ATT> att;
DigitalPin<MOSI> cmd;
DigitalPin<MISO> dat;
DigitalPin<SCK> clk;

#include <SPI.h>

static SPISettings spiSettings (150 * 1000, LSBFIRST, SPI_MODE3);

template <size_t S>
void send_data(std::array<byte, S>& data){

  SPI.beginTransaction(spiSettings);
  att.low();
  
  for (byte& b : data){
    b = SPI.transfer(b);
    delayMicroseconds(25);
  }

  att.high();
  SPI.endTransaction();
}


void ps2_enter_config(){
 
  std::array<byte, 5> data = {0x01, 0x43, 0x00, 0x01, 0x00};
  send_data(data);

}

void ps2_exit_config(){
 
  std::array<byte, 9> data = {0x01, 0x43, 0x00, 0x00, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A};
  send_data(data);
  
}

void ps2_enable_analog_mode(){
 
  std::array<byte, 9> data = {0x01, 0x44, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00};
  send_data(data);
  
}

void ps2_send_all_pressures(){

  std::array<byte, 9> data = {0x01, 0x4F, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00};
  send_data(data);
  
}

void ps2_poll(byte* out, int n){
  
  std::array<byte, 9> data = {0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  send_data(data);

  for(int i = 0; i < 21 && i < n; i++){
    out[i] = data[i];
  }
  
}

void setup() {

  
  att.config (OUTPUT, HIGH);    // HIGH -> Controller not selected
  cmd.config (OUTPUT, HIGH);
  clk.config (OUTPUT, HIGH);
  dat.config (INPUT, HIGH);     // Enable pull-up

  //delay(50);

  SPI.begin();
  XInput.begin();
  XInput.setAutoSend(false);
  Serial.begin(115200);
  
  
  ps2_poll(nullptr, 0);
  ps2_enter_config();
  ps2_enable_analog_mode();
  ps2_send_all_pressures();
  ps2_exit_config();
  
}

void enterBootSel(){

  _reboot_Teensyduino_();

}

void loop() {

  static uint32_t time = micros();

  byte data[9];
  ps2_poll(data, 9);
  do{
    ps2_poll(data, 9);
  
  }while(data[1] == 0xFF);

  bool G_fret = !(data[4] & (1 << 1));
  bool R_fret = !(data[4] & (1 << 5));
  bool Y_fret = !(data[4] & (1 << 4));
  bool B_fret = !(data[4] & (1 << 6));
  bool O_fret = !(data[4] & (1 << 7));

  bool Start_button = !(data[3] & (1 << 3));
  bool Select_button = !(data[3] & (1 << 0));

  bool U_dpad = !(data[3] & (1 << 4));
  bool D_dpad = !(data[3] & (1 << 6));

  bool Star_tilt = !(data[4] & (1 << 0));

  int whammy = (127 - (data[8] & 127));

  
  XInput.setButton(BUTTON_A,  G_fret);
  XInput.setButton(BUTTON_B,  R_fret);
  XInput.setButton(BUTTON_Y,  Y_fret);
  XInput.setButton(BUTTON_X,  B_fret);
  XInput.setButton(BUTTON_LB, O_fret);
  
  XInput.setButton(BUTTON_START, Start_button);
  XInput.setButton(BUTTON_BACK,  Select_button);

  XInput.setDpad(U_dpad, D_dpad, false, false);
  
  XInput.setJoystick(JOY_LEFT, 0, 0);
  XInput.setJoystick(JOY_RIGHT, Star_tilt ? 32767 : 0, whammy * 258);
  
  XInput.send();

  Serial.print(G_fret ? "G" : "-");
  Serial.print(R_fret ? "R" : "-");
  Serial.print(Y_fret ? "Y" : "-");
  Serial.print(B_fret ? "B" : "-");
  Serial.print(O_fret ? "O" : "-");
  Serial.print(" ");
  Serial.print(Start_button  ? "S" : "-");
  Serial.print(Select_button ? "s" : "-");
  Serial.print(" ");
  Serial.print(U_dpad ? "U" : "-");
  Serial.print(D_dpad ? "D" : "-");
  Serial.print(" ");
  Serial.print(Star_tilt ? "*" : "-");
  Serial.print(" ");

  for(int i = 0; i < 16; i++) {Serial.print((whammy > 8 * i) ? "W" : "-");}
  
  Serial.print("   ");
  for (byte b : data){Serial.print(b, HEX); Serial.print(" "); }
  Serial.print("\n");

  static uint32_t configTime = 0;

  bool config = Select_button && Start_button && G_fret && R_fret && Y_fret;
  
  if(!config){
    configTime = time;
  }else{
    if((time - configTime) > 2*1000*1000){
      enterBootSel();  
    }
  }

  uint32_t now = micros();
  uint32_t delta = now - time;
  time = now;

  uint32_t target = (1000000 / 144);

  if(target >= delta)
    delayMicroseconds(target - delta);
  
}
