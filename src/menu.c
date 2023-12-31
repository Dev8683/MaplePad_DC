/*
 * MaplePad Menu
 *
 * Attempt at a flexible & extensible hierarchical menu system
 *
 * TO-DO:
 * ☐ VMU Palette submenu
 * ☐ UI Palette submenu
 * ☐ Deadzone adjust submenus for stick + trigger
 * ☐ Button test submenu (lineart + ellipse routine for now)
 * ☐ OLED Detect (ssd1331present, needs to make certain menu items invisible if ssd1306 is selected)
 *
 */

#include "maple.h"
#include "menu.h"
#include "display.h"

uint32_t flipLockout;
volatile bool redraw = 1;

struct repeating_timer redrawTimer;

static uint16_t color = 0x0000;

int paletteVMU(menu *self) {
  // draw VMU palette selection
  return (1);
}

int paletteUI(menu *self) {
  // draw UI palette selection
  return (1);
}

int buttontest(menu *self) {
  // draw button test
  return (1);
}

int sCal(menu *self) {
  // draw stick calibration
  redraw = 0; // Disable redrawMenu

  while (!gpio_get(ButtonInfos[0].InputIO));

  clearDisplay();
  char *cal_string = "Center stick,";
  putString(cal_string, 0, 0, color);
  cal_string = "then press A!";
  putString(cal_string, 0, 1, color);
  updateDisplay();

  sleep_ms(50);
  while (gpio_get(ButtonInfos[0].InputIO));

  adc_select_input(0); // X
  xCenter = adc_read() >> 4;

  adc_select_input(1); // Y
  yCenter = adc_read() >> 4;

  clearDisplay();
  cal_string = "Move stick";
  putString(cal_string, 0, 0, color);
  cal_string = "  around";
  putString(cal_string, 0, 1, color);
  cal_string = "  a lot!";
  putString(cal_string, 0, 2, color);
  updateDisplay();

  xMin = 0x80;
  xMax = 0x80;
  yMin = 0x80;
  yMax = 0x80;

  uint32_t start = to_ms_since_boot(get_absolute_time());
  while ((to_ms_since_boot(get_absolute_time()) - start) < 4000 ? true : gpio_get(ButtonInfos[0].InputIO)) {
    static bool prompt = true;
    static uint8_t xData = 0;
    static uint8_t yData = 0;

    adc_select_input(0); // X
    xData = adc_read() >> 4;

    adc_select_input(1); // Y
    yData = adc_read() >> 4;

    if (xData < xMin)
      xMin = xData;
    else if (xData > xMax)
      xMax = xData;

    if (yData < yMin)
      yMin = yData;
    else if (yData > yMax)
      yMax = yData;

    // printf("\033[H\033[2J");
    // printf("\033[36m");
    // printf("xRaw: 0x%04x  yRaw: 0x%02x\n", xRaw, yRaw);
    // printf("xData: 0x%02x  yData: 0x%02x\n", xData, yData);
    // printf("xMin: 0x%02x  xMax: 0x%02x yMin: 0x%02x yMax: 0x%02x\n", xMin, xMax, yMin, yMax);
    // sleep_ms(50);

    if ((to_ms_since_boot(get_absolute_time()) - start) >= 4000 && prompt) {
      prompt = false;
      cal_string = "  Press A";
      putString(cal_string, 0, 3, color);
      cal_string = " when done!";
      putString(cal_string, 0, 4, color);
      updateDisplay();
    }
  }

  // Write config values to flash
  updateFlashData();

  redraw = 1;

  return (1);
}

