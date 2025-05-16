const int buttonPin = 32;

const int xPin = 34; //VRX pin
const int yPin = 35; //VRY pin

int buttonState = 0;
int xValue = 0;
int yValue = 0;


void setup() {
  Serial.begin(115200);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());
  // Set the button pin as input with internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  Serial.print("loop() running on core ");
  Serial.println(xPortGetCoreID());
  // Read the state of the button (it will be LOW when pressed, High when not pressed)
  buttonState = digitalRead(buttonPin);

  xValue = analogRead(xPin); // Read the X-axis value
  yValue = analogRead(yPin); // Read the Y-axis value

  Serial.print("vx : ");
  Serial.println(xValue);

  Serial.print("\t");

  Serial.print("vy : ");
  Serial.println(yValue);

  Serial.println(buttonState);

  delay(100);

}
