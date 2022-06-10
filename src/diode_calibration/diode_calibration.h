#include "Arduino.h"
#include <vector>
#include <algorithm>

// Resistor struct. Pins determine which pins should be on, value represetns the resistive value that is then reached.
struct Resistor {
    std::vector<uint8_t> pins;
    float value;
};

// Resistor comparator function for sorting in decreasing order.
bool comparator(Resistor const& a, Resistor const& b) {
  return a.value > b.value;
}

// Set available resistor values.
const Resistor resistors[] = {{{D12}, 660000}, {{D11}, 330000}, {{D10}, 100000}, {{D9}, 22000}};

// Set diode to finetune
const uint8_t diode = A0;


// Set a list of resistors.
void set_resistor(const std::vector<uint8_t> indices) {
  // First unset all resistors
  for(uint8_t i = 0; i < sizeof(resistors)/sizeof(Resistor); i++) {
    digitalWrite(resistors[i].pins.at(0), HIGH);
  }

  // Then set all resistors
  for (unsigned int j = 0; j < indices.size(); j++) {
    digitalWrite(indices.at(j), LOW);
  }
}

// Calculates the total resistive value of a set of resistors.
// 1/Rtot = 1/R1 + 1/R2 ...
float calculate_total_resistance_paralel(std::vector<float> values) {
  float sum = 0;
  for(unsigned int i = 0; i < values.size(); i++) {
    sum += 1/values.at(i);
  }
  return 1/sum;
}

float calculate_total_resistance_series(std::vector<float> values) {
  float sum = 0;
  for(unsigned int i = 0; i < values.size(); i++) {
    sum += values.at(i);
  }
  return sum;
}

std::vector<Resistor> createPowerSet(const Resistor* set, int size) {

  std::vector<Resistor> powerSet(size);

  // Fill powerset
  for(int counter = 0; counter < size; counter++) {
    std::vector<uint8_t> pin_numbers;
    std::vector<float> values;

    for(int j = 0; j < size; j++) {
      if(counter & (1<<j)) {
        values.push_back(set[j].value);
        pin_numbers.push_back(set[j].pins.at(0)); // Since these are single resistors the pin numbers are at postion 0.
      }       
    }
    powerSet[counter] = Resistor{pin_numbers, calculate_total_resistance_series(values)};
  }
  return powerSet;
}

// Parameters for diode calibration. 
const int window = 10;
const int delay_period = 10;

// TODO: remove recursion and use iteration
void calibrate_diode(uint8_t resistorIndex, std::vector<Resistor> powerSet, int powerSetSize) {

  // Check if index exists.
  if(resistorIndex >= powerSetSize) {
    digitalWrite(22, LOW);
    return;
  }

  set_resistor(powerSet[resistorIndex].pins);
  // allow the capacitors to lose charge?
  delay(500);

  int read_sum = 0;
  for (int i = 0; i < window; i++) {
    read_sum += analogRead(diode);
    delay(delay_period);
  }

  
  if ((read_sum / window) > 750) {
    // Reading too large, try next
    calibrate_diode(resistorIndex+1, powerSet, powerSetSize);
  } else if ((read_sum / window) < 325) {
    // Reading too small, revert to previous (even though that was too large). Check that previous resistorIndex exists
    if (resistorIndex != 0) {
      set_resistor(powerSet[resistorIndex-1].pins);
      digitalWrite(24, LOW);
    } else {
      digitalWrite(22, LOW);
    }
  } else {
    // Appropriate reading achieved
    digitalWrite(23, LOW);
  }



}

// Function that calibrates the diodes.
void calibrate_diode_setup() {
  // Allow the capacitor to charge up i guess
  set_resistor({resistors[0].pins.at(0), resistors[1].pins.at(0), resistors[2].pins.at(0), resistors[3].pins.at(0)});
  delay(100);


  // Create set of appropriate size
  const unsigned int size = pow(2, sizeof(resistors)/sizeof(Resistor));
  std::vector<Resistor> powerSet = createPowerSet(resistors, size);

  // Erase the empty set
  //powerSet.erase(powerSet.begin());

  // Sort the powerset on decreasing resistive values.
  std::sort(powerSet.begin(), powerSet.end(), &comparator);
  calibrate_diode(0, powerSet, size);
  //calibrate_diode(0, {Resistor(resistors[0]), Resistor(resistors[1]), Resistor(resistors[2]), Resistor(resistors[3])}, 4);
}