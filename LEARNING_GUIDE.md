# 📚 Fuel Pump ECU - Learning Guide

## 🎯 เป้าหมาย
เข้าใจระบบควบคุม Fuel Pump ที่ใช้ GD32F103 + FreeRTOS แบบครบถ้วน

---

## 📋 ขั้นตอนการศึกษา (Step-by-Step)

### **ระดับ 1: Foundation (เริ่มต้น) - 2-3 ชั่วโมง**

#### 1.1 เข้าใจ Hardware (30 นาที)
**อ่านไฟล์นี้ก่อน:**
```
📁 src/core/hardware_config.h
```
**เรียนรู้:**
- MCU: GD32F103C8T6 (ARM Cortex-M3, 72/120MHz)
- Pin mapping (PC6=injector, PB5=PWM, PC8=enable, PA15=LED)
- Peripheral ที่ใช้ (TIMER2, TIMER7, ADC, USART, DMA)

**แบบฝึกหัด:**
- [ ] วาดแผนผัง block diagram ของระบบ
- [ ] ระบุหน้าที่ของแต่ละ pin

---

#### 1.2 เข้าใจ System Architecture (30 นาที)
**อ่านไฟล์นี้ต่อ:**
```
📁 src/main.c                  (main entry point)
📁 src/core/system_init.c      (hardware initialization)
📁 platformio.ini              (build configuration)
```

**เรียนรู้:**
- Boot sequence: main() → system_init() → FreeRTOS tasks
- 3 Tasks หลัก: task_main, task_bluetooth, task_serial
- Clock configuration (72MHz/120MHz)

**แบบฝึกหัด:**
- [ ] เขียน flowchart ของ boot sequence
- [ ] อธิบายหน้าที่ของแต่ละ task

---

#### 1.3 เข้าใจ Data Structures (30 นาที)
**อ่านไฟล์นี้:**
```
📁 src/core/global.h           (global variables & structs)
📁 src/main.h                  (main data structures)
```

**เรียนรู้:**
- `sPC` - Pump control data (RPM, PWM, PID)
- `HM10` - Bluetooth communication buffer
- `fus` - Fuel map & settings
- Flash memory layout (0x08030000-0x08040000)

**แบบฝึกหัด:**
- [ ] วาด memory map ของ flash
- [ ] อธิบาย struct sPC แต่ละ field

---

#### 1.4 เข้าใจ Project Structure (30 นาที)
```
Fuel_pump_demo_os/
├── src/
│   ├── core/              ← System initialization, config
│   ├── drivers/           ← Hardware abstraction (GPIO, TIMER, ADC, UART, DMA)
│   ├── modules/           ← Business logic (pump control, RPM calc, interpolation)
│   ├── communication/     ← Bluetooth, Serial protocol
│   ├── storage/           ← Flash memory management
│   └── tasks/             ← FreeRTOS tasks
├── include/               ← FreeRTOS config
└── lib/                   ← FreeRTOS library
```

**Design Pattern:**
- **Layered Architecture**: Hardware → Driver → Module → Task
- **Separation of Concerns**: แยก I/O, control logic, communication

---

### **ระดับ 2: Core Functionality (หัวใจของระบบ) - 4-5 ชั่วโมง**

#### 2.1 Input Sensing: RPM Detection (1 ชั่วโมง)
**อ่านไฟล์ตามลำดับ:**
```
1. 📁 src/drivers/timer_driver.c       → init_injector()
2. 📁 src/modules/rpm_calculator.c     → calGeneral()
3. 📁 src/drivers/timer_driver.c       → TIMER7_Channel_IRQHandler()
```

**เรียนรู้:**
- TIMER7 capture mode (CH0=rising, CH1=falling, CH2=timeout)
- คำนวณ RPM จาก injector pulse: `RPM = 60,000,000 / (pulse_width_us * cylinders)`
- Timeout detection (500ms)

**ทดลอง:**
```c
// Test: ถ้า injector 44Hz (22.7ms period)
// → RPM = 60,000,000 / (22,700 * 4) = 661 RPM? ผิด!
// → RPM = 60,000,000 / (11,350 * 2) = 2642 RPM? ผิด!
// จริงๆ: RPM = (Freq * 60) / (pulses_per_rev)
// → RPM = (44 * 60) / 2 = 1320 RPM (สำหรับ 4-stroke, 4-cyl)
```

**แบบฝึกหัด:**
- [ ] คำนวณ RPM ถ้า injector pulse = 33Hz
- [ ] อธิบายว่าทำไมใช้ CH0 + CH1 แทน CH0 อย่างเดียว
- [ ] ทดลองเปลี่ยน timeout เป็น 1000ms แล้วดูผล

---