int tCal(menu *self) {
  // draw trigger calibration
  redraw = 0; // Disable redrawMenu

  while (!gpio_get(ButtonInfos[0].InputIO))
    ;

  clearDisplay();
  char *cal_string = "leave";
  putString(cal_string, 0, 0, color);
  cal_string = "triggers idle";
  putString(cal_string, 0, 1, color);
  cal_string = "and press A";
  putString(cal_string, 0, 2, color);
  updateDisplay();

  sleep_ms(500);
  while (gpio_get(ButtonInfos[0].InputIO))
    ;

  adc_select_input(2); // L
  lMin = adc_read() >> 4;

  adc_select_input(3); // R
  rMin = adc_read() >> 4;

  clearDisplay();
  cal_string = "hold lMax";
  putString(cal_string, 0, 0, color);
  cal_string = "and press A";
  putString(cal_string, 0, 1, color);
  updateDisplay();

  sleep_ms(500);
  while (gpio_get(ButtonInfos[0].InputIO))
    ;

  adc_select_input(2); // lMax
  lMax = adc_read() >> 4;

  clearDisplay();
  cal_string = "hold rMax";
  putString(cal_string, 0, 0, color);
  cal_string = "and press A";
  putString(cal_string, 0, 1, color);
  updateDisplay();

  sleep_ms(500);
  while (gpio_get(ButtonInfos[0].InputIO))
    ;

  adc_select_input(3); // rMax
  rMax = adc_read() >> 4;

  if (lMin > lMax) {
    uint temp = lMin;
    lMin = lMax;
    lMax = temp;
  }

  if (rMin > rMax) {
    uint temp = rMin;
    rMin = rMax;
    rMax = temp;
  }

  // Write config values to flash
  updateFlashData();

  redraw = 1;

  return (1);
}

int sDeadzone(menu *self) {
  // draw deadzone configuration

  redraw = 0;

  char* cal_string = "X Deadzone";
  char sdata[5] = {0};

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    putString(cal_string, 0, 0, color);
    sprintf(sdata, "0x%02x", xDeadzone);
    putString(sdata, 3, 2, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (xDeadzone < 128)
        xDeadzone++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (xDeadzone > 0)
        xDeadzone--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (xDeadzone > 7)
        xDeadzone = xDeadzone - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (xDeadzone < 121)
        xDeadzone = xDeadzone + 8;}

    sleep_ms(60);
  }

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    cal_string = "X";
    putString(cal_string, 5, 0, color);
    cal_string = "AntiDeadzone";
    putString(cal_string, 0, 1, color);
    sprintf(sdata, "0x%02x", xAntiDeadzone);
    putString(sdata, 3, 3, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (xAntiDeadzone < 128)
        xAntiDeadzone++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (xAntiDeadzone > 0)
        xAntiDeadzone--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (xAntiDeadzone > 7)
        xAntiDeadzone = xAntiDeadzone - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (xAntiDeadzone < 121)
        xAntiDeadzone = xAntiDeadzone + 8;}

    sleep_ms(60);
  }

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    cal_string = "Y Deadzone";
    putString(cal_string, 0, 0, color);
    sprintf(sdata, "0x%02x", yDeadzone);
    putString(sdata, 3, 2, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (yDeadzone < 128)
        yDeadzone++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (yDeadzone > 0)
        yDeadzone--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (yDeadzone > 7)
        yDeadzone = yDeadzone - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (yDeadzone < 121)
        yDeadzone = yDeadzone + 8;}

    sleep_ms(60);
  }

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    cal_string = "Y";
    putString(cal_string, 5, 0, color);
    cal_string = "AntiDeadzone";
    putString(cal_string, 0, 1, color);
    sprintf(sdata, "0x%02x", yAntiDeadzone);
    putString(sdata, 3, 3, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (yAntiDeadzone < 128)
        yAntiDeadzone++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (yAntiDeadzone > 0)
        yAntiDeadzone--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (yAntiDeadzone > 7)
        yAntiDeadzone = yAntiDeadzone - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (yAntiDeadzone < 121)
        yAntiDeadzone = yAntiDeadzone + 8;}

    sleep_ms(60);
  }

  updateFlashData();

  clearDisplay();
  
  redraw = 1;

  return (1);
}

