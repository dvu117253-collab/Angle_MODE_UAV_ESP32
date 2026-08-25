
// // ===================== CRSF constants =====================
// static const uint8_t CRSF_ADDR_FLIGHT_CONTROLLER  = 0xC8;
// static const uint8_t CRSF_ADDR_RADIO_TRANSMITTER  = 0xEA;
// static const uint8_t CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16;

// // ===================== Ring buffer =====================
// // Chọn power-of-2 để modulo nhanh
// static const uint16_t RB_SIZE = 256; // đủ lớn cho UART burst
// static uint8_t rb[RB_SIZE];
// static uint16_t rbHead = 0; // write
// static uint16_t rbTail = 0; // read

// static inline uint16_t rbCount() {   
//   return (uint16_t)((rbHead - rbTail) & (RB_SIZE - 1));
// }
// static inline bool rbIsFull() {
//   return rbCount() == (RB_SIZE - 1);
// }
// static inline bool rbIsEmpty() {
//   return rbHead == rbTail;
// }
// static inline void rbPush(uint8_t b) {
//   if (rbIsFull()) {
//     // Nếu đầy: bỏ 1 byte cũ để luôn nhận dữ liệu mới (tránh kẹt)
//     rbTail = (rbTail + 1) & (RB_SIZE - 1);
//   }
//   rb[rbHead] = b;
//   rbHead = (rbHead + 1) & (RB_SIZE - 1);
// }
// static inline uint8_t rbPeek(uint16_t index) {
//   // index tính từ tail: 0 = byte đầu tiên chưa đọc
//   uint16_t pos = (rbTail + index) & (RB_SIZE - 1);
//   return rb[pos];
// }
// static inline void rbPop(uint16_t n) {
//   rbTail = (rbTail + n) & (RB_SIZE - 1);
// }


// static uint8_t crsf_crc8(const uint8_t *ptr, uint8_t len) {
//   uint8_t crc = 0;
//   while (len--) {
//     crc ^= *ptr++;
//     for (uint8_t i = 0; i < 8; i++) {
//       if (crc & 0x80) crc = (crc << 1) ^ 0xD5;
//       else crc <<= 1;
//     }
//   }
//   return crc;
// }

// static void decodeRcChannelsPacked(const uint8_t *p22) {
//   uint32_t bits = 0;
//   uint8_t bitCount = 0;
//   int out = 0;

//   for (int i = 0; i < 22; i++) {
//     bits |= ((uint32_t)p22[i]) << bitCount;
//     bitCount += 8;

//     while (bitCount >= 11 && out < 16) {
//       ch11[out++] = bits & 0x7FF;
//       bits >>= 11;
//       bitCount -= 11;
//     }
//   }
// }

// // Map CRSF raw -> microseconds (kẹp 1000..2000)
// static int crsf11ToUs(uint16_t v11) {
//   // 172 ≈ 988us, 1811 ≈ 2012us
//   float us = 988.0f + ((float)(v11 - 172) * (2012.0f - 988.0f) / (1811.0f - 172));
//   if (us < 1000) us = 1000;
//   if (us > 2000) us = 2000;
//   return (int)(us + 0.5f);
// }

// static inline bool isCrsfAddr(uint8_t a) {
//   return (a == CRSF_ADDR_FLIGHT_CONTROLLER) || (a == CRSF_ADDR_RADIO_TRANSMITTER);
// }

// static void parseCrsfFromRing() {
//   // packet: [addr][len][type][payload...][crc]
//   // len = bytes from type..crc (>=2)
//   uint8_t pkt[64];

//   while (rbCount() >= 5) {
//     uint8_t addr = rbPeek(0);
//     if (!isCrsfAddr(addr)) {
//       rbPop(1); // resync
//       continue;
//     }

//     uint8_t len = rbPeek(1);
//     if (len < 2 || len > 62) {
//       rbPop(1); // len vô lý -> bỏ 1 byte để resync
//       continue;
//     }

//     uint16_t fullSize = (uint16_t)(2 + len);
//     if (rbCount() < fullSize) return; // chờ đủ dữ liệu

//     // copy packet ra buffer tạm
//     for (uint16_t i = 0; i < fullSize; i++) {
//       pkt[i] = rbPeek(i);
//     }
//     rbPop(fullSize);

//     uint8_t type = pkt[2];
//     uint8_t payloadLen = len - 2;
//     const uint8_t *payload = &pkt[3];
//     uint8_t crc_rx = pkt[2 + len - 1];

//     // CRC trên [type + payload]
//     uint8_t crc_calc = crsf_crc8(&pkt[2], (uint8_t)(1 + payloadLen));
//     if (crc_calc != crc_rx) {
//       // CRC sai: bỏ packet này, tiếp tục resync
//       continue;
//     }

//     if (type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED && payloadLen >= 22) {
//       decodeRcChannelsPacked(payload);
//       lastRcPacketMs = millis();
//     }
//   }
// }
// void read_TX() {
//     // 1) đọc UART và đẩy vào ring buffer
//   while (Serial2.available()) {
//     rbPush((uint8_t)Serial2.read());
//   }

//   // 2) parse CRSF từ ring buffer
//   parseCrsfFromRing();

//   // 3) in 10 channel (gọn, nhẹ hơn)
//   static uint32_t lastPrintMs = 0;
//   if (millis() - lastPrintMs >= 50) { // 20Hz
//     lastPrintMs = millis();

//     bool linkOk = (millis() - lastRcPacketMs) < 250;
//     // Serial.print(linkOk ? "OK," : "LOST,");

//     // CSV: CH1..CH10 theo microseconds
//     // for (int i = 0; i < 10; i++) {
//     //   Serial.print(crsf11ToUs(ch11[i]));
//     //   if (i < 9) Serial.print(',');
//     // }
//     // Serial.println();
//   }
// }