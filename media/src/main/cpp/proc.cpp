//
// Created by Scliang on 4/12/21.
//

#include "proc.h"

media::kalman::kalman():x_last(0.0), p_last(0.02), Q(0.018), R(0.542), kg(0.0),
x_mid(0.0), x_now(0.0), p_mid(0.0), p_now(0.0), z_real(0.0), z_measure(0.0) {
}

float media::kalman::filter(float i) {
    z_real = i;

    if (x_last == 0.0) {
        x_last = z_real;
        x_mid = x_last;
    }

    x_mid = x_last;
    p_mid = p_last + Q;
    kg = p_mid / (p_mid + R);
    z_measure = z_real;
    x_now = x_mid + kg * (z_measure - x_mid);
    p_now = (1 - kg) * p_mid;

    p_last = p_now;
    x_last = x_now;

    return x_now;
}