#### 2.2 Output Control: PWM & PID (1.5 ชั่วโมง)
**อ่านไฟล์ตามลำดับ:**
```
1. 📁 src/drivers/timer_driver.c       → init_PwmFet()
2. 📁 src/modules/pump_control.c       → controlPump()
3. 📁 src/drivers/adc_driver.c         → ADC0_1_IRQHandler()
```

**เรียนรู้:**
- TIMER2 CH1 PWM @ 120kHz (period=10000, prescaler=15)
- PID control (ปิดใช้งานเพราะใช้ R1k load)
- Open-drain output (PB5) + pull-up resistor
- ADC feedback (PC0 → current sensing)

**ทดลอง:**
```c
// Test: เปลี่ยน duty cycle
timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, 5000); // 50%
timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, 7500); // 75%
```

**แบบฝึกหัด:**
- [ ] คำนวณ PWM frequency จาก prescaler & period
- [ ] อธิบายทำไมใช้ GPIO_MODE_AF_OD แทน GPIO_MODE_AF_PP
- [ ] แก้ PID parameters (Kp, Ki, Kd) แล้วทดสอบ

---

#### 2.3 Fuel Map & Interpolation (1 ชั่วโมง)
**อ่านไฟล์ตามลำดับ:**
```
1. 📁 src/modules/interpolation.c      → calInterPolate2D()
2. 📁 src/modules/pump_control.c       → ดู fuel map usage
```

**เรียนรู้:**
- 2D Bilinear Interpolation (12 RPM points × 6 TPS points)
- Fuel map: `fus.MAP_Fuel[TPS_index][RPM_index]`
- Linear interpolation: `y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)`

**ทดลอง:**
```c
// Test: คำนวณมือ
// RPM=1500, TPS=50%
// Fuel map:
//   RPM: [1000, 2000]
//   TPS: [0%, 100%]
//   Values: [[100, 150], [200, 250]]
// Expected: interpolate(1500, 50%) = ?
```

**แบบฝึกหัด:**
- [ ] เขียน Excel sheet คำนวณ interpolation
- [ ] แก้ fuel map เพิ่ม 10% ทุกจุด
- [ ] เพิ่ม 3D map (RPM × TPS × Boost)

---

#### 2.4 Flash Memory Management (1 ชั่วโมง)
**อ่านไฟล์ตามลำดับ:**
```
1. 📁 src/storage/flash_memory.c       → mem_write(), find_last_index()
2. 📁 src/main.c                       → FLASH_Erase_Page()
```

**เรียนรู้:**
- Flash layout: 256 slots × 256 bytes (0x08030000-0x08040000)
- Circular buffer: เขียนครบ page → erase → เขียนต่อ
- Wear leveling: `index = (last_index + 1) % FLASH_SLOT_MAX`

**ทดลอง:**
```c
// Test: เขียน settings 10 ครั้ง
for(int i=0; i<10; i++) {
    fus.Set_0.target_pwm = i * 10;
    mem_write();
    osDelay(100);
}
// เช็คว่า find_last_index() หา index ล่าสุดถูกมั้ย?
```

**แบบฝึกหัด:**
- [ ] คำนวณ flash lifetime (100K erase cycles)
- [ ] เขียน function read_slot(index)
- [ ] เพิ่ม CRC16 checksum

---

### **ระดับ 3: Communication (สื่อสาร) - 3-4 ชั่วโมง**

#### 3.1 Bluetooth Protocol (1.5 ชั่วโมง)
**อ่านไฟล์ตามลำดับ:**
```
1. 📁 src/drivers/uart_driver.c        → init_uart_bluetooth()
2. 📁 src/communication/bluetooth.c    → checksumhm10(), getdatasetupmobile()
3. 📁 src/tasks/task_bluetooth.c       → task_bluetooth()
```

**เรียนรู้:**
- USART2 @ 115200 baud (PB10=TX, PB11=RX)
- DMA circular buffer (5 buffers × 100 bytes)
- Protocol: `#COMMAND<data>` + checksum
- Commands: #SET0, #SET1, #SET2, #TTUN0-5, #INJ, #SETB, #CLEAR

**ทดลอง:**
```
// Test: ส่งคำสั่งจาก Serial Monitor
#SET01000050030          → ตั้ง target PWM = 1000
#TTUN0500070008000900A00 → ตั้ง RPM map
#M                       → Request monitor data
```

**แบบฝึกหัด:**
- [ ] คำนวณ checksum ของ "#SET01000050030"
- [ ] เขียน Python script ส่งคำสั่งผ่าน Bluetooth
- [ ] เพิ่มคำสั่งใหม่ #GETLOG (อ่าน flash log)

---

#### 3.2 Serial Bootloader (1 ชั่วโมง)
**อ่านไฟล์ตามลำดับ:**
```
1. 📁 src/drivers/uart_driver.c        → init_uart0()
2. 📁 src/tasks/task_serial.c          → task_serial()
3. 📁 src/communication/serial_io.c    → serial_event_flags
```

