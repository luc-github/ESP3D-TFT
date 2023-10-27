#include <Arduino.h>
#include <time.h>
#include <string.h>


// Update hash
void update_hash(uint8_t *data, size_t len, uint8_t * hash_buffer, uint8_t hash_size) {
  static bool reverse = false;
  reverse=!reverse;
  int start_index = reverse?hash_size:0;
  for (int i = 0; i < hash_size; i++) {
     int idx = reverse?(start_index - i) % hash_size: (start_index + i) % hash_size;
    if (i>=len){
      hash_buffer[idx] ^= random(1,254); 
    }else {
    hash_buffer[idx] ^= data[i]; 
    }
  }
}

const char * generateToken(const char * path){
  // Buffer for hash
  uint8_t hash_buffer[HASH_SIZE];
  memset(hash_buffer, 0, HASH_SIZE);
  static String token;
  String tmp = "";
  //Use path 
  update_hash((uint8_t*)path, strlen(path), hash_buffer, HASH_SIZE);

  //use epoch time
  uint64_t millisec = millis();
  update_hash((uint8_t*)&millisec, sizeof(millisec), hash_buffer, HASH_SIZE);

  //use current time 
   time_t now;
   time(&now);
   update_hash((uint8_t*)&now, sizeof(now), hash_buffer, HASH_SIZE);

  //now hash all the buffer 
  for (int i = 0; i < HASH_SIZE; i++) {
    char hex[3];
    sprintf(hex, "%02x", hash_buffer[i]); 
    tmp += hex;
  }
  //format the uuid on 36 chars
  token = tmp.substring(0,8) + "-";
  token += tmp.substring(8,12) + "-";
  token += tmp.substring(12,16) + "-";
  token += tmp.substring(16,20) + "-";
  token += &tmp[20];
  return token.c_str();
}

void setup(){
  Serial.begin(115200);
  Serial.println("Setup done");
}

void loop(){
  delay(2000);
  Serial.println(generateToken("/sd/test.txt"));
}
