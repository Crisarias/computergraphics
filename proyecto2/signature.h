#ifndef SIGNATURE_H
#define SIGNATURE_H

typedef struct {
    double x;
    double y;
} ControlPoint;

typedef struct {
    ControlPoint *control_points;
    int control_point_count;
    int sample_count;
} Curve;

typedef struct {
    Curve *curves;
    int curve_count;
} Signature;

Signature *create_signature_from_file(char *filepath_str);

void free_signature(Signature *signature);

#endif