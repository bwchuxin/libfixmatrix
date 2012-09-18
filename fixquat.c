#include "fixquat.h"
#include "fixarray.h"

// Conjugate of quaternion
void qf16_conj(qf16 *dest, const qf16 *q)
{
    dest->a = q->a;
    dest->b = - q->b;
    dest->c = - q->c;
    dest->d = - q->d;
}

// Multiply two quaternions, dest = a * b.
void qf16_mul(qf16 *dest, const qf16 *q, const qf16 *r)
{
    qf16 tmp;
    fa16_unalias(dest, (void**)&q, (void**)&r, &tmp, sizeof(tmp));
    
    dest->a = fix16_mul(q->a, r->a) - fix16_mul(q->b, r->b) - fix16_mul(q->c, r->c) - fix16_mul(q->d, r->d);
    dest->b = fix16_mul(q->a, r->b) + fix16_mul(q->b, r->a) + fix16_mul(q->c, r->d) - fix16_mul(q->d, r->c);
    dest->c = fix16_mul(q->a, r->c) - fix16_mul(q->b, r->d) + fix16_mul(q->c, r->a) + fix16_mul(q->d, r->b);
    dest->d = fix16_mul(q->a, r->d) + fix16_mul(q->b, r->c) - fix16_mul(q->c, r->b) + fix16_mul(q->d, r->a);
}

// Multiply quaternion by scalar
void qf16_mul_s(qf16 *dest, const qf16 *q, fix16_t s)
{
    dest->a = fix16_mul(q->a, s);
    dest->b = fix16_mul(q->b, s);
    dest->c = fix16_mul(q->c, s);
    dest->d = fix16_mul(q->d, s);
}

// Divide quaternion by scalar
void qf16_div_s(qf16 *dest, const qf16 *q, fix16_t s)
{
    dest->a = fix16_div(q->a, s);
    dest->b = fix16_div(q->b, s);
    dest->c = fix16_div(q->c, s);
    dest->d = fix16_div(q->d, s);
}

// Quaternion norm
fix16_t qf16_norm(const qf16 *q)
{
    return fa16_norm(&q->a, &q->b - &q->a, 4);
}

// Normalize quaternion
void qf16_normalize(qf16 *dest, const qf16 *q)
{
    qf16_div_s(dest, q, qf16_norm(q));
}

// Quaternion power
void qf16_pow(qf16 *dest, const qf16 *q, fix16_t power)
{
    fix16_t old_half_angle = fix16_acos(q->a);
    fix16_t new_half_angle = fix16_mul(old_half_angle, power);
    fix16_t multiplier = 0;
    
    if (old_half_angle > 10) // Guard against almost-zero divider
    {
        multiplier = fix16_div(fix16_sin(new_half_angle),
                               fix16_sin(old_half_angle));
    }
    
    dest->a = fix16_cos(new_half_angle);
    dest->b = fix16_mul(q->b, multiplier);
    dest->c = fix16_mul(q->c, multiplier);
    dest->d = fix16_mul(q->d, multiplier);
}

void qf16_from_axis_angle(qf16 *dest, const v3d *axis, fix16_t angle)
{
    angle /= 2;
    fix16_t scale = fix16_sin(angle);
    
    dest->a = fix16_cos(angle);
    dest->b = fix16_mul(axis->x, scale);
    dest->c = fix16_mul(axis->y, scale);
    dest->d = fix16_mul(axis->z, scale);
}

// Unit quaternion to rotation matrix
void qf16_to_matrix(mf16 *dest, const qf16 *q)
{
    dest->rows = dest->columns = 3;
    dest->errors = 0;
    dest->data[0][0] = fix16_one - 2 * (fix16_sq(q->c) + fix16_sq(q->d));
    dest->data[1][1] = fix16_one - 2 * (fix16_sq(q->b) + fix16_sq(q->d));
    dest->data[2][2] = fix16_one - 2 * (fix16_sq(q->b) + fix16_sq(q->c));
    
    dest->data[1][0] = 2 * (fix16_mul(q->b, q->c) + fix16_mul(q->a, q->d));
    dest->data[0][1] = 2 * (fix16_mul(q->b, q->c) - fix16_mul(q->a, q->d));
    
    dest->data[2][0] = 2 * (fix16_mul(q->b, q->d) - fix16_mul(q->a, q->c));
    dest->data[0][2] = 2 * (fix16_mul(q->b, q->d) + fix16_mul(q->a, q->c));
    
    dest->data[2][1] = 2 * (fix16_mul(q->c, q->d) + fix16_mul(q->a, q->b));
    dest->data[1][2] = 2 * (fix16_mul(q->c, q->d) - fix16_mul(q->a, q->b));
}

void qf16_rotate(v3d *dest, const qf16 *q, const v3d *v)
{
    qf16 vector, q_conj;
    
    qf16_from_v3d(&vector, v, 0);
    qf16_conj(&q_conj, q);
    
    qf16_mul(&vector, q, &vector);
    qf16_mul(&vector, &vector, &q_conj);
    
    qf16_to_v3d(dest, &vector);
}
