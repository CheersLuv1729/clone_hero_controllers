#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>

#include "pico/stdlib.h"
#include "tusb.h"
#include "pico/time.h"
#include "device/usbd_pvt.h"
#include "hardware/adc.h"


uint8_t endpoint_in=0;
uint8_t endpoint_out=0;

struct usb_report
{
    void* data;
    size_t size;
};

struct GamepadInputState {
	bool BUTTON_A;
	bool BUTTON_B;
	bool BUTTON_X;
	bool BUTTON_Y;

	bool BUTTON_START;
	bool BUTTON_BACK;

	bool DPAD_UP;
	bool DPAD_DOWN;
	bool DPAD_LEFT;
	bool DPAD_RIGHT;

	bool BUTTON_LB;
	bool BUTTON_RB;

	bool BUTTON_LS;
	bool BUTTON_RS;

  bool BUTTON_XBOX;

	int32_t STICK_LEFT_X;
	int32_t STICK_LEFT_Y;
	int32_t STICK_RIGHT_X;
	int32_t STICK_RIGHT_Y;
	uint8_t TRIGGER_LEFT;
	uint8_t TRIGGER_RIGHT;
};

struct __attribute((packed, aligned(1))) xinput_report{
	uint8_t rid;
	uint8_t rsize;
	uint8_t digital_buttons_1;
	uint8_t digital_buttons_2;
	uint8_t lt;
	uint8_t rt;
	int16_t l_x;
	int16_t l_y;
	int16_t r_x;
	int16_t r_y;
	uint8_t reserved_1[6];
};

xinput_report gamepad_state_to_report(const GamepadInputState& state){
  xinput_report report {
      .rid = 0,
      .rsize = 20,
      .digital_buttons_1 = 
        (state.DPAD_UP      << 0) |
        (state.DPAD_DOWN    << 1) | 
        (state.DPAD_LEFT    << 2) | 
        (state.DPAD_RIGHT   << 3) | 
        (state.BUTTON_START << 4) | 
        (state.BUTTON_BACK  << 5) | 
        (state.BUTTON_LS    << 6) | 
        (state.BUTTON_RS    << 7), 
      .digital_buttons_2 = 
        state.BUTTON_LB    << 0 |
        state.BUTTON_RB    << 1 | 
        state.BUTTON_XBOX  << 2 | 
 
        state.BUTTON_A     << 4 | 
        state.BUTTON_B     << 5 | 
        state.BUTTON_X     << 6 | 
        state.BUTTON_Y     << 7, 
      .lt = state.TRIGGER_LEFT,
      .rt = state.TRIGGER_RIGHT,
      .l_x = state.STICK_LEFT_X,
      .l_y = state.STICK_LEFT_Y,
      .r_x = state.STICK_RIGHT_X,
      .r_y = state.STICK_RIGHT_Y,
      .reserved_1 = { },
  };
  return report;
}

template<typename T>
static void sendReportData(T report) {
  if (tud_suspended())
    tud_remote_wakeup();
  if ((tud_ready()) && ((endpoint_in != 0)) && (!usbd_edpt_busy(0, endpoint_in))){
    usbd_edpt_claim(0, endpoint_in);
    usbd_edpt_xfer(0, endpoint_in, (uint8_t*)(&report), sizeof(T));
    usbd_edpt_release(0, endpoint_in);
  }   
}

class GHLiveGamepad{


  // Pin refers to the GPIO pin number
  // Not the numbered pin on the board
  // ie physical pin 1 is GP0

  void initPinInput(int pin){
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
  }

  void initPinOutput(int pin){
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
  }

  void initPinADC(int pin){
    adc_gpio_init(pin);
  }

  uint16_t readInputADC(int input){
    adc_select_input(input);
    uint16_t ret = adc_read() << 4;
    return ret;
  }

  bool readButton(int pin){
    return gpio_get(pin) ? 0 : 1;
  }

