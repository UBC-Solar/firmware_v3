#include "soc.h"
#include <math.h>
#include <assert.h>

float g_total_pack_voltage_soc;
float g_pack_current_soc;

/*--- Type Definitions ---*/

// 1×2 state vector: [SOC, Uc]
typedef struct {
    float soc;
    float uc;
} State;

typedef struct {
    float x[2];        // state estimate [SOC; Uc]
    float P[2][2];     // state covariance
    float Q[2][2];     // process noise covariance
    float R;           // measurement noise covariance (1×1)
    float F[2][2];     // state transition Jacobian
    float B[2];        // control matrix
    float y;           // residual
    float K[2];        // Kalman gain (2×1)
    float S;           // innovation covariance (1×1)
} EKF;

// Configuration inputs (model + noise parameters)
typedef float (*ModelFunc)(float);
typedef struct {
    float Q_total;
    ModelFunc get_Uoc;
    ModelFunc get_R0;
    ModelFunc get_Rp;
    ModelFunc get_Cp;
    float process_noise[2][2];
    float state_covariance[2][2];
    float meas_noise;
    float alpha;      // low-pass filter factor
} FilterConfig;

void SOC_predict_then_update(float g_total_pack_voltage_soc, float g_pack_current_soc, float time_step)
{
    predict_state(ekf, cfg, current, dt);
    update_filter( ekf, cfg, measured_V, current, filtered_I);    
}



/*--- Utility: 2×2 Matrix Multiply ---*/
static void mat2_mul(const float A[2][2], const float B[2][2], float out[2][2]) {
    out[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0];
    out[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1];
    out[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0];
    out[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1];
}

/*--- Utility: 2×2 Matrix Add ---*/
static void mat2_add(const float A[2][2], const float B[2][2], float out[2][2]) {
    out[0][0] = A[0][0] + B[0][0];
    out[0][1] = A[0][1] + B[0][1];
    out[1][0] = A[1][0] + B[1][0];
    out[1][1] = A[1][1] + B[1][1];
}

/*--- Finite‐difference derivative ---*/
static float central_diff(ModelFunc f, float v) {
    const float h = 1e-6f;
    float v0 = v - h, v1 = v + h;
    if (v0 < 0.0f) v0 = 0.0f;
    if (v1 > 1.0f) v1 = 1.0f;
    return (f(v1) - f(v0)) / (2.0f * h);
}

/*--- Initialize EKF + Model ---*/
void init_filter(EKF *ekf, FilterConfig *cfg, State init, float *filtered_I) {
    // sanity checks
    assert(init.soc >= 0.0f && init.soc <= 1.1f);
    assert(cfg->alpha >= 0.0f && cfg->alpha <= 1.0f);

    // init state
    ekf->x[0] = init.soc;
    ekf->x[1] = init.uc;

    // init covariances
    ekf->P[0][0] = cfg->state_covariance[0][0];
    ekf->P[0][1] = cfg->state_covariance[0][1];
    ekf->P[1][0] = cfg->state_covariance[1][0];
    ekf->P[1][1] = cfg->state_covariance[1][1];

    // copy noise
    ekf->Q[0][0] = cfg->process_noise[0][0];
    ekf->Q[0][1] = cfg->process_noise[0][1];
    ekf->Q[1][0] = cfg->process_noise[1][0];
    ekf->Q[1][1] = cfg->process_noise[1][1];
    ekf->R     = cfg->meas_noise;

    *filtered_I = 0.0f;
}

