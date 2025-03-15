#define STEP_X 2
#define DIR_X  5
#define STEP_Y 3
#define DIR_Y  6
#define STEP_Z 4
#define DIR_Z  7

// Motor parameters
const int STEPS_PER_REV = 200;  // Steps per revolution for KV4234
long currentPosition[3] = {0, 0, 0};  // Current position of each motor
float currentSpeed = 1000;  // Initial speed (microseconds between steps)
float maxSpeed = 500;       // Maximum speed (minimum microseconds between steps)
float acceleration = 50;    // Acceleration rate

void setup() {
    // Setup pins
    pinMode(STEP_X, OUTPUT);
    pinMode(DIR_X, OUTPUT);
    pinMode(STEP_Y, OUTPUT);
    pinMode(DIR_Y, OUTPUT);
    pinMode(STEP_Z, OUTPUT);
    pinMode(DIR_Z, OUTPUT);
    
    Serial.begin(115200);
    Serial.setTimeout(5);
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        parseCommand(command);
    }
}

void parseCommand(String command) {
    char cmd = command.charAt(0);
    int value = 0;
    if (command.length() > 1) {
        value = command.substring(1).toInt();
    }
    
    switch(cmd) {
        case 'F': // Forward
            moveAllMotors(value, true);
            break;
        case 'B': // Backward
            moveAllMotors(value, false);
            break;
        case 'S': // Set Speed
            setSpeed(value);
            break;
        case 'P': // Get Position
            reportPosition();
            break;
        case 'E': // Emergency Stop
            emergencyStop();
            break;
    }
}

void setSpeed(int speed) {
    if (speed >= 500 && speed <= 2000) {
        currentSpeed = speed;
    }
}

void moveAllMotors(int steps, bool direction) {
    // Set direction for all motors
    digitalWrite(DIR_X, direction ? HIGH : LOW);
    digitalWrite(DIR_Y, direction ? HIGH : LOW);
    digitalWrite(DIR_Z, direction ? HIGH : LOW);
    
    float currentDelay = currentSpeed;
    
    // Move with acceleration and deceleration
    for (int i = 0; i < steps; i++) {
        // Acceleration phase
        if (i < steps/4) {
            currentDelay = max(maxSpeed, currentDelay - acceleration);
        }
        // Deceleration phase
        else if (i > (steps * 3/4)) {
            currentDelay = min(currentSpeed, currentDelay + acceleration);
        }
        
        // Step all motors simultaneously
        digitalWrite(STEP_X, HIGH);
        digitalWrite(STEP_Y, HIGH);
        digitalWrite(STEP_Z, HIGH);
        delayMicroseconds(currentDelay);
        digitalWrite(STEP_X, LOW);
        digitalWrite(STEP_Y, LOW);
        digitalWrite(STEP_Z, LOW);
        delayMicroseconds(currentDelay);
        
        // Update positions
        if (direction) {
            currentPosition[0]++;
            currentPosition[1]++;
            currentPosition[2]++;
        } else {
            currentPosition[0]--;
            currentPosition[1]--;
            currentPosition[2]--;
        }
    }
}

void reportPosition() {
    Serial.print("POS:");
    Serial.print(currentPosition[0]);
    Serial.print(",");
    Serial.print(currentPosition[1]);
    Serial.print(",");
    Serial.println(currentPosition[2]);
}

void emergencyStop() {
    // Immediately stop all movement
    digitalWrite(STEP_X, LOW);
    digitalWrite(STEP_Y, LOW);
    digitalWrite(STEP_Z, LOW);
    Serial.println("STOPPED");
} 