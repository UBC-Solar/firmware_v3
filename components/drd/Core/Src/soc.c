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
//--- Model & EKF Parameters -- From kalman_filter_example.py in STG's physics repo.
//-----------------------------

// Battery capacity (Coulombs)
static const float Q_total   = 151000.0;  

// Low-pass factor for current
static const float alpha     = 0.9f;

// Process noise covariance Q (2×2)
static const float Q_proc[2][2] = {
    {1e-10f * 0.1,    0.0f},
    {   0.0f, 1e-6f * 0.1}
};

// State covariance P (2×2), initialized at compile-time
static float P[2][2] = {
    {1e-2f * 0.5,    0.0f},
    {   0.0f, 1e-1f}
};

// Measurement noise (scalar)
static const float R_meas = 1e0f * 0.5;

//-----------------------------
//--- Static EKF State -------
//-----------------------------

// State vector state = [SOC; Uc]
static float state[2] = {
    1.04f,   // initial SOC = 104%
    0.0f    // initial Uc = 0 V
};

// Low-pass filtered current
static float filtered_I = 0.0f;

// Last predicted terminal voltage
static float last_predicted_V = 0.0f;

//-----------------------------
//--- Battery Model Curves ----
//-----------------------------
// Coefficients based on fitting in STG's Physics repo battery_config.py
// U_oc_coeffs = [  2964.52101072, -11872.64866062,  19108.52190655, -15842.17383716,
//                   7225.57921631,  -1800.93540695,    254.87035949,     94.15508582 ]
static float get_Uoc(float x) {
    return
        2964.52101072f
      + (-11872.64866062f) * x
      + ( 19108.52190655f) * x * x
      + (-15842.17383716f) * x * x * x
      + (  7225.57921631f) * x * x * x * x
      + (-1800.93540695f) * x * x * x * x * x
      + (   254.87035949f) * x * x * x * x * x * x
      + (    94.15508582f) * x * x * x * x * x * x * x;
}

// R_0_coeffs = [-1.54613590e+02,  5.52593795e+02, -7.94776446e+02,  5.92975205e+02,
//               -2.47297657e+02,  5.80013511e+01, -7.19686618e+00,  4.92268729e-01]
static float get_R0(float x) {
    return
        -154.613590f
      +   552.593795f  * x
      +  -794.776446f  * x * x
      +   592.975205f  * x * x * x
      +  -247.297657f  * x * x * x * x
      +    58.001351f  * x * x * x * x * x
      +    -7.196866f  * x * x * x * x * x * x
      +     0.4922687f * x * x * x * x * x * x * x;
}

// R_P_coeffs = [ 1.08817913e+02, -3.95312179e+02,  5.53636103e+02, -3.69486684e+02,
//                1.13440342e+02, -9.44922096e+00, -1.96859647e+00,  3.64860692e-01]
static float get_Rp(float x) {
    return
         108.817913f
      +  (-395.312179f)  * x
      +   553.636103f   * x * x
      +  (-369.486684f)  * x * x * x
      +   113.440342f   * x * x * x * x
      +    -9.44922096f * x * x * x * x * x
      +    -1.96859647f * x * x * x * x * x * x
      +     0.364860692f* x * x * x * x * x * x * x;
}

// C_P_coeffs = [ 2.59786749e+06, -7.85135009e+06,  9.21597530e+06, -5.19570513e+06,
//                1.38789832e+06, -1.46483144e+05,  6.37034956e+03,  2.60645952e+02]
static float get_Cp(float x) {
    return
        2597867.49f
      + (-7851350.09f)   * x
      +  9215975.30f     * x * x
      + (-5195705.13f)   * x * x * x
      +  1387898.32f     * x * x * x * x
      +  (-146483.144f)  * x * x * x * x * x
      +    6370.34956f   * x * x * x * x * x * x
      +     260.645952f  * x * x * x * x * x * x * x;
}

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
    float Rp = get_Rp(state[SOC]);
    float tau = Rp * get_Cp(state[SOC]);
    float B1  = Rp * (1.0f - expf(-dt / tau));

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
