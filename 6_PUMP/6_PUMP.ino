#define PUMP_COUNT 6


const char PUMP[PUMP_COUNT] = {'1', '2', '3', '4', '5', '6'};
const int PUMP_IN1[] = {22, 24, 26, 28, 30, 32}; // Green 
const int PUMP_IN2[] = {23, 25, 27, 29, 31, 33}; // Yellow
const int PUMP_PWM[] = {2, 3, 4, 5, 6, 7}; // White


unsigned long currentTime = 0;
unsigned long lastTime = 0;
unsigned long deltaTime = 0;

const int FORWARD = 'F';
const int REVERSE = 'R';
const int HALT = 'H';

unsigned long durations[PUMP_COUNT] = {0}; // Initialize durations array

void setup() {

    setupTimekeeping();
    for (int i = 0; i < PUMP_COUNT; i++) {
        pinMode(PUMP_IN1[i], OUTPUT);
        pinMode(PUMP_IN2[i], OUTPUT);
        pinMode(PUMP_PWM[i], OUTPUT);
    }

    Serial.begin(115200);
}

void setupTimekeeping() {
    currentTime = millis();
    lastTime = currentTime;
}

void loop() {
    currentTime = millis();
    deltaTime = currentTime - lastTime;

    reduceDurations(deltaTime);
    handleInput();

    lastTime = currentTime; // Update lastTime after handling input and reducing durations
}

void handleInput() {
    char *array[4]; // Assuming there are at most 4 tokens
    int count = 0; // Counter for the number of tokens

    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        char commandBuffer[command.length() + 1]; // Create a char array buffer for strtok
        command.toCharArray(commandBuffer, sizeof(commandBuffer)); // Convert String to char array

        char *token;
        const char *delimiter = " ";
        token = strtok(commandBuffer, delimiter);

        while (token != NULL && count < 4) { // Ensure array index doesn't exceed 4
            array[count++] = token; // Store token in the array and increment count
            token = strtok(NULL, delimiter);
        }
        //Asign which part of the input command to be read
        String pump = array[0];
        String dir = array[1];
        String durationString = array[2];
        String speedString = array[3];

        // Convert String to char array
        char pumpChar = pump.charAt(0);
        char dirChar = dir.charAt(0);

        // Convert duration and speed from String to int
        int duration = extractDuration(durationString.c_str());
        int speed = extractSpeed(speedString.c_str());

         // Start or update duration only if it's greater than 0
        if (speed >= 0 && speed <= 255) {
            turn(pumpChar, dirChar, speed);                
            if (duration > 0) {
                startDuration(pumpChar, duration); 
            } else {
                Serial.println("Error: Speed out of range (0 - 255)");
            }
        }
        //Print input double check
        //LCD Crystal add later
        Serial.println(pump);
        Serial.println(dir);
        Serial.println(duration);
        Serial.println(speed);
    }
}

// Extract the duration from the string
int extractDuration(const char* str) {
    int duration = 0;
    for (int i = 0; i < 5 && isDigit(str[i]); i++) {
        duration = duration * 10 + (str[i] - '0');
    }
    return duration;
}

    // Extract the speed from the string
int extractSpeed(const char* str) {
    int speed = atoi(str);
    return speed;
}

// Calculate the index based on the pump number
void startDuration(char pump, int duration) {
    int pumpIndex = pump - '1'; 
    durations[pumpIndex] = duration;
}

// Stop the pump when duration ends
void reduceDurations(unsigned long deltaTime) {
    for (int i = 0; i < PUMP_COUNT; i++) {
        if (durations[i] != 0) {
            durations[i] = max(0, durations[i] - deltaTime);
            if (durations[i] == 0) {
                turn(PUMP[i], HALT, 0); 
            }
        }
    }
}

void turn(char pump, char dir, int speed) {
    Serial.println("Setting speed!");
    turnDirection(pump, dir);
    // Set the speed using PWM (analogWrite)
    int index = pump - '1'; // Calculate the index based on the pump number
    if (index >= 0 && index < PUMP_COUNT) { // Ensure index is within bounds
        int pin = PUMP_PWM[index];
        analogWrite(pin, speed);
        Serial.print("Pump ");
        Serial.print(pump);
        Serial.print(" set to ");
        Serial.println(speed);
    } else {
        Serial.println("Invalid pump index!");
    }
}

//Case for pump direction and stop
void turnDirection(char pump, char dir) {
    int out1;
    int out2;
    switch (dir) {
        case HALT:
            out1 = LOW;
            out2 = LOW;
            break;
        case FORWARD:
            out1 = HIGH;
            out2 = LOW;
            break;
        case REVERSE:
            out1 = LOW;
            out2 = HIGH;
            break;
        default:
            return;
    }

    // Calculate the index based on the pump number
    int index = pump - '1';
    if (index >= 0 && index < PUMP_COUNT) { // Ensure index is within bounds
        digitalWrite(PUMP_IN1[index], out1);
        digitalWrite(PUMP_IN2[index], out2);
    } else {
        Serial.println("Invalid pump index!");
    }
}