int tDeadzone(menu *self) {
  // draw deadzone configuration

  redraw = 0;

  char* cal_string = "L Deadzone";
  char tdata[5];

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    putString(cal_string, 0, 0, color);
    sprintf(tdata, "0x%02x", lDeadzone);
    putString(tdata, 3, 2, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (lDeadzone < 128)
        lDeadzone++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (lDeadzone > 0)
        lDeadzone--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (lDeadzone > 7)
        lDeadzone = lDeadzone - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (lDeadzone < 121)
        lDeadzone = lDeadzone + 8;}

    sleep_ms(60);
  }

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    cal_string = "L";
    putString(cal_string, 5, 0, color);
    cal_string = "AntiDeadzone";
    putString(cal_string, 0, 1, color);
    sprintf(tdata, "0x%02x", lAntiDeadzone);
    putString(tdata, 3, 3, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (lAntiDeadzone < 128)
        lAntiDeadzone++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (lAntiDeadzone > 0)
        lAntiDeadzone--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (lAntiDeadzone > 7)
        lAntiDeadzone = lAntiDeadzone - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (lAntiDeadzone < 121)
        lAntiDeadzone = lAntiDeadzone + 8;}

    sleep_ms(60);
  }

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    cal_string = "R Deadzone";
    putString(cal_string, 0, 0, color);
    sprintf(tdata, "0x%02x", rDeadzone);
    putString(tdata, 3, 2, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (rDeadzone < 128)
        rDeadzone++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (rDeadzone > 0)
        rDeadzone--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (rDeadzone > 7)
        rDeadzone = rDeadzone - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (rDeadzone < 121)
        rDeadzone = rDeadzone + 8;}

    sleep_ms(60);
  }

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    cal_string = "R";
    putString(cal_string, 5, 0, color);
    cal_string = "AntiDeadzone";
    putString(cal_string, 0, 1, color);
    sprintf(tdata, "0x%02x", rAntiDeadzone);
    putString(tdata, 3, 3, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (rAntiDeadzone < 128)
        rAntiDeadzone++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (rAntiDeadzone > 0)
        rAntiDeadzone--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (rAntiDeadzone > 7)
        rAntiDeadzone = rAntiDeadzone - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (rAntiDeadzone < 121)
        rAntiDeadzone = rAntiDeadzone + 8;}

    sleep_ms(60);
  }

  updateFlashData();

  clearDisplay();
  redraw = 1;

  return (1);
}

int timerAdjust(menu *self) {
  // draw deadzone configuration

  redraw = 0;

  char* cal_string = "Autoreset";
  char tdata[5];

  while(!gpio_get(ButtonInfos[0].InputIO));

  while (gpio_get(ButtonInfos[0].InputIO)) {
    clearDisplay();
    putString(cal_string, 0, 0, color);
    sprintf(tdata, "%03d seconds", autoResetTimer * 2);
    putString(tdata, 0, 2, color);
    updateDisplay();

    if (!gpio_get(ButtonInfos[4].InputIO)){
      if (autoResetTimer < 255)
        autoResetTimer++;}
    else if (!gpio_get(ButtonInfos[5].InputIO)){
      if (autoResetTimer > 0)
        autoResetTimer--;}
    else if (!gpio_get(ButtonInfos[6].InputIO)){
      if (autoResetTimer > 7)
        autoResetTimer = autoResetTimer - 8;}
    else if (!gpio_get(ButtonInfos[7].InputIO)){
      if (autoResetTimer < 248)
        autoResetTimer = autoResetTimer + 8;}

    sleep_ms(60);
  }

  while(!gpio_get(ButtonInfos[0].InputIO));

  updateFlashData();

  clearDisplay();
  redraw = 1;

  return (1);
}

int toggleOption(menu *self) {

  if (!strcmp(self->name, "OLED Flip     ")) {
    if ((to_ms_since_boot(get_absolute_time()) - flipLockout) > 500) {

      if (self->type == 1)
        self->on = !(self->on);

      flipLockout = to_ms_since_boot(get_absolute_time());

      cancel_repeating_timer(&redrawTimer);
      updateFlags();
      updateFlashData();

      if(oledType)
        ssd1331_init();
      else ssd1306_init();
      sleep_ms(100);

      add_repeating_timer_ms(-10, rainbowCycle, NULL, &redrawTimer);
      return (1);
    } else
      return (1);
  } else {
    if (self->type == 1)
      self->on = !(self->on);
  }

  return (1);
}

