#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QKeyEvent>


// commands
#define START 0xff
#define END 0xfe
#define Ack 0x20 // Ack
#define SETCLOCK 0x25 // Update Clock
#define SETALARM 0x26
#define Cron 0x22 // Update Cron
#define Alarm 0x23 // Update
#define ALL 0x24 // all


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void readSerial();

    void on_Home_button_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *target = nullptr;
    int vendor = 0, product = 0;
    QString portName = "";
    void openSerialPort( QString p );

    // flags
    bool Ack_       = false,
         NoAck_     = false,
         data_check = false;

    // plot
    QVector<double> x, y,x_a,y_a;
    uint16_t val_ant;
    int ace_ant;

    uint16_t max_vel, max_ac;

    bool change_vel,change_a;

    //codes
    uint8_t _Ack[6] = {0xff, 0x20, 0x01, 0x01, 0x20, 0xfe};
    uint8_t _No_Ack[6] = {0xff, 0x20, 0x01, 0x00, 0x21, 0xfe};

    // decode_serial
    void decode_serial( unsigned char* data ); // ALT + ENTER crear funcion en cpp
    void showDatato(unsigned char *data);
    //ACK send
    void sendAck(bool check);
    void UpdateFlags(unsigned char *data);

    //control select
    void chooseKey( uint8_t *data );
    void writeC(QString text);

    // make Plot
    void MakePlot();
    void UpdatePlot( unsigned char* data );

    float adc_float;

    // comunication

    void ArmarPack(uint8_t* data, uint8_t command, uint8_t packLen);
    uint8_t paquete[125];
    uint8_t data_Ch[125];
    uint8_t data_send[125];
    uint8_t sum = 0;
    uint8_t i;
    void XORData(uint8_t* data);
    void keyPressEvent(QKeyEvent *event);

    // x y

    uint16_t ll, lr,w,h;
    uint16_t x_pos,y_pos;

    void cal_x_y();

    // dibujar

    float m_e,x_e,b_e;

    float t, delta_t;
    uint16_t px , py;
    float c_x,c_y;
    const double PI =  3.1415926;

    uint16_t px_1,px_2, py_1,py_2;
    void Interpolar_recta();
    void Interpolar_curva();
    void recta( float t );

    // curva
    void curva( float t);
    float I_x, J_y,r;
    float alpha, beta;

    void Send_pos();

    bool set_pos;

    // leer archivo

    void getData();

    // cargar palabra

    char line[1024];
    char* token;
    FILE* stream;

    typedef struct Palabra{
        char principal[80];
        char sinonimo[80];
    }Palabra;

    Palabra diccionarioP[5000];

    unsigned int pos = 0; // ultima posición

    //pruebas
    unsigned int a;       // largo de una palabra


    // decode archivo nc

    void Decode( int pos );
    uint8_t instru;
    QString dir;

    typedef struct Instruccion{
        char let;
        float num;
    }Instruccion;

    Instruccion res;

    // seleccionar que hacer

    void select_action();
    void goTo();

    uint16_t a_pos;
    bool file,decode;

    void resetAll();

};
#endif // MAINWINDOW_H