**เรียนรู้:**
- USART0 @ 115200 baud, 9-bit, even parity
- Protocol: 0x7F → 0x79 (ACK)
- Commands: 0x00=device info, 0x81=jump to bootloader, 0x83=save token

**ทดลอง:**
```python
# Test: Send bootloader command
import serial
ser = serial.Serial('COM3', 115200, parity='E', bytesize=9)
ser.write(b'\x7F')  # Sync
response = ser.read(1)  # Expect 0x79
print(f"Response: {response.hex()}")
```

**แบบฝึกหัด:**
- [ ] เขียน Python script ทดสอบ bootloader protocol
- [ ] เพิ่มคำสั่ง 0x84 (read flash)
- [ ] ทำ firmware update ผ่าน serial

---

#### 3.3 Monitor & Streaming (30 นาที)
**อ่านไฟล์:**
```
📁 src/communication/bluetooth.c       → s_dataBTN()
```

**เรียนรู้:**
- Monitor format: `#M<RPM><PWM><ADC><flags>...`
- Streaming @ 10Hz (ทุก 100ms)
- Data packing: 16-bit integers → ASCII hex

**ทดลอง:**
```
// Test: Decode monitor data
// #M158704400026000000003110000AA
// RPM: 0x1587 = 5511 → 551.1 RPM (หาร 10)
// PWM: 0x0440 = 1088 → 10.88% (หาร 100)
```

**แบบฝึกหัด:**
- [ ] เขียน Python script decode monitor data
- [ ] Plot real-time graph (RPM vs PWM)
- [ ] เพิ่ม data compression (reduce bandwidth)

---

### **ระดับ 4: Advanced Topics (ขั้นสูง) - 3-4 ชั่วโมง**

#### 4.1 FreeRTOS Deep Dive (1.5 ชั่วโมง)
**อ่านไฟล์:**
```
1. 📁 include/FreeRTOSConfig.h         → RTOS configuration
2. 📁 src/tasks/*.c                    → Task implementation
3. 📁 lib/FreeRTOS/src/tasks.c         → RTOS internals (optional)
```

**เรียนรู้:**
- Task priorities (main=1, bluetooth=2, serial=3)
- Stack size (128-256 words)
- Heap management (heap_4.c)
- osDelay(), osEventFlagsSet(), osThreadYield()

**ทดลอง:**
```c
// Test: CPU utilization
void vTaskGetRunTimeStats(char *pcWriteBuffer);
osDelay(5000);
vTaskGetRunTimeStats(buffer);
printf("%s", buffer);
```

**แบบฝึกหัด:**
- [ ] วัด stack usage ของแต่ละ task
- [ ] เพิ่ม task ใหม่ (task_logger)
- [ ] ทดลอง priority inversion problem

---

#### 4.2 Security & Authentication (1 ชั่วโมง)
**อ่านไฟล์:**
```
📁 src/main.c                          → save_token(), checkIdCpu()
```

**เรียนรู้:**
- Token storage @ 0x0803F800 (XOR encrypted)
- CPU ID verification (96-bit unique ID)
- Flash write protection

**ทดลอง:**
```c
// Test: Read CPU ID
uint32_t id[3];
id[0] = *(uint32_t*)0x1FFFF7E8;
id[1] = *(uint32_t*)0x1FFFF7EC;
id[2] = *(uint32_t*)0x1FFFF7F0;
printf("CPU ID: %08X-%08X-%08X\n", id[0], id[1], id[2]);
```

**แบบฝึกหัด:**
- [ ] เพิ่ม AES-128 encryption
- [ ] ทำ challenge-response authentication
- [ ] เพิ่ม secure boot

---

#### 4.3 LED Effects (30 นาที)
**อ่านไฟล์:**
```
📁 src/modules/led_effects.c           → LED breathing effect
📁 src/drivers/timer_driver.c          → init_LED()
```

**เรียนรู้:**
- TIMER1 CH0 PWM (PA15)
- Breathing effect: fade 0→150→0
- State machine: idle → breathing → error

**ทดลอง:**
```c
// Test: เปลี่ยน pattern
// Blink: on 500ms, off 500ms
// Fast blink: on 100ms, off 100ms
```

**แบบฝึกหัด:**
- [ ] เพิ่ม RGB LED support (3 channels)
- [ ] เพิ่ม pattern: rainbow, strobe
- [ ] ทำ LED แสดง error code (morse code)

---

### **ระดับ 5: Debugging & Optimization (ขั้นเทพ) - 2-3 ชั่วโมง**

#### 5.1 Debug Techniques (1 ชั่วโมง)
**Tools:**
- ST-Link debugger (GDB)
- Logic analyzer (Saleae)
- Oscilloscope (PWM, ADC)