  static constexpr int GP_STRUM_U = 0;
  static constexpr int GP_STRUM_D = 1;
  
  static constexpr int GP_LED_1 = 5;
  static constexpr int GP_LED_2 = 4;
  static constexpr int GP_LED_3 = 3;
  static constexpr int GP_LED_4 = 2;
  
  static constexpr int GP_LED_VCC = 6;
  
  static constexpr int GP_START  = 7;
  static constexpr int GP_SELECT = 8;
  
  static constexpr int GP_STICK_R = 10;
  static constexpr int GP_STICK_L = 11;
  static constexpr int GP_STICK_D = 12;
  static constexpr int GP_STICK_U = 13;
  static constexpr int GP_GH_KEY  = 14;
  static constexpr int GP_MENU    = 15;
  
  static constexpr int GP_FRET_B_1 = 18;
  static constexpr int GP_FRET_B_2 = 17;
  static constexpr int GP_FRET_B_3 = 16;
  static constexpr int GP_FRET_W_1 = 21;
  static constexpr int GP_FRET_W_2 = 20;
  static constexpr int GP_FRET_W_3 = 19;
  
  static constexpr int GP_FRET_MODE = 22;
  
  static constexpr int GP_TILT   = 27;
  static constexpr int GP_WHAMMY = 28;
  
  static constexpr int ADC_TILT   = 1;
  static constexpr int ADC_WHAMMY = 2;

public:


  GHLiveGamepad(){

    initPinInput(GP_STRUM_U);  // Up   (Strum)
    initPinInput(GP_STRUM_D);  // Down (Strum)

    initPinOutput(GP_LED_1); // LED Ch1
    initPinOutput(GP_LED_2); // LED Ch2
    initPinOutput(GP_LED_3); // LED Ch3
    initPinOutput(GP_LED_4); // LED Ch4

    // VCC Used with LEDs. We set this high and pull Ch low to power them
    initPinOutput(GP_LED_VCC); 

    initPinInput(GP_START); // Start
    initPinInput(GP_SELECT); // Select

    initPinInput(GP_STICK_R); // Right (Stick)
    initPinInput(GP_STICK_L); // Left  (Stick)
    initPinInput(GP_STICK_D); // Down  (Stick)
    initPinInput(GP_STICK_U); // Up    (Stick)
    initPinInput(GP_GH_KEY); // GH_Key

    // On the board, Menu is labelled as VB
    // It's used as the sync button and connected to power in a typical controller
    // However for this project, VCC for this board is connected to GND and the button
    // works the same as the rest of them (low = pressed)
    initPinInput(GP_MENU); // Menu 

    initPinInput(GP_FRET_B_1); // B1
    initPinInput(GP_FRET_B_2); // B2
    initPinInput(GP_FRET_B_3); // B3
    initPinInput(GP_FRET_W_1); // W1
    initPinInput(GP_FRET_W_2); // W2
    initPinInput(GP_FRET_W_3); // W3

    // Mode (Connects to the neck but goes nowhere inside the neck)
    // Maybe used if different necks are connected?? but none that I know of
    // The port for the neck has 4 pins that don't even connect to it so ??
    initPinInput(GP_FRET_MODE);

    // For this project the tilt sensor is a binary high/low but it's connected to
    // ADC just in case I change my mind in future and replace the sensor
    initPinADC(GP_TILT); // Tilt   (ADC1)
    initPinADC(GP_WHAMMY); // Whammy (ADC2)

    gpio_put(GP_LED_VCC, 1);
    
    gpio_put(GP_LED_1, 1);
    gpio_put(GP_LED_2, 1);
    gpio_put(GP_LED_3, 0);
    gpio_put(GP_LED_4, 1);

  }

