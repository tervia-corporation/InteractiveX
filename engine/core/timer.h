#ifndef IX_CORE_TIMER_H
#define IX_CORE_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void Timer_Initialize();
void Timer_BeginFrame();
void Timer_EndFrame();
void Timer_AddSimulationSteps(unsigned int steps);

float Timer_GetDeltaSeconds();
float Timer_GetDeltaMilliseconds();
float Timer_GetSmoothedFrameMilliseconds();
float Timer_GetRenderFPS();
float Timer_GetSimUPS();
double Timer_GetTotalSeconds();
unsigned int Timer_GetFrameCount();
unsigned int Timer_GetFrameSimulationSteps();

#ifdef __cplusplus
}
#endif

#endif
