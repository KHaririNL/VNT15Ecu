#include <Arduino.h>
#include <ResponsiveAnalogRead.h>
#include <SoftwareSerial.h>
#include <PID_v1.h>

/*
       N75 to vanes, according to vanbcguy's data
       =======================
       204 = 80% N75 duty cycle = vanes fully closed (100%)
       70  = 23% N75 duty cycle = vanes fully opened (0%)
       
       According to "https://dieselnet.com/tech/air_turbo_vgt.php" ,
       "peak efficiency of a variable geometry turbine occurs at about 60% nozzle opening" so;
       70 + (204-70) * 0.6 = 150
       150 / 255 = 0.589 <-- round up to 60%

  ---- 60% should handle the cruise situation too;
       https://forums.tdiclub.com/index.php?threads/vnt-is-killing-your-fuel-economy.508304/
        Cruise at 70-90% PWM: 6-10psi
        Cruise at 30-45% PWM: 1-2psi

        Predicted boost while cruising at 60%: 4psi = 0.28bar (good)
*/
#define VNT_SETPOINT_POWER 0.60f
#define BOOST_MAX_SPEC_KPA 750.0  //0.75 bar
#define BOOST_DESIRED_SPEC_KPA 400
#define BOOST_IDLE_THRESHOLD 250  //No boost situation, when below this value
#define MASTER_BOOST_PWM_PIN 5
#define BOOST_LVL_PIN A0

#define EXEC_UPDATE 30
#define SENSOR_FAST_UPDATE 15
#define N75_FREQ 24  //24 clicks per second max

//PID SETTINGS
//Ziegler–Nichols method; https://en.wikipedia.org/wiki/Proportional%E2%80%93integral%E2%80%93derivative_controller#Ziegler%E2%80%93Nichols_method
//Basically keep increasing Kp until the PID outpu Ku = half Kp.
double Ku = 0.40;
double Tu = 0.5;

double Kp = 0.6 * Ku;
double Ki = 1.2 * Ku / Tu;
double Kd = 3.0 * Ku * Tu / 40.0;

//Global vars
double boostkPa = 0;
double pidOutput = 0.0;
double desiredKpa = BOOST_DESIRED_SPEC_KPA;
float vntOpening = 0.0f;

ResponsiveAnalogRead map_read(BOOST_LVL_PIN, false);  // no sleep on the MAP read; need the resolution
PID vntPid(&boostkPa, &pidOutput, &desiredKpa, Kp, Ki, Kd, P_ON_E, DIRECT);

/************<AVGSTRUCTS****************/
#define AVG_MAX 15
struct avgStruct {
  unsigned char pos;
  unsigned char size = AVG_MAX;
  volatile unsigned int avgData[AVG_MAX];
};

volatile int mapInput;
double mapCorrected;

avgStruct mapAvg;
int getFilteredAverage(struct avgStruct* a) {
  long int avgAll = 0;

  for (int i = 0; i < a->size; i++) {
    avgAll += a->avgData[i];
  }

  avgAll = (int)(avgAll / a->size);
  return avgAll;
}
/***********AVGSTRUCTS/>****************/

void setup(void) {
  Serial.begin(115200);  //Initialize Serial Port

  pinMode(MASTER_BOOST_PWM_PIN, OUTPUT);
  vntPid.SetSampleTime(EXEC_UPDATE);
  analogWriteFrequency(MASTER_BOOST_PWM_PIN, N75_FREQ);
}

unsigned long sensorFastUpdate = 0;
unsigned long execLoop = 0;

void loop(void) {
  //Reads the MAP data
  if ((millis() - sensorFastUpdate) >= SENSOR_FAST_UPDATE) {
    mapAvg.pos++;
    if (mapAvg.pos >= mapAvg.size)
      mapAvg.pos = 0;

    map_read.update();
    mapAvg.avgData[mapAvg.pos] = map_read.getValue();
    mapInput = getFilteredAverage(&mapAvg);
  }

  //Do execution which sets the boost
  if ((millis() - execLoop) >= EXEC_UPDATE) {
    double voltage_Boost = mapInput * (5.0 / 1024.0);
    double boostValue = 0.66 * voltage_Boost - 1.06;
    double boostValueKpa = boostValue * 1000.0;
    bool hasUsedPid = false;

    if (boostValueKpa >= BOOST_MAX_SPEC_KPA || boostValue < -0.7 || boostValue >= 1.5) {
      digitalWrite(MASTER_BOOST_PWM_PIN, LOW);
    } else {
      /*
      We need to fully actuate in +- 2.5seconds, from 0 to 255
      This being called +- 20 times per second on average. 20*2.5 = 50 ticks
      So delta per tick is 255/50 = +- 5, make it 8 => 8 / 255 = 0.03
    */
      float vntOpeningDelta = 0.03f;

      if (boostkPa <= BOOST_IDLE_THRESHOLD) {
        vntOpening += vntOpeningDelta;  //within 3 seconds close it
      } else if (boostkPa <= BOOST_MAX_SPEC_KPA) {
        //do PID here
        hasUsedPid = true;
        vntPid.SetMode(AUTOMATIC);
        vntPid.Compute();

        vntOpening = pidOutput / 255.0f;
      } else {
        //Default, we shouldnt be getting here
        vntOpening = 0.0f;
      }

      vntOpening = min(VNT_SETPOINT_POWER, vntOpening);
      vntOpening = max(0.0f, vntOpening);

      float vntSignal = vntOpening * 255.0f;
      int iVNTSignal = (int)vntSignal;

      iVNTSignal = min(255, iVNTSignal);
      iVNTSignal = max(0, iVNTSignal);
      analogWrite(MASTER_BOOST_PWM_PIN, iVNTSignal);
    }


    if (!hasUsedPid) {
      vntPid.SetMode(MANUAL);
    }
  }
}
