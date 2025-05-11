#include "soc.h"
#include <math.h>
#include <assert.h>
#include "main.h"

float g_total_pack_voltage_soc;
float g_pack_current_soc;

/* DEFINES */
#define SOC     0
#define UC      1   

//-----------------------------
//--- Model & EKF Parameters --
//-----------------------------

// Battery capacity (Coulombs)
static const float Q_total   = 3600.0f;  // e.g., 1 Ah

// Low-pass factor for current
static const float alpha     = 0.9f;

// Process noise covariance Q (2×2)
static const float Q_proc[2][2] = {
    {1e-7f,    0.0f},
    {   0.0f, 1e-5f}
};

// State covariance P (2×2), initialized at compile-time
static float P[2][2] = {
    {1e-3f,    0.0f},
    {   0.0f, 1e-3f}
};

// Measurement noise (scalar)
static const float R_meas = 1e-2f;

//-----------------------------
//--- Static EKF State -------
//-----------------------------

// State vector state = [SOC; Uc]
static float state[2] = {
    1.0f,   // initial SOC = 100%
    0.0f    // initial Uc = 0 V
};

// Low-pass filtered current
static float filtered_I = 0.0f;

// Last predicted terminal voltage
static float last_predicted_V = 0.0f;

//-----------------------------
//--- Battery Model Curves ----
//-----------------------------
// Replace with your real data if needed
static float get_Uoc(float soc) {
    return 3.0f + 1.2f * soc;  // 3.0 V at 0%, 4.2 V at 100%
}
static float get_R0(float soc) { return 0.05f - 0.02f * soc; }
static float get_Rp(float soc) { (void)soc; return 0.1f; }
static float get_Cp(float soc) { (void)soc; return 2000.0f; }

//-----------------------------
//--- Static Helpers ---------
//-----------------------------

// 2×2 matrix multiply: C = A·B
static void mat2_mul(const float A[2][2],
                     const float B[2][2],
                     float       C[2][2])
{
    C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0];
    C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1];
    C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0];
    C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1];
}

// C = A + B
static void mat2_add(const float A[2][2],
                     const float B[2][2],
                     float       C[2][2])
{
    C[0][0] = A[0][0] + B[0][0];
    C[0][1] = A[0][1] + B[0][1];
    C[1][0] = A[1][0] + B[1][0];
    C[1][1] = A[1][1] + B[1][1];
}

// Central‐difference derivative of f at v
static float central_diff(float (*f)(float), float v) {
    const float h = 1e-6f;
    float v0 = v - h, v1 = v + h;
    // if (v0 < 0.0f) v0 = 0.0f;
    // if (v1 > 1.0f) v1 = 1.0f;
    return (f(v1) - f(v0)) / (2.0f * h);
}

//-----------------------------
//--- EKF Steps --------------
//-----------------------------

// Predict only (updates state[] and P[][])
static void predict_state(float current, float dt) {
    // B matrix
    float B0  = -dt / Q_total;
    float tau = get_Rp(state[SOC]) * get_Cp(state[SOC]);
    float B1  = get_Rp(state[SOC]) * (1.0f - expf(-dt / tau));

    // F = [[1,0];[0, exp(-dt/tau)]]
    float F[2][2] = {
        {1.0f,                 0.0f},
        {0.0f, expf(-dt / tau)}
    };

    // state = F·x + B·u
    float x0 = F[0][0]*state[SOC] + F[0][1]*state[UC] + B0*current;
    float x1 = F[1][0]*state[SOC] + F[1][1]*state[UC] + B1*current;
    state[SOC] = x0;  state[UC] = x1;

    // P = F·P·Fᵀ + Q_proc
    float Ft[2][2] = { {F[0][0], F[1][0]},
                       {F[0][1], F[1][1]} };
    float FP[2][2];
    mat2_mul(F,   P,   FP);
    mat2_mul(FP, Ft,  P);
    mat2_add(P,   Q_proc, P);
}

// Update only (refines state[] and P[][])
static void update_filter(float measured_V) {
    // low-pass filter current
    filtered_I = alpha * filtered_I + (1.0f - alpha) * filtered_I;

    // H = [dUoc/dSOC - dR0/dSOC·I,  -1]
    float dU = central_diff(get_Uoc, fmin(1.0f, state[SOC]));
    float dR = central_diff(get_R0,  fmin(1.0f, state[SOC]));
    float H0 = dU - dR * filtered_I;
    float H1 = -1.0f;
    
    // S = H·P·Hᵀ + R
    float PHt0 = P[0][0]*H0 + P[0][1]*H1;      
    float PHt1 = P[1][0]*H0 + P[1][1]*H1;
    float S    = H0*PHt0 + H1*PHt1 + R_meas;
    
    // K = P·Hᵀ / S
    float K0 = PHt0 / S;
    float K1 = PHt1 / S;
    
    // Measurement Function hx = Uoc - Uc - R0·I
    float Uoc = get_Uoc(state[SOC]);
    float R0  = get_R0(state[SOC]);
    float hx   = Uoc - state[UC] - R0 * filtered_I;
    last_predicted_V = hx;

    // Update State
    float y = measured_V - hx;
    state[SOC] += K0 * y;
    state[UC] += K1 * y;

    // P = (I-KH)P(I-KH)ᵀ + KRKᵀ
    float IKH[2][2] = {
        {1.0f - K0*H0,   -K0*H1},
        {  -K1*H0,    1.0f - K1*H1}
    };
    float IKHT[2][2] = {
        {IKH[0][0],   IKH[1][0]},
        {IKH[0][1],   IKH[1][1]}
    };
    float tmp1[2][2];
    mat2_mul(IKH, P, tmp1);
    mat2_mul(tmp1, IKHT, P);        // TODO: Check this line to see if IKHT is needed or just regular IKH
    // add K*R*Kᵀ. KRKT is a 2x2 because K = 2 x 1.
    P[0][0] += K0*K0*R_meas;
    P[0][1] += K0*K1*R_meas;
    P[1][0] += K1*K0*R_meas;
    P[1][1] += K1*K1*R_meas;

    // clamp SOC
    if (state[SOC] < 0.0f)  state[SOC] = 0.0f;
    if (state[SOC] > 1.1f)  state[SOC] = 1.1f;
}

//-----------------------------
//--- Public API -------------
//-----------------------------

void SOC_predict_then_update(float g_total_pack_voltage_soc, float g_pack_current_soc, float time_step)
{
    // When debugging we can check the duration of this function to measure performance.
    #ifdef DEBUG
        uint32_t soc_time_start = HAL_GetTick();
    #endif // DEBUG

    predict_state(g_pack_current_soc, time_step);
    update_filter(g_total_pack_voltage_soc);

    #ifdef DEBUG
        uint32_t soc_time_diff = HAL_GetTick() - soc_time_start;
    #endif // DEBUG
}

uint8_t SOC_get_soc()
{
    return (uint8_t)(state[SOC] * 100);
}