/*--- Predict step ---*/
void predict_state(EKF *ekf, FilterConfig *cfg, float current, float dt) {
    // build B
    ekf->B[0] = -dt / cfg->Q_total;
    float tau = cfg->get_Rp(ekf->x[0]) * cfg->get_Cp(ekf->x[0]);
    ekf->B[1] = cfg->get_Rp(ekf->x[0]) * (1.0f - expf(-dt / tau));

    // build F
    ekf->F[0][0] = 1.0f;                ekf->F[0][1] = 0.0f;
    ekf->F[1][0] = 0.0f;
    ekf->F[1][1] = expf(-dt / tau);

    // x = F x + B u
    float x0 = ekf->F[0][0]*ekf->x[0] + ekf->F[0][1]*ekf->x[1] + ekf->B[0]*current;
    float x1 = ekf->F[1][0]*ekf->x[0] + ekf->F[1][1]*ekf->x[1] + ekf->B[1]*current;
    ekf->x[0] = x0; ekf->x[1] = x1;

    // P = F P Fᵀ + Q
    float tmp[2][2], Ft[2][2];
    // compute Ft = Fᵀ
    Ft[0][0]=ekf->F[0][0]; Ft[0][1]=ekf->F[1][0];
    Ft[1][0]=ekf->F[0][1]; Ft[1][1]=ekf->F[1][1];
    mat2_mul(ekf->F, ekf->P, tmp);
    mat2_mul(tmp, Ft, ekf->P);
    mat2_add(ekf->P, ekf->Q, ekf->P);
}

/*--- Update step ---*/
void update_filter(EKF *ekf, FilterConfig *cfg, float measured_V, float current, float *filtered_I) {
    // low-pass current
    *filtered_I = cfg->alpha * (*filtered_I) + (1.0f - cfg->alpha) * current;

    // measurement Jacobian H = [dUoc/dSOC - dR0/dSOC * I,  -1]
    float dU = central_diff(cfg->get_Uoc, ekf->x[0]);
    float dR = central_diff(cfg->get_R0,  ekf->x[0]);
    float H0 = dU - dR * (*filtered_I);
    float H1 = -1.0f;

    // predict measurement h = Uoc - Uc - R0*I
    float Uoc = cfg->get_Uoc(ekf->x[0]);
    float R0  = cfg->get_R0( ekf->x[0]);
    float h   = Uoc - ekf->x[1] - R0 * (*filtered_I);

    // innovation covariance S = H P Hᵀ + R
    ekf->S = H0*(H0*ekf->P[0][0] + H1*ekf->P[1][0])
           + H1*(H0*ekf->P[0][1] + H1*ekf->P[1][1])
           + ekf->R;

    // Kalman gain K = P Hᵀ / S
    ekf->K[0] = (ekf->P[0][0]*H0 + ekf->P[0][1]*H1) / ekf->S;
    ekf->K[1] = (ekf->P[1][0]*H0 + ekf->P[1][1]*H1) / ekf->S;

    // update state x = x + K*(z - h)
    float y = measured_V - h;
    ekf->x[0] += ekf->K[0] * y;
    ekf->x[1] += ekf->K[1] * y;

    // update covariance P = (I - K H) P (I - K H)ᵀ + K R Kᵀ
    float I_KH[2][2] = {
        {1.0f - ekf->K[0]*H0,     - ekf->K[0]*H1},
        {   - ekf->K[1]*H0, 1.0f - ekf->K[1]*H1}
    };
    float tmp1[2][2], tmp2[2][2];
    mat2_mul(I_KH, ekf->P, tmp1);
    // tmp2 = tmp1 * (I_KH)ᵀ
    mat2_mul(tmp1, I_KH, tmp2);
    // KRKᵀ (since R is scalar): [K0*K0*R  K0*K1*R; K1*K0*R  K1*K1*R]
    float KRKt[2][2] = {
        {ekf->K[0]*ekf->K[0]*ekf->R, ekf->K[0]*ekf->K[1]*ekf->R},
        {ekf->K[1]*ekf->K[0]*ekf->R, ekf->K[1]*ekf->K[1]*ekf->R}
    };
    mat2_add(tmp2, KRKt, ekf->P);

    // clamp SOC
    if (ekf->x[0] < 0.0f) ekf->x[0] = 0.0f;
    if (ekf->x[0] > 1.1f) ekf->x[0] = 1.1f;
}