int exitToPad(menu *self) {
  // gather up flags and update them
  updateFlags();
  return (0);
}

int dummy(menu *self) { return (1); }

static menu mainMenu[6] = {
  {"Button Test   ", 2, 1, 1, 1, 1, buttontest},
  {"Stick Config  ", 0, 1, 0, 1, 1, dummy},
  {"Trigger Config", 0, 1, 0, 1, 1, dummy},
  {"Edit VMU Color", 2, 1, 0, 1, 1, paletteVMU}, // ssd1331 present
  {"Settings      ", 0, 1, 0, 1, 1, dummy},
  {"Exit          ", 2, 0, 0, 1, 1, exitToPad}
};

menu *currentMenu = mainMenu;
uint8_t currentNumEntries = sizeof(mainMenu) / sizeof(menu);
uint8_t currentEntry = 0;
uint8_t selectedEntry = 0;
uint8_t firstVisibleEntry = 0;
uint8_t lastVisibleEntry = 4;
uint8_t prevEntryModifier = 0;
uint8_t entryModifier = 0;

int mainmen(menu *self) {
  currentMenu = mainMenu;
  currentNumEntries = sizeof(mainMenu) / sizeof(menu);
  entryModifier = prevEntryModifier;
  return (1);
}

static menu stickConfig[6] = {
  {"Back          ", 2, 1, 0, 1, 1, mainmen}, // i.e. setCurrentMenu to mainMenu
  {"Calibration   ", 2, 1, 1, 1, 1, sCal},
  {"Deadzone Edit ", 2, 1, 0, 1, 1, sDeadzone},
  {"Invert X      ", 1, 1, 0, 0, 1, toggleOption},
  {"Invert Y      ", 1, 1, 0, 0, 1, toggleOption},
  {"Swap X&Y      ", 1, 0, 0, 0, 1, toggleOption}
};

int sConfig(menu *self) {
  currentMenu = stickConfig;
  currentNumEntries = sizeof(stickConfig) / sizeof(menu);
  prevEntryModifier = entryModifier;
  entryModifier = 0;
  return (1);
}

static menu triggerConfig[6] = {
  {"Back          ", 2, 1, 0, 1, 1, mainmen}, 
  {"Calibration   ", 2, 1, 1, 1, 1, tCal}, 
  {"Deadzone Edit ", 2, 1, 0, 1, 1, tDeadzone}, 
  {"Invert L      ", 1, 1, 0, 0, 1, toggleOption}, 
  {"Invert R      ", 1, 1, 0, 0, 1, toggleOption}, 
  {"Swap L&R      ", 1, 0, 0, 0, 1, toggleOption}
};

int tConfig(menu *self) {
  currentMenu = triggerConfig;
  currentNumEntries = sizeof(triggerConfig) / sizeof(menu);
  prevEntryModifier = entryModifier;
  entryModifier = 0;
  return (1);
}

static menu settings[11] = {
  {"Back          ", 2, 1, 1, 1, 1, mainmen}, 
  {"Boot Video    ", 3, 1, 0, 1, 1, dummy},        
  {"Rumble        ", 1, 1, 0, 1, 1, toggleOption}, 
  {"VMU           ", 1, 1, 0, 1, 1, toggleOption}, 
  {"UI Color      ", 2, 1, 0, 1, 1, paletteUI}, // ssd1331 present
  {"OLED:         ", 3, 0, 0, 1, 1, dummy},
  {"OLED Flip     ", 1, 0, 0, 0, 1, toggleOption},
  {"Autoreset     ", 1, 0, 0, 0, 1, toggleOption},
  {"Adjust Timeout", 2, 0, 0, 1, 1, timerAdjust},

  #if HKT7700 
  {"Dev:  HKT-7700", 3, 0, 0, 1, 1, dummy},
  #elif HKT7300 
  {"Dev:  HKT-7300", 3, 0, 0, 1, 1, dummy},
  #endif
  
  {"FW:        1.5", 3, 0, 0, 1, 1, dummy}
};

