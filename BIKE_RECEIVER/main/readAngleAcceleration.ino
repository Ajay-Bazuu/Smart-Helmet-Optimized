
//scl-9
// sda-8

void loopReadAngleAcceleration() {
  long timer = 0;
  mpu.update();

  if(millis() - timer > 1000){ // print data every second
    //using xaceleartion for measuring the speed
    Serial.print(F("ACCELERO  X: "));Serial.println(mpu.getAccX());
    //using y angle for measuring the tilting
    Serial.print(F("ANGLE     Y: "));Serial.println(mpu.getAngleY());
    timer = millis();
  }

}
