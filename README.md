# VNT15Ecu
Minimalistic VNT15 controller using only a boost sensor

It does so using the following logic;
- If boost is below 0.2 bar / 3 psi: VNT is fully closed. Max boost is generated
- Above 0.2 bar / 3 psi: use PID to control the N75 valve

Basically PID is a way for a computer to control automatically how open/closed the VNT15 needs to be to achieve the desired boost. A good explanaition can be found here; https://www.youtube.com/watch?v=wkfEZmsQqiA

# Desired Boost
This part is specific to the VW 1.6D engine (CR). 

**Maximum Exhaust Gas Temperature (EGT) for NA Engine**
This is a Naturally Aspirated (NA) engine, and thus not originally built to operate under boost. SAE paper 820441 describes the differences;
- [For the turbo engine] the maximum inlet temperature is approx. 820C (1500F)
- Because valve temperatures being approx. 100 - 150C higher better valve head material is used
- Cooling of piston bottom; special oil cooling lowers critical piston temperature by approx. 30C

So we can calculate our max exhaust gas temperature (EGT) for the NA engine: 820 - 150 - 30 = **640C / 1200F**

**Maximum Boost for NA engine**