  GamepadInputState getState(){
    GamepadInputState s{
      .BUTTON_A = readButton(GP_FRET_B_1),
      .BUTTON_B = readButton(GP_FRET_B_2),
      .BUTTON_X = readButton(GP_FRET_W_1),
      .BUTTON_Y = readButton(GP_FRET_B_3),

      .BUTTON_START = readButton(GP_START),
      .BUTTON_BACK  = readButton(GP_SELECT),
      
      .DPAD_UP   = readButton(GP_STICK_U) || readButton(GP_STRUM_U),
      .DPAD_DOWN = readButton(GP_STICK_D) || readButton(GP_STRUM_D),
      .DPAD_LEFT  = readButton(GP_STICK_L),
      .DPAD_RIGHT = readButton(GP_STICK_R),

      .BUTTON_LB = readButton(GP_FRET_W_2),
      .BUTTON_RB = readButton(GP_FRET_W_3),

      .BUTTON_LS = readButton(GP_GH_KEY),
      .BUTTON_RS = false,

      .BUTTON_XBOX = readButton(GP_MENU),

      .STICK_LEFT_X = (1 << 7),
      .STICK_LEFT_Y = (1 << 7),
      .STICK_RIGHT_X = 0x7fff - (readInputADC(ADC_TILT) >> 1),
      .STICK_RIGHT_Y = readInputADC(ADC_WHAMMY) >> 1,

      .TRIGGER_LEFT  = (1 << 7),
      .TRIGGER_RIGHT = (1 << 7),


    };
    return s;
  };

};


int main(void)
{


  GHLiveGamepad gamepad;
  stdio_init_all();
  adc_init();
  tusb_init();

  uint64_t time = to_us_since_boot(get_absolute_time());

  while (true)
  {

    uint64_t now = to_us_since_boot(get_absolute_time());
    uint64_t target_interval = 1000;
    time += target_interval;

    int64_t delta = time - now;

    if(delta > 0)
      sleep_us(delta);

    GamepadInputState state = gamepad.getState();
    xinput_report report = gamepad_state_to_report(state);
    sendReportData(report);
    tud_task();
  }
}


static void xinput_init(void) {

}

static void xinput_reset(uint8_t __unused rhport) {
    
}

static uint16_t xinput_open(uint8_t __unused rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
  //+16 is for the unknown descriptor 
  uint16_t const drv_len = sizeof(tusb_desc_interface_t) + itf_desc->bNumEndpoints*sizeof(tusb_desc_endpoint_t) + 16;
  TU_VERIFY(max_len >= drv_len, 0);

  uint8_t const * p_desc = tu_desc_next(itf_desc);
  uint8_t found_endpoints = 0;
  while ( (found_endpoints < itf_desc->bNumEndpoints) && (drv_len <= max_len)  )
  {
    tusb_desc_endpoint_t const * desc_ep = (tusb_desc_endpoint_t const *) p_desc;
    if ( TUSB_DESC_ENDPOINT == tu_desc_type(desc_ep) )
    {
      TU_ASSERT(usbd_edpt_open(rhport, desc_ep));

      if ( tu_edpt_dir(desc_ep->bEndpointAddress) == TUSB_DIR_IN )
      {
        endpoint_in = desc_ep->bEndpointAddress;
      }else
      {
        endpoint_out = desc_ep->bEndpointAddress;
      }
      found_endpoints += 1;
    }
    p_desc = tu_desc_next(p_desc);
  }
  return drv_len;
}

static bool xinput_device_control_request(uint8_t __unused rhport, tusb_control_request_t const *request) {
  return true;
}

static bool xinput_control_complete_cb(uint8_t __unused rhport, tusb_control_request_t __unused const *request) {
    return true;
}
//callback after xfer_transfer
static bool xinput_xfer_cb(uint8_t __unused rhport, uint8_t __unused ep_addr, xfer_result_t __unused result, uint32_t __unused xferred_bytes) {
  return true;
}

static usbd_class_driver_t xinput_driver ={
    .init             = xinput_init,
    .reset            = xinput_reset,
    .open             = xinput_open,
    .control_request  = xinput_device_control_request,
    .control_complete = xinput_control_complete_cb,
    .xfer_cb          = xinput_xfer_cb,
    .sof              = NULL
};

