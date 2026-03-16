#include "timer.h"
#include <windows.h>

static LARGE_INTEGER g_perfFreq;
static LARGE_INTEGER g_lastFrameCounter;
static LARGE_INTEGER g_currentFrameCounter;

static float g_deltaSeconds = 0.0f;
static float g_deltaMs = 0.0f;
static float g_smoothedFrameMs = 16.0f;
static float g_renderFPS = 0.0f;
static float g_simUPS = 0.0f;

static double g_totalSeconds = 0.0;
static double g_statsAccumSeconds = 0.0;
static unsigned int g_frameCount = 0;
static unsigned int g_frameRenderAccum = 0;
static unsigned int g_simStepsAccum = 0;
static unsigned int g_frameSimSteps = 0;

static double Timer_CounterDeltaSeconds(const LARGE_INTEGER* a, const LARGE_INTEGER* b)
{
    return (double)(a->QuadPart - b->QuadPart) / (double)g_perfFreq.QuadPart;
}

void Timer_Initialize()
{
    QueryPerformanceFrequency(&g_perfFreq);
    QueryPerformanceCounter(&g_lastFrameCounter);
    g_currentFrameCounter = g_lastFrameCounter;

    g_deltaSeconds = 0.0f;
    g_deltaMs = 0.0f;
    g_smoothedFrameMs = 16.0f;
    g_renderFPS = 0.0f;
    g_simUPS = 0.0f;
    g_totalSeconds = 0.0;
    g_statsAccumSeconds = 0.0;
    g_frameCount = 0;
    g_frameRenderAccum = 0;
    g_simStepsAccum = 0;
    g_frameSimSteps = 0;
}

void Timer_BeginFrame()
{
    double deltaSeconds;

    QueryPerformanceCounter(&g_currentFrameCounter);
    deltaSeconds = Timer_CounterDeltaSeconds(&g_currentFrameCounter, &g_lastFrameCounter);
    g_lastFrameCounter = g_currentFrameCounter;

    if (deltaSeconds < 0.0)
        deltaSeconds = 0.0;
    if (deltaSeconds > 0.25)
        deltaSeconds = 0.25;

    g_deltaSeconds = (float)deltaSeconds;
    g_deltaMs = (float)(deltaSeconds * 1000.0);
    g_smoothedFrameMs = (g_smoothedFrameMs * 0.90f) + (g_deltaMs * 0.10f);
    g_totalSeconds += deltaSeconds;
    g_frameSimSteps = 0;
}

void Timer_EndFrame()
{
    g_frameCount++;
    g_frameRenderAccum++;
    g_statsAccumSeconds += (double)g_deltaSeconds;

    if (g_statsAccumSeconds >= 0.25)
    {
        g_renderFPS = (float)((double)g_frameRenderAccum / g_statsAccumSeconds);
        g_simUPS = (float)((double)g_simStepsAccum / g_statsAccumSeconds);
        g_frameRenderAccum = 0;
        g_simStepsAccum = 0;
        g_statsAccumSeconds = 0.0;
    }
}

void Timer_AddSimulationSteps(unsigned int steps)
{
    g_frameSimSteps += steps;
    g_simStepsAccum += steps;
}

float Timer_GetDeltaSeconds() { return g_deltaSeconds; }
float Timer_GetDeltaMilliseconds() { return g_deltaMs; }
float Timer_GetSmoothedFrameMilliseconds() { return g_smoothedFrameMs; }
float Timer_GetRenderFPS() { return g_renderFPS; }
float Timer_GetSimUPS() { return g_simUPS; }
double Timer_GetTotalSeconds() { return g_totalSeconds; }
unsigned int Timer_GetFrameCount() { return g_frameCount; }
unsigned int Timer_GetFrameSimulationSteps() { return g_frameSimSteps; }
