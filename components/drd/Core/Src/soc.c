#include "soc.h"
#include "main.h"
#include <math.h>
#include "CAN_comms.h"

volatile float g_total_pack_voltage_soc;
volatile float g_pack_current_soc;


/* DEFINES */
#define SOC     0
#define UC      1   

#define VOLTAGE_MAP_MIN 94
#define VOLTAGE_MAP_SIZE 40
#define SOC_MAX     (1.04f)
#define SOC_MIN     (0.00f)

// Maps integer voltage from 94V to 133V (inclusive) to float percentage
// See this thread https://ubcsolar26.monday.com/boards/7524367653/pulses/9144506936
static const float voltage_to_percent_map[VOLTAGE_MAP_SIZE] = {
    0.0000,   // 94 V
    0.0030,   // 95 V
    0.0070,   // 96 V
    0.0120,   // 97 V
    0.0160,   // 98 V
    0.0220,   // 99 V
    0.0270,   // 100 V
    0.0330,   // 101 V
    0.0400,   // 102 V
    0.0480,   // 103 V
    0.0560,   // 104 V
    0.0660,   // 105 V
    0.0780,   // 106 V
    0.0920,   // 107 V
    0.1100,   // 108 V
    0.1330,   // 109 V
    0.1640,   // 110 V
    0.2020,   // 111 V
    0.2400,   // 112 V
    0.2750,   // 113 V
    0.3080,   // 114 V
    0.3410,   // 115 V
    0.3740,   // 116 V
    0.4090,   // 117 V
    0.4470,   // 118 V
    0.4900,   // 119 V
    0.5380,   // 120 V
    0.5870,   // 121 V
    0.6340,   // 122 V
    0.6750,   // 123 V
    0.7110,   // 124 V
    0.7430,   // 125 V
    0.7740,   // 126 V
    0.8040,   // 127 V
    0.8350,   // 128 V
    0.8670,   // 129 V
    0.9040,   // 130 V
    0.9510,   // 131 V
    1.0050,   // 132 V
    1.0430    // 133 V
};

//-----------------------------
//--- Model & EKF Parameters -- From kalman_filter_example.py in STG's physics repo.
//-----------------------------

// Battery capacity (Coulombs)
static const float Q_total   = 145000.0;  

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
    SOC_MAX,   // initial SOC = 104%
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
// U_oc_coeffs = [  2964.5351135  -11872.69601585  19108.58449175 -15842.2151069
// 7225.59336211  -1800.93776157    254.87051039     94.15508448]
static float get_Uoc(float x) {
    return
            94.15508448
        + 254.87051039   * x
        + (-1800.93776157)* x * x
        + 7225.59336211   * x * x * x
        + (-15842.2151069)* x * x * x * x
        + 19108.58449175  * x * x * x * x * x
        + (-11872.69601585)* x * x * x * x * x * x
        + 2964.5351135    * x * x * x * x * x * x * x;
}

// R_0_coeffs = [-1.54613590e+02,  5.52593795e+02, -7.94776446e+02,  5.92975205e+02,
//               -2.47297657e+02,  5.80013511e+01, -7.19686618e+00,  4.92268729e-01]
static float get_R0(float x) {
    return
         0.492268729
      + (-7.19686618)   * x
      + 58.0013511      * x * x
      + (-247.297657)   * x * x * x
      + 592.975205      * x * x * x * x
      + (-794.776446)   * x * x * x * x * x
      + 552.593795      * x * x * x * x * x * x
      + (-154.61359)    * x * x * x * x * x * x * x;
}

// R_P_coeffs = [ 1.08804893e+02, -3.95266369e+02,  5.53572478e+02, -3.69442515e+02,
//                1.13424395e+02, -9.44642125e+00, -1.96878597e+00,  3.64862429e-01]
static float get_Rp(float x) {
    return
         0.364862429
      + (-1.96878597)   * x
      + (-9.44642125)   * x * x
      + 113.424395      * x * x * x
      + (-369.442515)   * x * x * x * x
      + 553.572478      * x * x * x * x * x
      + (-395.266369)   * x * x * x * x * x * x
      + 108.804893      * x * x * x * x * x * x * x;
}

// C_P_coeffs = [ 2.59785258e+06, -7.85129803e+06,  9.21590368e+06, -5.19565604e+06,
//                1.38788088e+06, -1.46480151e+05,  6.37015318e+03,  2.60647678e+02]
static float get_Cp(float x) {
    return
         260.647678
      + 6370.15318      * x
      + (-146480.151)   * x * x
      + 1387880.88      * x * x * x
      + (-5195656.04)   * x * x * x * x
      + 9215903.68      * x * x * x * x * x
      + (-7851298.03)   * x * x * x * x * x * x
      + 2597852.58      * x * x * x * x * x * x * x;
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
static void update_filter(float measured_V, float current) {
    // low-pass filter current
    filtered_I = alpha * filtered_I + (1.0f - alpha) * current;

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

static float get_soc_from_voltage(int voltage) {
    if (voltage < VOLTAGE_MAP_MIN)
    {
        return SOC_MIN;
    } 
    else if (voltage >= (VOLTAGE_MAP_MIN + VOLTAGE_MAP_SIZE)) 
    {
        return SOC_MAX;
    }
    else
    {
        return voltage_to_percent_map[voltage - VOLTAGE_MAP_MIN];
    }
}


//-----------------------------
//--- Public API -------------
//-----------------------------

#ifdef DEBUG
static uint32_t soc_time_start;
static uint32_t soc_time_diff;
#endif // DEBUG

void SOC_predict_then_update(float g_total_pack_voltage_soc, float g_pack_current_soc, float time_step)
{
    // When debugging we can check the duration of this function to measure performance.
    #ifdef DEBUG
        soc_time_start = HAL_GetTick();
    #endif // DEBUG

    predict_state(g_pack_current_soc, time_step);
    update_filter(g_total_pack_voltage_soc, g_pack_current_soc);

    #ifdef DEBUG
        soc_time_diff = HAL_GetTick() - soc_time_start;
        UNUSED(soc_time_diff);
    #endif // DEBUG
}

float SOC_get_soc()
{
    return (state[SOC]);
}

float SOC_get_voltage()
{
    return last_predicted_V;
}

float SOC_get_Uc()
{
    return (state[UC]);
}

void SOC_init_soc(int voltage)
{
    state[SOC] = get_soc_from_voltage(voltage);
}

void SOC_send_can()
{
	CAN_comms_Tx_msg_t soc_can_msg;
    soc_can_msg
}