// Implement callback to add our custom driver
usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return &xinput_driver;
}

tusb_desc_device_t const xinputDeviceDescriptor = {.bLength = sizeof(tusb_desc_device_t),
                                        .bDescriptorType = TUSB_DESC_DEVICE,
                                        .bcdUSB = 0x0200,
                                        .bDeviceClass = 0xFF,
                                        .bDeviceSubClass = 0xFF,
                                        .bDeviceProtocol = 0xFF,
                                        .bMaxPacketSize0 =
                                            CFG_TUD_ENDPOINT0_SIZE,

                                        .idVendor = 0x045E,
                                        .idProduct = 0x028E,
                                        .bcdDevice = 0x0572,

                                        .iManufacturer = 0x01,
                                        .iProduct = 0x02,
                                        .iSerialNumber = 0x03,

                                        .bNumConfigurations = 0x01};


const uint8_t xinputConfigurationDescriptor[] = {
//Configuration Descriptor:
0x09,	//bLength
0x02,	//bDescriptorType
0x30,0x00, 	//wTotalLength   (48 bytes)
0x01,	//bNumInterfaces
0x01,	//bConfigurationValue
0x00,	//iConfiguration
0x80,	//bmAttributes   (Bus-powered Device)
0xFA,	//bMaxPower      (500 mA)

//Interface Descriptor:
0x09,	//bLength
0x04,	//bDescriptorType
0x00,	//bInterfaceNumber
0x00,	//bAlternateSetting
0x02,	//bNumEndPoints
0xFF,	//bInterfaceClass      (Vendor specific)
0x5D,	//bInterfaceSubClass   
0x01,	//bInterfaceProtocol   
0x00,	//iInterface

//Unknown Descriptor:
0x10,
0x21, 
0x10, 
0x01, 
0x01, 
0x24, 
0x81, 
0x14, 
0x03, 
0x00, 
0x03,
0x13, 
0x02, 
0x00, 
0x03, 
0x00, 

//Endpoint Descriptor:
0x07,	//bLength
0x05,	//bDescriptorType
0x81,	//bEndpointAddress  (IN endpoint 1)
0x03,	//bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x20,0x00, 	//wMaxPacketSize    (1 x 32 bytes)
0x04,	//bInterval         (4 frames)

//Endpoint Descriptor:

0x07,	//bLength
0x05,	//bDescriptorType
0x02,	//bEndpointAddress  (OUT endpoint 2)
0x03,	//bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x20,0x00, 	//wMaxPacketSize    (1 x 32 bytes)
0x08,	//bInterval         (8 frames)
};


// string descriptor table
char const *string_desc_arr_xinput[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "GENERIC",                   // 1: Manufacturer
    "XINPUT CONTROLLER",         // 2: Product
    "1.0"                       // 3: Serials
};


/* A combination of interfaces must have a unique product id, since PC will save
 * device driver after the first plug. Same VID/PID with different interface e.g
 * MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&xinputDeviceDescriptor;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(void) { 
  return 0;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+



// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index; // for multiple configurations
    return xinputConfigurationDescriptor;
  return 0;
}
//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  
      (void)langid;

        uint8_t chr_count;

        if (index == 0) {
          memcpy(&_desc_str[1], string_desc_arr_xinput[0], 2);
          chr_count = 1;
        } else {
          // Convert ASCII string into UTF-16

          if (!(index < sizeof(string_desc_arr_xinput) / sizeof(string_desc_arr_xinput[0])))
            return NULL;

          const char *str = string_desc_arr_xinput[index];

          // Cap at max char
          chr_count = strlen(str);
          if (chr_count > 31)
            chr_count = 31;

          for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
          }
        }

        // first byte is length (including header), second byte is string type
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

        return _desc_str;
  
 
}