#ifndef HW_LF1000_H
#define HW_LF1000_H

#include <math.h>

#define DIDJ_BOARD_ID 3

class LF1000 : public Platform {
private:
	volatile uint16_t *memregs;
	int memdev = 0;
	int32_t tickBattery = 0;

	typedef struct {
		uint16_t batt;
		uint16_t remocon;
	} MMSP2ADC;

public:
	LF1000(GMenu2X *gmenu2x) : Platform(gmenu2x) {
				INFO("LF1000");
	};

	void hwInit() {
		setenv("SDL_NOMOUSE", "1", 1);
		w = 320;
		h = 240;

        batteryStatus = getBatteryStatus(getBatteryLevel(), 0, 0);


		cpu_menu = 393;
		cpu_link = 393;
		cpu_max = 533;
		cpu_min = 300;
		cpu_step = 20;
		
        // Move CPU back down in case we came back from a linked app w/ overclock.
        setCPU(cpu_menu);

        if(getBoardId() == DIDJ_BOARD_ID) {
			setenv("SDL_JOYSTICK_DEVICE", "/dev/input/event0", 0);
		}


		INFO("LF1000 Init Done!");
	}

	void hwDeinit() {
		INFO("LF1000 DeInit done!");
		if(getBoardId() == DIDJ_BOARD_ID) {
			unsetenv("SDL_JOYSTICK_DEVICE");
		}
	}

	uint32_t hwCheck(unsigned int interval=0, void *param = NULL) {
               tickBattery++;
                if (tickBattery > 30) { // update battery level every 30 hwChecks
                        tickBattery = 0;
                        batteryStatus = getBatteryStatus(getBatteryLevel(), 0, 0);
                }

	}

	void ledOn() {
	}

	void ledOff() {
	}

	void setBacklight(int val) {
		if (FILE *f = fopen("/sys/class/graphics/fb0/blank", "w")) {
			fprintf(f, "%d", val <= 0);
			fclose(f);
		}

		if (FILE *f = fopen("/sys/class/backlight/lf1000-pwm-bl/brightness", "w")) {
			fprintf(f, "%d", 400 - (val * 4));
			INFO("Set Backlight %d %d", val, 400 - (val * 4));
			fclose(f);
		}

		return;
	}

	int16_t getBacklight() {
		int val = -1;
		if (FILE *f = fopen("/sys/class/backlight/lf1000-pwm-bl/brightness", "r")) {
			fscanf(f, "%i", &val);
			fclose(f);
		}
		INFO("Get backlight %d %d", val, (400-val)/4);
		return (400-val)/4 ;
	}

	int16_t getBatteryLevel() {
		int val = -1;
		char power_source[8];
		if (FILE *f = fopen("/sys/devices/platform/lf1000-power/voltage", "r")) {
			fscanf(f, "%i", &val);
			fclose(f);
		}
		if (FILE *f = fopen("/sys/devices/platform/lf1000-power/power_source", "r")) {
			fscanf(f, "%8c", &power_source);
			fclose(f);
			INFO("Power Source %s\n", &power_source);
		}

		INFO("Battery level %d\n", val);
		// AC Adapter
		if (std::string(power_source) == "EXTERNAL") { 
			INFO("AC Adapter Connected.\n");
			return -1;
			}
		return val;
	}


	uint8_t getBatteryStatus(int32_t val, int32_t min, int32_t max) {
		if ((val > 10000) || (val < 0)) return 6;
		/* Needs tuning. LF's voltage refs in /sys say:
         "full battery" 8000mv
		 "normal battery" 4600mv
         "low battery" 4200mv  
		 "critical battery" 2000mv
		*/
	    INFO("Battery status %d %d %d\n", val, min, max);
		if (val == -1)  return 6;  // AC Adapter
		if (val > 5500) return 5; // 100%
		if (val > 5000) return 4; // 80%
		if (val > 4600) return 3; // 55%
		if (val > 4200) return 2; // 30%
		if (val > 3000) return 1; // 15%
		return 0; // 0% :(
	}

	void setCPU(uint32_t mhz) {
		// FIXME: It should be possible to do more granular clocking, but the kernel-side CPU code appears to only
		// want certain magic numbers in hz, or the system will lock up. My best guess is that the new frequency
		// needs to be an even multiple of one of the PLL clocks, but for now, high/low clocking will suffice.
        uint32_t hz = 393218109;
		if(mhz > 500) hz = 532500000;

		if (FILE *f = fopen("/sys/devices/platform/lf1000-clock/cpu_freq_in_hz", "w")) {
			fprintf(f, "%d", hz);
			INFO("Set CPU to %d hz (requested %d mhz)", hz, mhz);
			fclose(f);
		}

	}

	uint8_t getVolumeMode(uint8_t vol) {
		if (!vol) return VOLUME_MODE_MUTE;
		return VOLUME_MODE_NORMAL;
	}

	void setVolume(int val) {
		val = val * (255.0f / 100.0f);
		volumeMode = getVolumeMode(val);

		int hp = 0;
		char cmd[96];
                // For now, use near-100% on Speaker/Headphone specifics, etc
		// TODO: Set mute off/on when calling exec, so that audio isn't running during the menu
		sprintf(cmd, "amixer set Master %d; amixer set Master on; amixer set Headphone 250; amixer set Speaker 250", val);
		system(cmd);
	}


	int getBoardId() {
		int board_id = -1;
		if (FILE *f = fopen("/sys/devices/platform/lf1000-gpio/board_id", "r")) {
			fscanf(f, "%d", &board_id);
			fclose(f);
			INFO("LF1000 Board ID = 0x%x\n", board_id);
		}
        if (board_id < 0) {
		    ERROR("Can't read LF1000 board ID!");
		}
        return board_id;
	}

};

#endif