int setting(menu *self) {
  currentMenu = settings;
  currentNumEntries = sizeof(settings) / sizeof(menu);
  prevEntryModifier = entryModifier;
  entryModifier = 0;
  return (1);
}

void loadFlags() {
  stickConfig[3].on = invertX;
  stickConfig[4].on = invertY;
  stickConfig[5].on = swapXY;
  triggerConfig[3].on = invertL;
  triggerConfig[4].on = invertR;
  triggerConfig[5].on = swapLR;
  settings[2].on = rumbleEnable;
  settings[3].on = vmuEnable;
  settings[6].on = oledFlip;
  settings[7].on = autoResetEnable;
}

void updateFlags() {
  invertX = stickConfig[3].on;
  invertY = stickConfig[4].on;
  swapXY = stickConfig[5].on;
  invertL = triggerConfig[3].on;
  invertR = triggerConfig[4].on;
  swapLR = triggerConfig[5].on;
  rumbleEnable = settings[2].on;
  vmuEnable = settings[3].on;
  oledFlip = settings[6].on;
  autoResetEnable = settings[7].on;
}

void getSelectedEntry() {
  for (int i = 0; i < currentNumEntries; i++) {
    if (currentMenu[i].selected) {
      selectedEntry = i;
      break;
    }
  }
}

void getFirstVisibleEntry() {
  for (int i = 0; i < currentNumEntries; i++) {
    if (currentMenu[i].visible) {
      firstVisibleEntry = i;
      break;
    }
  }
}

void getLastVisibleEntry() {
  for (int i = currentNumEntries - 1; i >= 0; i--) {
    if (currentMenu[i].visible) {
      lastVisibleEntry = i;
      break;
    }
  }
}

void redrawMenu() {
  clearDisplay();

  int i = 0;

  for (uint8_t n = 0; n < currentNumEntries; n++) {
    if (currentMenu[n].visible) {
      putString(currentMenu[n].name, 0, n + entryModifier, color);
      if (currentMenu[n].type == 1) // boolean type menu
        drawToggle(n + entryModifier, color, currentMenu[n].on);
    }
    drawCursor(selectedEntry + entryModifier, color);
  }
  updateDisplay();
}

static uint16_t hue = 0;

bool rainbowCycle(struct repeating_timer *t) {
  static uint8_t r = 0x00;
  static uint8_t g = 0x00;
  static uint8_t b = 0x00;

  fast_hsv2rgb_32bit(hue, 255, 255, &r, &g, &b);

  color = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);

  if (hue == 1535)
    hue = 0;
  else
    hue++;

  if (redraw)
    redrawMenu();

  return (true);
}