**ทดลอง:**
```gdb
# GDB debugging
break pump_control.c:controlPump
continue
print sPC.rpm
print sPC.pwm_duty
watch sPC.pwm_duty
```

**แบบฝึกหัด:**
- [ ] ดู PWM waveform บน oscilloscope
- [ ] ดู UART data บน logic analyzer
- [ ] ทำ stack trace ตอน hard fault

---

#### 5.2 Performance Optimization (1 ชั่วโมง)
**Profiling:**
```c
// Measure execution time
uint32_t start = DWT->CYCCNT;
controlPump();
uint32_t cycles = DWT->CYCCNT - start;
printf("controlPump: %lu cycles (%.2f us)\n", 
       cycles, cycles / 72.0f);
```

**Optimization Techniques:**
- [ ] Replace float with fixed-point (Q16.16)
- [ ] Use lookup tables instead of interpolation
- [ ] Enable compiler optimization (-O2 → -O3)
- [ ] Use DMA for data transfer

---

## 🎯 แผนการเรียนรู้ (Learning Path)

### **สัปดาห์ที่ 1: Foundation**
- วันที่ 1-2: ระดับ 1 (Foundation)
- วันที่ 3-5: ระดับ 2.1-2.2 (Input/Output)
- วันที่ 6-7: ระดับ 2.3-2.4 (Fuel Map/Flash)

### **สัปดาห์ที่ 2: Communication**
- วันที่ 1-3: ระดับ 3.1-3.2 (Bluetooth/Serial)
- วันที่ 4-5: ระดับ 3.3 (Monitor)
- วันที่ 6-7: ทดสอบทั้งระบบ

### **สัปดาห์ที่ 3: Advanced**
- วันที่ 1-2: ระดับ 4.1 (FreeRTOS)
- วันที่ 3-4: ระดับ 4.2-4.3 (Security/LED)
- วันที่ 5-7: ระดับ 5 (Debug/Optimization)

---

## 📚 แหล่งเรียนรู้เพิ่มเติม

### **Documentation**
- [GD32F103 Datasheet](https://www.gigadevice.com/products/microcontrollers/gd32/arm-cortex-m3/)
- [GD32F103 User Manual](https://www.gigadevice.com/)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [ARM Cortex-M3 Guide](https://developer.arm.com/documentation/)

### **Video Tutorials**
- FreeRTOS Basics (YouTube)
- PID Control Theory (YouTube)
- Embedded C Programming (Udemy)

### **Books**
- "Embedded Systems with ARM Cortex-M" - Jonathan Valvano
- "FreeRTOS Real-Time Kernel" - Richard Barry
- "Designing Embedded Systems" - Jack Ganssle

---

## ✅ Checklist: ความเข้าใจ

### **Foundation**
- [ ] อธิบาย hardware architecture ได้
- [ ] เขียน flowchart ของระบบได้
- [ ] อธิบาย memory layout ได้

### **Core Functionality**
- [ ] คำนวณ RPM จาก injector pulse ได้
- [ ] ตั้งค่า PWM duty cycle ได้
- [ ] อธิบาย PID control ได้
- [ ] ใช้ 2D interpolation ได้
- [ ] เขียน/อ่าน flash memory ได้

### **Communication**
- [ ] ส่ง/รับ Bluetooth command ได้
- [ ] ทำ serial bootloader protocol ได้
- [ ] Decode monitor data ได้

### **Advanced**
- [ ] สร้าง/ลบ FreeRTOS task ได้
- [ ] Debug ด้วย GDB ได้
- [ ] Optimize code performance ได้

---

## 🚀 โปรเจคทดลอง (Practice Projects)

### **Beginner**
1. เปลี่ยน LED breathing → blink pattern
2. เพิ่ม Bluetooth command ใหม่ (#GETRPM)
3. แก้ fuel map เพิ่ม 20%

### **Intermediate**
1. เพิ่ม data logging (เก็บ 1000 samples)
2. ทำ Python dashboard แสดง real-time graph
3. Enable PID และ tune parameters

### **Advanced**
1. เพิ่ม CAN bus interface
2. ทำ OTA firmware update
3. เพิ่ม predictive maintenance (AI)

---

## 💡 เคล็ดลับการเรียนรู้

1. **เรียนรู้ทีละส่วน** - อย่าพยายามเข้าใจทั้งหมดในครั้งเดียว
2. **ทดลองเขียนโค้ด** - อ่านอย่างเดียวไม่พอ ต้องลงมือทำ
3. **ใช้ debugger** - ดู variable real-time
4. **วาดแผนภาพ** - block diagram, flowchart, timing diagram
5. **ถามคำถาม** - ถ้าไม่เข้าใจถามเลย!

---

## 📞 ติดต่อสอบถาม

มีคำถามหรือติดปัญหา? สามารถถามได้เลยครับ! 🚀

**Good luck with your learning journey!** 💪
