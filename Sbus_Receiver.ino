HardwareSerial SBUSSerial(2);

#define SBUS_RX_PIN 35

static const uint8_t SBUS_FRAME_SIZE = 25;
static const uint8_t SBUS_HEADER = 0x0F;

// Dữ liệu SBUS thô
uint8_t sbusFrame[SBUS_FRAME_SIZE];
uint16_t sbusChannelsRaw[16];

// Dữ liệu RC đã map 1000-2000


bool sbusFailSafe = false;
bool sbusLostFrame = false;

// =========================
// Hàm đọc 1 frame SBUS
// =========================
bool readSbusFrame(uint8_t *frame) {
  static uint8_t buffer[SBUS_FRAME_SIZE];
  static uint8_t index = 0;

  while (SBUSSerial.available()) {
    uint8_t b = SBUSSerial.read();

    if (index == 0) {
      if (b != SBUS_HEADER) {
        continue;
      }
    }

    buffer[index++] = b;

    if (index == SBUS_FRAME_SIZE) {
      index = 0;
      memcpy(frame, buffer, SBUS_FRAME_SIZE);
      return true;
    }
  }

  return false;
}

// =========================
// Giải mã 16 kênh SBUS
// =========================
void decodeSbusChannels(const uint8_t *frame, uint16_t *channels, bool &lostFrame, bool &failsafe) {
  channels[0]  = ((frame[1]      | frame[2]  << 8) & 0x07FF);
  channels[1]  = ((frame[2] >> 3 | frame[3]  << 5) & 0x07FF);
  channels[2]  = ((frame[3] >> 6 | frame[4]  << 2 | frame[5] << 10) & 0x07FF);
  channels[3]  = ((frame[5] >> 1 | frame[6]  << 7) & 0x07FF);
  channels[4]  = ((frame[6] >> 4 | frame[7]  << 4) & 0x07FF);
  channels[5]  = ((frame[7] >> 7 | frame[8]  << 1 | frame[9] << 9) & 0x07FF);
  channels[6]  = ((frame[9] >> 2 | frame[10] << 6) & 0x07FF);
  channels[7]  = ((frame[10] >> 5 | frame[11] << 3) & 0x07FF);

  channels[8]  = ((frame[12]      | frame[13] << 8) & 0x07FF);
  channels[9]  = ((frame[13] >> 3 | frame[14] << 5) & 0x07FF);
  channels[10] = ((frame[14] >> 6 | frame[15] << 2 | frame[16] << 10) & 0x07FF);
  channels[11] = ((frame[16] >> 1 | frame[17] << 7) & 0x07FF);
  channels[12] = ((frame[17] >> 4 | frame[18] << 4) & 0x07FF);
  channels[13] = ((frame[18] >> 7 | frame[19] << 1 | frame[20] << 9) & 0x07FF);
  channels[14] = ((frame[20] >> 2 | frame[21] << 6) & 0x07FF);
  channels[15] = ((frame[21] >> 5 | frame[22] << 3) & 0x07FF);

  lostFrame = frame[23] & (1 << 2);
  failsafe  = frame[23] & (1 << 3);
}

// =========================
// Chuyển SBUS -> RC 1000-2000
// SBUS thường khoảng 172 -> 1811
// =========================
int sbusToRC(uint16_t val) {
  if (val < 172) val = 172;
  if (val > 1811) val = 1811;

  return (int)((val - 172) * 1000.0 / (1811 - 172) + 1000);
}

// =========================
// Deadband quanh 1500
// =========================
int applyDeadband(int val, int center = 1500, int db = 10) {
  if (abs(val - center) < db) return center;
  return val;
}

// =========================
// In toàn bộ kênh RC
// =========================
void printAllChannels() {
  for (int i = 0; i < 16; i++) {
    Serial.print("CH");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(rcChannels[i]);
    Serial.print("  ");
  }

  Serial.print("FS: ");
  Serial.print(sbusFailSafe);
  Serial.print("  LF: ");
  Serial.println(sbusLostFrame);
}
void sbus_setup() {
    SBUSSerial.begin(100000, SERIAL_8E2, SBUS_RX_PIN, -1, true);
}
void read_sbus() {

    if (readSbusFrame(sbusFrame)) {
    decodeSbusChannels(sbusFrame, sbusChannelsRaw, sbusLostFrame, sbusFailSafe);

    // Map toàn bộ 16 kênh sang 1000-2000
    for (int i = 0; i < 16; i++) {
      rcChannels[i] = sbusToRC(sbusChannelsRaw[i]);
    }
    }
    // printAllChannels();
}