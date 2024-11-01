//trig-17
//echopin-18

void loopReadDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  // Reading the echo pulse duration
  long duration = pulseIn(echoPin, HIGH);

  // Calculating the distance in centimeters
  long distance_cm = duration * 0.034 / 2;  // Speed of sound is approximately 34,000 cm/s (divided by 2 for one-way distance)
  if (distance_cm < 400) {
    // Print the distance to the Serial monitor
    Serial.print("Distance: ");
    Serial.print(distance_cm);
    Serial.println(" cm");
    Serial.println("");
  }

  delay(200);  // Adjust delay if needed
}
