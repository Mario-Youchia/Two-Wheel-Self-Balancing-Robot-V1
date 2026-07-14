// The following information is based on the MPU6050 datasheet, and the MPU6050 register map

#include <Wire.h>

// First: Begin by defining the address of the MPU6050 slave, its default value is 0x68, but it could also be 0x69 depending on how
// you are connecting the AD0 pin. When connecting the AD0 to VCC, the sensor's address will become 0b0110 1001
// Second: Start Communication between Arduino and MPU6050 as shown below in the initialization block
// Third: Defining the operating range of accelerometer
// Fourth: Defining the operating range of gyroscope
// Fifth: Constructing and Printing values
#define I2C_Address_Of_MPU6050 0x68
#define MPU6050_PWR_MGMT_1_Register 0x6B
#define ACCEL_CONFIG 0x1C
#define GYRO_CONFIG 0x1B
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H 0x41
#define TEMP_OUT_L 0x42
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48

void setup() {
  // Initializing the serial communication baud rate for debugging purposes
  Serial.begin(9600);
  Serial.println("\nEstablishing connection with MPU6050");
  ///////////////////////////////// The following communication is done according to the standards of I2C protocol //////////////////////////////////
  ///////////////// Initialization /////////////////
  Wire.begin();                                    // Beginning the I2C Communication using Wire Library
  Wire.beginTransmission(I2C_Address_Of_MPU6050);  // Communication with a slave of address 0x68, then waits for ACK or NACK
  Wire.write(MPU6050_PWR_MGMT_1_Register);         // Writing to the main power register 0x6B of MPU6050 to wake it up.
  // Note: the PWR_MGMT_2 is not used as we not going to modify any of the power settings of internal components.
  // Writing 0x00 to the PWR_MGMT_1 register selects the internal oscillator of MPU6050 to be used for synchronization.
  // Waiting for ACK or NACK to be sent before writing the following segment, this is to ensure that the PWR register is
  // ready to receive data.
  Wire.write(0x00);  // Writes the data (0x00) to the register, and then waits for ACK or NACK.
  // Finally, ending the first transmission that intializes the sensor.
  Wire.endTransmission(true);

  ///////////////////////////////// Defining the range of accelerometer /////////////////////////////////
  Wire.beginTransmission(I2C_Address_Of_MPU6050);  // Same as before
  Wire.write(ACCEL_CONFIG);                        // Writing to Accelerometer Settings Register
  // Choosing the suitable range for your application:
  // ||||||||||||||||||||||||||||||||
  // |||AFS_SEL|||Full Scale Range|||
  // |||   0   |||     +/- 2g     |||
  // |||   1   |||     +/- 4g     |||
  // |||   2   |||     +/- 8g     |||
  // |||   3   |||     +/- 16g    |||
  // ||||||||||||||||||||||||||||||||
  // The first and last 3 bits are all zeros, while the AFS_SEL bits in the middle are dedicated to operating range selection.
  // I will choose the +/-16g range as an example! So, AFS_SEL bits are 11
  // Then, we will write: 000 11 000 = (00011000)
  Wire.write(0b00011000);  // Here we write the required range, and we will read it later on to check if it has been changed or not.
  // Remember: in I2C after each segment transmission or reception there should be an ACK or NACK from the other side.
  Wire.endTransmission(true);  // Terminating the accelerometer range selection.
  ///////////////// Checking Range /////////////////
  Wire.beginTransmission(I2C_Address_Of_MPU6050);  // Same as before
  Wire.write(ACCEL_CONFIG);                        // Same as before, but this time we will NOT write to the ACCEL_CONFIG register,
  // but we will request data from it.
  Wire.endTransmission(true);
  // Requesting data from the register. Note, the ACCEL_CONFIG contains 8 bits, and it should return 0b00011000, this is equivalent to 1 byte.
  Wire.requestFrom(I2C_Address_Of_MPU6050, 1);  // We will request 1 byte of data, as ACCEL_CONFIG contains only 1 byte.
  if (Wire.available()) {
    int accel_config = Wire.read();
    int i = 0;
    // Applying bitwise masking as follows:
    // Assume that the received data is 0b00011000
    // 1) Right shift 0b10000000 by (0), apply bitwise AND with the received byte: 0b00011000 & 0b10000000 => 0b00000000
    // 2) Right shift 0b10000000 by (1), apply bitwise AND with the received byte: 0b00011000 & 0b01000000 => 0b00000000
    // 3) Right shift 0b10000000 by (1), apply bitwise AND with the received byte: 0b00011000 & 0b00100000 => 0b00000000
    // 4) Right shift 0b10000000 by (3), apply bitwise AND with the received byte: 0b00011000 & 0b00010000 => 0b00010000
    // 5) Right shift 0b10000000 by (4), apply bitwise AND with the received byte: 0b00011000 & 0b00001000 => 0b00001000
    // 6) Right shift 0b10000000 by (5), apply bitwise AND with the received byte: 0b00011000 & 0b00000100 => 0b00000000
    // 7) Right shift 0b10000000 by (6), apply bitwise AND with the received byte: 0b00011000 & 0b00000010 => 0b00000000
    // 8) Right shift 0b10000000 by (7), apply bitwise AND with the received byte: 0b00011000 & 0b00000001 => 0b00000000
    // The printed result will be: 0 0 0 1 1 0 0 0
    Serial.print("accel_config = ");
    while (i < 8) Serial.print(String((accel_config & (0b10000000 >> i)) == (0b10000000 >> i++)) + " ");
    Serial.println("");
  }

  ///////////////////////////////// Defining the range of gyroscope /////////////////////////////////
  Wire.beginTransmission(I2C_Address_Of_MPU6050);  // Same as before
  Wire.write(GYRO_CONFIG);                         // Writing to Gyroscope Settings Register
  // Choosing the suitable range for your application:
  // ||||||||||||||||||||||||||||||||
  // ||| FS_SEL|||Full Scale Range|||
  // |||   0   |||   +/- 250 °/s  |||
  // |||   1   |||   +/- 500 °/s  |||
  // |||   2   |||   +/- 1000 °/s |||
  // |||   3   |||   +/- 2000 °/s |||
  // ||||||||||||||||||||||||||||||||
  // The first and last 3 bits are all zeros, while the FS_SEL bits in the middle are dedicated to operating range selection.
  // I will choose the +/- 500 °/s range as an example! So, FS_SEL bits are 01
  // Then, we will write: 000 01 000 = 0b00001000
  Wire.write(0b00001000);  // Here we write the required range, and we will read it later on to check if it has been changed or not.
  // Remember: in I2C after each segment transmission or reception there should be an ACK or NACK from the other side.
  Wire.endTransmission(true);  // Terminating the gyroscope range selection.
  ///////////////// Checking Range /////////////////
  Wire.beginTransmission(I2C_Address_Of_MPU6050);  // Same as before
  Wire.write(GYRO_CONFIG);                         // Same as before, but this time we will NOT write to the GYRO_CONFIG register,
  // but we will request data from it.
  Wire.endTransmission(true);
  // Requesting data from the register. Note, the GYRO_CONFIG contains 8 bits, and it should return 0b00001000, this is equivalent to 1 byte.
  Wire.requestFrom(I2C_Address_Of_MPU6050, 1);  // We will request 1 byte of data, as GYRO_CONFIG contains only 1 byte.
  if (Wire.available()) {
    int gyro_config = Wire.read();
    int i = 0;
    // Applying bitwise masking as follows:
    // Assume that the received data is 0b00001000
    // 1) Right shift 0b10000000 by (0), apply bitwise AND with the received byte: 0b00001000 & 0b10000000 => 0b00000000
    // 2) Right shift 0b10000000 by (1), apply bitwise AND with the received byte: 0b00001000 & 0b01000000 => 0b00000000
    // 3) Right shift 0b10000000 by (1), apply bitwise AND with the received byte: 0b00001000 & 0b00100000 => 0b00000000
    // 4) Right shift 0b10000000 by (3), apply bitwise AND with the received byte: 0b00001000 & 0b00010000 => 0b00000000
    // 5) Right shift 0b10000000 by (4), apply bitwise AND with the received byte: 0b00001000 & 0b00001000 => 0b00001000
    // 6) Right shift 0b10000000 by (5), apply bitwise AND with the received byte: 0b00001000 & 0b00000100 => 0b00000000
    // 7) Right shift 0b10000000 by (6), apply bitwise AND with the received byte: 0b00001000 & 0b00000010 => 0b00000000
    // 8) Right shift 0b10000000 by (7), apply bitwise AND with the received byte: 0b00001000 & 0b00000001 => 0b00000000
    // The printed result will be: 0 0 0 0 1 0 0 0
    Serial.print("gyro_config = ");
    while (i < 8) Serial.print(String((gyro_config & (0b10000000 >> i)) == (0b10000000 >> i++)) + " ");
    Serial.println("");
  }
  // To avoid code repetition, a reading function `Read_Bytes_From_Register` has been created
}

