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
I've cross-referenced this value and it seems there's a consensus at forums (vwvortex.net/vwdiesel.net) that 1200F is indeed a good max EGT for this engine.

**Theoretical Safe Boost for NA engine**

The same paper also describes the boost / EGT curve. 
![egt curve](https://raw.githubusercontent.com/KHaririNL/VNT15Ecu/main/images/MaxSafeBoost.png)

The horizontal lines annotated by 400, 500, 600, .. are temperatures in C. It means that if we run the engine at 1 bar boost (14psi) at 2200rpm, we expect an EGT at 700C / 1300F.
I've added also our 640C/1200F in this graph (the purple line). Meaning we can theoretically run **0.8bar/12psi peak** (at 2000rpm) safely.

**Practical Safe Boost**

Considering the 0.8bar/12psi safe limit we found previously, a safety margin needs to be considered due to;
- The paper considers a brand new engine (mine is over 40 years old)
- The turbo used in the paper is a T3 which is a lot bigger than the VNT15, meaning less flow restrictions, meaning lower EGT
- The engine in the paper has a specific fuel pump (with LDA) that controls fueling according to the boost generated. The amount of air relative to fuel (air fuel ratio, AFR) has a large impact on EGT in diesels.

Therefore I run the engine at half the theorized pressure (0.4bar/8psi), with the fuel pump adjusted so it doesn't smoke.
