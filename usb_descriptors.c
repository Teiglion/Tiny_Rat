#include "tusb.h"

// Индексы строк (совпадают с порядком в string-дескрипторах)
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

// Номер интерфейса HID
enum {
  ITF_NUM_HID = 0,
  ITF_NUM_TOTAL
};

// Endpoint адрес для клавиатуры: 0x81 = IN endpoint 1
#define EPNUM_HID   0x81

// Размер конфигурационного дескриптора (пока можно оставить 0, TinyUSB посчитает макросами)
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

// Стандартный дескриптор устройства
tusb_desc_device_t const desc_device = {
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = 0x0200,        // USB 2.0
  .bDeviceClass       = 0x00,          // каждый интерфейс сам объявляет класс
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x00,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor           = 0xCafe,        // временный VID (для экспериментов)
  .idProduct          = 0x4000,        // PID под себя
  .bcdDevice          = 0x0100,        // версия устройства 1.00

  .iManufacturer      = STRID_MANUFACTURER,
  .iProduct           = STRID_PRODUCT,
  .iSerialNumber      = STRID_SERIAL,

  .bNumConfigurations = 1
};

// Callback — TinyUSB запрашивает device descriptor
uint8_t const* tud_descriptor_device_cb(void) {
  return (uint8_t const*) &desc_device;
}

// Описание формата HID-репортов (клавиатура)
uint8_t const desc_hid_report_keyboard[] = {
  TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(1) )
};

// Callback — TinyUSB отдаёт HID report descriptor
uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf) {
  (void) itf;
  return desc_hid_report_keyboard;
}

// Конфигурационный дескриптор:
//  - 1 конфигурация
//  - 1 HID интерфейс
//  - 1 interrupt IN endpoint
uint8_t const desc_configuration[] = {
  // Конфигурация
  TUD_CONFIG_DESCRIPTOR(
      1,                  // bConfigurationValue
      ITF_NUM_TOTAL,      // количество интерфейсов
      STRID_PRODUCT,      // строка конфигурации (можно 0)
      CONFIG_TOTAL_LEN,   // общая длина
      0x00,               // атрибуты (bus powered)
      100                 // max power в *2 mA, 100 = 200 mA
  ),

  // HID интерфейс (клавиатура)
  TUD_HID_DESCRIPTOR(
      ITF_NUM_HID,        // номер интерфейса
      0,                  // строка интерфейса
      HID_ITF_PROTOCOL_KEYBOARD, // boot protocol keyboard
      sizeof(desc_hid_report_keyboard), // длина HID report descriptor
      EPNUM_HID,          // endpoint address
      CFG_TUD_HID_EP_BUFSIZE,   // размер endpoint буфера
      10                  // polling interval в мс
  ),
};

// Callback — TinyUSB запрашивает configuration descriptor
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
  (void) index;
  return desc_configuration;
}

// Языковой дескриптор (обязательный нулевой)
static uint16_t const desc_string_langid[] = {
  (TUSB_DESC_STRING << 8) | (2 + 2),
  0x0409  // English (United States)
};

// Строки: manufacturer, product, serial
static char const* string_desc[] = {
  (const char*)"",            // 0: не используется напрямую
  "Dmitry Labs",              // 1: Manufacturer
  "Automaton HID Keyboard",   // 2: Product
  "0001",                     // 3: Serial (можно сгенерить)
};

static uint16_t _desc_str[32];

// Callback — TinyUSB запрашивает string descriptor по index
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void) langid;

  if (index == 0) {
    return desc_string_langid;
  }

  index--;
  if (index >= sizeof(string_desc) / sizeof(string_desc[0])) return NULL;

  const char* str = string_desc[index];

  // Конвертация ASCII -> UTF16-LE
  uint8_t len = 0;
  while (str[len] && len < 31) {
    _desc_str[1 + len] = str[len];
    len++;
  }

  _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 + 2 * len);

  return _desc_str;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer, uint16_t bufsize)
{
    // Пока ничего не делаем.
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t* buffer, uint16_t reqlen)
{
    // Ничего не возвращаем, просто говорим "нет данных".
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}