void runMenu() {

  // Check for menu button combo (Start + Down + B)

  /* Menu
  -Stick to 96x64 size even on monochrome OLED... Simpler!
  -Need method to handle number of current menu entries. Iterate through different arrays of menu structs to know which entries to draw...?
  -menu_item struct can have a "visible" flag that menuRedraw or menuScroll will toggle
  -Function call to redraw menu list (menuRedraw()?)
  -Draw cursor next to current menu item, move up and down when dpad (or optionally, analog stick) is moved
  -Cursor should default to second item on menus with 'Back' option
  -Scroll menu when cursor reaches bottom of screen

  -SSD1306 and SSD1331 drawing functions can accelerate menu development:
          -Drawing squares with top left and bottom right coordinates and specific color ✓
          -Drawing hollow rectangles with inner and outer top left and bottom right coordinates and specific color ✓
          -Drawing circles (AA?) at certain coordinates ✓
          -Drawing cursor at generic menu item locations ✓
          -Drawing menu items (text) at prescribed locations using specific font ✓

  Menu Layout:
  -Button Test
          -Enters button test screen (Press and hold B to exit)
  -Sticks Config
          -Back
          -Calibration ✓
          -Deadzone Adjust
          -Invert X  ✓
          -Invert Y  ✓
          -Swap X and Y ✓
  -Triggers Config
          -Back
          -Analog/Digital ✓
          -Calibration, greyed out if digital selected ✓
          -Invert L, greyed out if digital selected ✓
          -Invert R, greyed out if digital selected ✓
          -Swap L and R ✓
  -Edit VMU Colors (greyed out if SSD1306 detected)
          -Enters VMU Palette screen. Cycle left and right through all 8 VMU pages, and select from 8 preset colors or enter a custom RGB565 value (Press and hold B to exit)
  -Settings
          -Back
          -Splashscreen (on/off), gets greyed out when Boot Video is turned on
          -Boot Video (on/off), gets greyed out when Splashscreen is turned on or if SSD1306 detected
          -Rumble (on/off)
          -UI Color (same interface as VMU Colors, greyed out if SSD1306 detected)
          -Control UI with analog stick (on/off)
          -OLED Model (shows 'SSD1306' or 'SSD1331' next to text)
          -Firmware Version (shows firmware version next to text)
  -Exit

  */

  mainMenu[1].run = sConfig;
  mainMenu[2].run = tConfig;
  mainMenu[4].run = setting;

  loadFlags();

  if (oledType) { // SSD1331
    strcpy(settings[5].name, "OLED: SSD1331");
  } else { // SSD1306
    strcpy(settings[5].name, "OLED: SSD1306");

    // disable color-only menu entries
    //mainMenu[3].enabled = false;
    //settings[4].enabled = false;

  }

  // negative interval means the callback func is called every 10ms regardless of how long callback takes to execute
  add_repeating_timer_ms(-10, rainbowCycle, NULL, &redrawTimer);

  while (1) {
    getSelectedEntry(); // where to draw cursor
    // redrawMenu(); // called by redrawTimer

    // Wait for A button release (submenu rate-limit)
    while (!gpio_get(ButtonInfos[0].InputIO));

    uint8_t pressed = 0;
    do {
      for (int i = 0; i < 9; i++) {
        pressed |= (!gpio_get(ButtonInfos[i].InputIO));
      }
    } while (!pressed);

    sleep_ms(75); // Wait out switch bounce + rate-limiting

    if (!gpio_get(ButtonInfos[4].InputIO)) { // Up
      /* check currently selected entry
      if element is not the top one, deselect current entry
      and select the first enabled entry above it */
      if (selectedEntry) { // i.e. not 0
        currentMenu[selectedEntry].selected = false;
        currentMenu[selectedEntry - 1].selected = true;

        getFirstVisibleEntry();
        if ((selectedEntry == firstVisibleEntry) && (firstVisibleEntry)) {
          currentMenu[firstVisibleEntry + 4].visible = false;
          currentMenu[firstVisibleEntry - 1].visible = true;
          entryModifier++;
        }
      }

    }

    else if (!gpio_get(ButtonInfos[5].InputIO)) { // Down
      /* check currently selected entry
      if entry is not the bottom one, deselect current entry
      and select first enabled entry below it */
      if (selectedEntry < currentNumEntries - 1) {
        currentMenu[selectedEntry].selected = false;
        currentMenu[selectedEntry + 1].selected = true;

        getLastVisibleEntry();
        if ((selectedEntry == lastVisibleEntry) && (lastVisibleEntry < currentNumEntries)) {
          currentMenu[lastVisibleEntry - 4].visible = false;
          currentMenu[lastVisibleEntry + 1].visible = true;
          entryModifier--;
        }
      }
    }

    else if (!gpio_get(ButtonInfos[0].InputIO)) { // A
      /* check currently selected entry
      if entry is enabled, run entry's function
      entry functions should set currentMenu if they enter a submenu. */
      if (currentMenu[selectedEntry].enabled)
        if (!currentMenu[selectedEntry].run(&currentMenu[selectedEntry]))
          break;
    }
    /* Entrys' functions should all return 1 except for mainMenu.exitToPad,
    which should return 0 and result in a break from this while loop. */
  }
  cancel_repeating_timer(&redrawTimer);
}
