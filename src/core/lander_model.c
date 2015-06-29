#include <math.h>
#include "GL/glew.h"
#include "core/lander_model.h"
#include "core/game.h"
#include "tools/linmath.h"
#include "tools/glt_tools.h"

#define PI 3.14159265359
#define DEFAULT_SCALE 0.06
//#define DEFAULT_SCALE 0.3

static GLfloat position_data[];

void lndr_gen_colors(GLfloat *buf)
{
    int i;
    /*
    for(i = 0; i < 56 * 3; i += 3)
    {
        buf[i] = 1;
        buf[i + 1] = 1;
        buf[i + 2] = 1;
    }*/

    for(i = 0; i < 4 * 3; i += 3)
    {
        buf[i] = 1.0;
        buf[i + 1] = 0.68;
        buf[i + 2] = 0;
    }

    for(; i < 8 * 3; i += 3)
    {
        buf[i] = 1.0;
        buf[i + 1] = 0.27;
        buf[i + 2] = 0.11;
    }
}

void lndr_gen_mv_matrix(Lander *lander)
{
    mat4x4 ident, temp, trans;
    mat4x4_identity(ident);

    float glx = 2 / glob_game.ppw;
    float gly = -2 / glob_game.pph;

    glx *= lander->x;
    gly *= lander->y;

    glx--;
    gly++;

    mat4x4_scale_aniso(temp, ident, DEFAULT_SCALE, DEFAULT_SCALE, DEFAULT_SCALE);
    //mat4x4_translate_in_place(temp, glx, gly, 0);
    mat4x4_rotate_Z(temp, temp, lander->rotation);
    mat4x4_translate(trans, glx, gly, 0);
    mat4x4_mul(ident, trans, temp);
    mat4x4_transpose(lander->mvMatrix, ident);
}

void lndr_gen_jet_mv_matrix(Lander *lander)
{
    if(lander->jetState == JS_OFF)
        return;

    float scale = 0;
    if(lander->jetState == JS_INCREASING)
        scale = lander->jetFrames / 500.;

    if(lander->jetState == JS_ON)
    {
        if(lander->jetFrames < 10)
            return;

        lander->jetFrames = 0;

        int rnd = rand() % 6 - 3;
        scale = 1.0 + (0.07 * rnd);
    }

    mat4x4 ident, temp;
    mat4x4_identity(ident);

    mat4x4_translate_in_place(ident, 0, -0.3529412, 0);
    mat4x4_scale_aniso(temp, ident, 1.0, scale, 1.0);
    mat4x4_translate_in_place(temp, 0, 0.3529412, 0);

    mat4x4_transpose(lander->jetMatrix, temp);
}

Lander lndr_new()
{
    GLfloat cbuf[8 * 3];
    lndr_gen_colors(cbuf);

    Lander lander;
    lander.mesh = mh_make(position_data, NULL, 56);
    lander.jetMesh = mh_make(position_data + 112, cbuf, 8);

    lander.dX = 150;
    lander.dY = 50;
    lander.dYY = 5;
    lander.dR = 30;
    lander.rotation = -PI / 4;
    lander.x = 30;
    lander.y = 30;
    lander.jetState = JS_ON;
    lander.jetFrames = 0;

    lndr_gen_mv_matrix(&lander);
    lndr_gen_jet_mv_matrix(&lander);

    mh_set_u_color(&lander.mesh, 1, 1, 1);

    return lander;
}

void lndr_step(Lander *lander, float dT)
{
    lander->x += lander->dX * dT;
    lander->y += lander->dY * dT;
    lander->dY += lander->dYY * dT;

    lander->jetFrames += dT * 1000;

    lndr_gen_mv_matrix(lander);
    lndr_gen_jet_mv_matrix(lander);
}

void lndr_render(Lander *lander)
{

    /*
    mat4x4 ident, mv, temp;
    mat4x4_identity(ident);

    mat4x4_scale_aniso(temp, ident, 0.5, 0.5, 0.5);
    mat4x4_transpose(mv, temp);*/

    glUniformMatrix4fv(glob_locs.uMVMatrix, 1, GL_FALSE, (GLfloat *)lander->mvMatrix);

    glBindVertexArray(lander->mesh.VAO);
    mh_prepare_for_render(&lander->mesh);
    glDrawArrays(GL_LINES, 0, 56);
    glBindVertexArray(0);

    if(lander->jetState != JS_OFF)
    {
        mat4x4 mv;
        mat4x4_mul(mv, lander->jetMatrix, lander->mvMatrix);
        glUniformMatrix4fv(glob_locs.uMVMatrix, 1, GL_FALSE, (GLfloat *)mv);
        
        glBindVertexArray(lander->jetMesh.VAO);
        mh_prepare_for_render(&lander->jetMesh);
        glDrawArrays(GL_LINES, 0, 8);
        glBindVertexArray(0);
    }
}

static GLfloat position_data[] = {
        -1.875000e-01, 0.500000,
        0.187500, 0.500000,
        -1.875000e-01, 0.500000,
        -3.125000e-01, 0.382353,
        0.187500, 0.500000,
        0.312500, 0.382353,
        -3.125000e-01, 0.382353,
        -3.125000e-01, 0.147059,
        0.312500, 0.382353,
        0.312500, 0.147059,
        -3.125000e-01, 0.147059,
        -1.875000e-01, 0.029412,
        0.312500, 0.147059,
        0.187500, 0.029412,
        -3.125000e-01, 0.029412,
        0.312500, 0.029412,
        -3.125000e-01, 0.029412,
        -3.125000e-01, -1.470588e-01,
        0.312500, 0.029412,
        0.312500, -1.470588e-01,
        -3.125000e-01, -1.470588e-01,
        0.312500, -1.470588e-01,
        -3.125000e-01, -5.882353e-02,
        -4.375000e-01, -3.235294e-01,
        0.312500, -5.882353e-02,
        0.437500, -3.235294e-01,
        -2.187500e-01, -1.470588e-01,
        -4.375000e-01, -3.235294e-01,
        0.218750, -1.470588e-01,
        0.437500, -3.235294e-01,
        -4.375000e-01, -3.235294e-01,
        -4.375000e-01, -4.411765e-01,
        0.437500, -3.235294e-01,
        0.437500, -4.411765e-01,
        -5.000000e-01, -4.411765e-01,
        -3.750000e-01, -4.411765e-01,
        0.375000, -4.411765e-01,
        0.500000, -4.411765e-01,
        -9.375000e-02, -1.470588e-01,
        -9.375000e-02, -2.058824e-01,
        0.093750, -1.470588e-01,
        0.093750, -2.058824e-01,
        -9.375000e-02, -2.058824e-01,
        0.093750, -2.058824e-01,
        -9.375000e-02, -2.058824e-01,
        -1.562500e-01, -3.235294e-01,
        0.093750, -2.058824e-01,
        0.156250, -3.235294e-01,
        -1.562500e-01, -3.235294e-01,
        0.156250, -3.235294e-01,
        -1.562500e-01, -3.235294e-01,
        -1.562500e-01, -3.529412e-01,
        0.156250, -3.235294e-01,
        0.156250, -3.529412e-01,
        -1.562500e-01, -3.529412e-01,
        0.156250, -3.529412e-01,

        -0.1562500, -0.3529412,
        0.000000, -1.5,
        0.156250, -0.3529412,
        0.000000, -1.5,

        -0.11, -0.3529412,
        0.0, -0.8,
        0.11, -0.3529412,
        0.0, -0.8
};