uint8_t Read_Byte_From_Register(int SLAVE_ADDR, int REGISTER_ADDR, String text) {
  // This function reads and prints the byte saved in the given register
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(REGISTER_ADDR);
  Wire.endTransmission(true);
  uint8_t byte = 0;
  Wire.requestFrom(SLAVE_ADDR, 1);
  if (Wire.available()) {
    byte = Wire.read();
    // Applying bitwise masking as follows:
    // Assume that the received data is 0b00001000
    // 1) Right shift 0b10000000 by (0), apply bitwise AND with the received byte: 0b00001000 & 0b10000000 => 0b00000000
    // 2) Right shift 0b10000000 by (1), apply bitwise AND with the received byte: 0b00001000 & 0b01000000 => 0b00000000
    // 3) Right shift 0b10000000 by (1), apply bitwise AND with the received byte: 0b00001000 & 0b00100000 => 0b00000000
    // 4) Right shift 0b10000000 by (3), apply bitwise AND with the received byte: 0b00001000 & 0b00010000 => 0b00000000
    // 5) Right shift 0b10000000 by (4), apply bitwise AND with the received byte: 0b00001000 & 0b00001000 => 0b00001000
    // 6) Right shift 0b10000000 by (5), apply bitwise AND with the received byte: 0b00001000 & 0b00000100 => 0b00000000
    // 7) Right shift 0b10000000 by (6), apply bitwise AND with the received byte: 0b00001000 & 0b00000010 => 0b00000000
    // 8) Right shift 0b10000000 by (7), apply bitwise AND with the received byte: 0b00001000 & 0b00000001 => 0b00000000
    // The printed result will be: 0 0 0 0 1 0 0 0
    Print_Bytes(1, text, byte);
  }
  return byte;
}
void Print_Bytes(int NumberOfBytes, String text, int bytes) {
  int i = 0;
  Serial.print(text + " = ");
  uint16_t mask = 0b10000000 << 8 * (NumberOfBytes - 1);
  while (i < 8 * NumberOfBytes) { Serial.print(String((bytes & (mask >> i)) == (mask >> i++)) + " "); }
  Serial.println("");
}
void loop() {
  Serial.println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-");
  ///////////////////////////////// Constructing and Printing values /////////////////////////////////
  // Printing Acceleration Data
  uint8_t A_X_H = Read_Byte_From_Register(I2C_Address_Of_MPU6050, ACCEL_XOUT_H, "Acceleration_x_H");
  uint8_t A_X_L = Read_Byte_From_Register(I2C_Address_Of_MPU6050, ACCEL_XOUT_L, "Acceleration_x_L");
  uint8_t A_Y_H = Read_Byte_From_Register(I2C_Address_Of_MPU6050, ACCEL_YOUT_H, "Acceleration_y_H");
  uint8_t A_Y_L = Read_Byte_From_Register(I2C_Address_Of_MPU6050, ACCEL_YOUT_L, "Acceleration_y_L");
  uint8_t A_Z_H = Read_Byte_From_Register(I2C_Address_Of_MPU6050, ACCEL_ZOUT_H, "Acceleration_z_H");
  uint8_t A_Z_L = Read_Byte_From_Register(I2C_Address_Of_MPU6050, ACCEL_ZOUT_L, "Acceleration_z_L");
  // Printing Temperature Data
  uint8_t T_H = Read_Byte_From_Register(I2C_Address_Of_MPU6050, TEMP_OUT_H, "Temp_H");
  uint8_t T_L = Read_Byte_From_Register(I2C_Address_Of_MPU6050, TEMP_OUT_L, "Temp_L");
  // Printing Gyroscope Data
  uint8_t G_X_H = Read_Byte_From_Register(I2C_Address_Of_MPU6050, GYRO_XOUT_H, "Gyroscope_x_H");
  uint8_t G_X_L = Read_Byte_From_Register(I2C_Address_Of_MPU6050, GYRO_XOUT_L, "Gyroscope_x_L");
  uint8_t G_Y_H = Read_Byte_From_Register(I2C_Address_Of_MPU6050, GYRO_YOUT_H, "Gyroscope_y_H");
  uint8_t G_Y_L = Read_Byte_From_Register(I2C_Address_Of_MPU6050, GYRO_YOUT_L, "Gyroscope_y_L");
  uint8_t G_Z_H = Read_Byte_From_Register(I2C_Address_Of_MPU6050, GYRO_ZOUT_H, "Gyroscope_z_H");
  uint8_t G_Z_L = Read_Byte_From_Register(I2C_Address_Of_MPU6050, GYRO_ZOUT_L, "Gyroscope_z_L");

  // Constructing Data From Bytes:
  // /\/\/\/\/\/\/\/\/ Example: /\/\/\/\/\/\/\/\/
  // Before consturcting the acceleration in x-asis direction, you should know the following:
  // Acceleration in the x-axis is represented by 2 bytes, with each byte stored in a separate register.
  // The first byte stores the 8 most significant bits (MSBs), while the second byte stores the 8 least significant bits (LSBs).
  // The first bytes is stored in ACCEL_XOUT_H, and the other byte is stored in ACCEL_XOUT_L.
  // Let the first byte be: 1111 1110, and the other byte be: 0100 0001
  // Note: these 16 bits represent a 16 signed integer. This is to ensure that the data is more accurate, and also ranges from -2^15 to 2^15 - 1.
  // In order to construct the 16 bits:
  // 1) Take the first byte (1111 1110) and left shift it by 8 bits: 1111 1110 0000 0000
  // 2) To obtain the complete value, we need to combine the two bytes.
  // We perform a bitwise OR operation between the previous output (1111 1110 0000 0000) and the second byte (0100 0001)
  // 3) The obtained value is: 1111 1110 0100 0001, which is equivalent to -447 in decimal

  // This also applies for the rest.
  int16_t A_X = (int16_t)((uint16_t)A_X_H << 8 | A_X_L);
  int16_t A_Y = (int16_t)((uint16_t)A_Y_H << 8 | A_Y_L);
  int16_t A_Z = (int16_t)((uint16_t)A_Z_H << 8 | A_Z_L);

  int16_t T = (int16_t)((uint16_t)T_H << 8 | T_L);

  int16_t G_X = (int16_t)((uint16_t)G_X_H << 8 | G_X_L);
  int16_t G_Y = (int16_t)((uint16_t)G_Y_H << 8 | G_Y_L);
  int16_t G_Z = (int16_t)((uint16_t)G_Z_H << 8 | G_Z_L);
  Serial.println("----------------------------------------------");
  Print_Bytes(2, "Acceleration_x_Bits", A_X);
  Print_Bytes(2, "Acceleration_y_Bits", A_Y);
  Print_Bytes(2, "Acceleration_z_Bits", A_Z);
  Print_Bytes(2, "Temperature_Bits", T);
  Print_Bytes(2, "Gyroscope_x_Bits", G_X);
  Print_Bytes(2, "Gyroscope_y_Bits", G_Y);
  Print_Bytes(2, "Gyroscope_z_Bits", G_Z);
  Serial.println("----------------------------------------------");
  Serial.println("Raw Values (Before Mapping):");
  Serial.println("Acceleration_x = " + String(A_X) + ", Acceleration_y = " + String(A_Y) + ", Acceleration_z = " + String(A_Z));
  Serial.println("Temperature = " + String(T));
  Serial.println("Gyroscope_x = " + String(G_X) + ", Gyroscope_y = " + String(G_Y) + ", Gyroscope_z = " + String(G_Z));
  Serial.println("----------------------------------------------");
  // We can map the obtained values (from -2^15 to 2^15 - 1) to the range of (0 to 255).
  // Note that it is not mandatory to perform this mapping; you can choose to use the raw values without mapping,
  // or you can opt for the mapped values. Whichever range you decide to use, it's important to maintain consistency
  // throughout the entire project to ensure data consistency.
  // However, it's worth noting that reducing the data to 8 bits instead of 16 bits can result in a decrease in data accuracy in a significant way.
  uint8_t Ax = map(A_X, -32768L, 32767L, 0L, 255L);
  uint8_t Ay = map(A_Y, -32768L, 32767L, 0L, 255L);
  uint8_t Az = map(A_Z, -32768L, 32767L, 0L, 255L);
  uint8_t t  = map(T,   -32768L, 32767L, 0L, 255L);
  uint8_t Gx = map(G_X, -32768L, 32767L, 0L, 255L);
  uint8_t Gy = map(G_Y, -32768L, 32767L, 0L, 255L);
  uint8_t Gz = map(G_Z, -32768L, 32767L, 0L, 255L);
  Serial.println("Mapped Values:");
  Serial.println("Acceleration_x = " + String(Ax) + ", Acceleration_y = " + String(Ay) + ", Acceleration_z = " + String(Az));
  Serial.println("Temperature = " + String(t));
  Serial.println("Gyroscope_x = " + String(Gx) + ", Gyroscope_y = " + String(Gy) + ", Gyroscope_z = " + String(Gz));
  delay(1000);
}
// Finally, this is a demonstration of how to communicate with the MPU6050 sensor, perform read and write operations on different registers, 
// and make modifications as needed.
// It is REALLY worth noting that this implementation is intended ONLY for DEMONSTRATION purposes and is NOT practical in real-world scenarios,
// including the project at hand (Self-Balanced Robot). Instead of reading byte by byte from individual registers, it is more efficient to 
// read 14 bytes from the ACCEL_XOUT_H register of the MPU6050 sensor at once. This approach reduces communication latency, and ensures data 
// consistency across samples, which is the MOST IMPORTANT concern.

// Done by: Mario Y.     Date: 14-May-23