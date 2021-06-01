#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QString>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QKeyEvent>
#include <cstdio>
#include <iostream>

#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* strtof */

#include <iostream>
#include <list>

#include <stdio.h>
#include <string.h>

#include <math.h>       /* sqrt */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug() << "prograa iniciado";
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        qDebug() <<"name" + info.portName();
        qDebug() <<"Description" + info.description();
        qDebug() <<"VendorId" + QString::number( info.vendorIdentifier() );
        qDebug() <<"ProdructId" + QString::number( info.productIdentifier() );
        vendor = info.vendorIdentifier();
        product = info.productIdentifier();
        if(vendor == 1155 && product == 22336){
            portName = info.portName();
            qDebug() << "Tarjeta conectada";
            target = new QSerialPort(this);
            openSerialPort(portName);
            break;
        }
    }

    MakePlot();
    ace_ant = 0;
    val_ant = 0;
    max_vel = 0; max_ac = 0;

     change_vel = false;
     change_a = false;

     ui->x_space->setPlaceholderText("x_0");
     ui->x1_space->setPlaceholderText("x_1");
     ui->y_space->setPlaceholderText("y_0");
     ui->y1_space->setPlaceholderText("y_1");

     set_pos = false;

     ll= 0, lr =0;
     w = 500;
     h = 700;

     px = px_2 = px_1= 250;
     py = py_2 = py_1 = 387;

     I_x = 0;
     J_y = 0;

     Ack_ = true;
     file = false;
     decode = true;

     a_pos = 1;

     instru = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openSerialPort( QString p )
{
    if(target->isOpen()){
        target->close();
    }

    disconnect(target,SIGNAL(readyRead()),this,SLOT(readSerial()));

    target->setPortName(p);
    target->setBaudRate(QSerialPort::Baud115200);
    target->setDataBits(QSerialPort::Data8);
    target->setParity(QSerialPort::NoParity);
    target->setStopBits(QSerialPort::OneStop);
    target->setFlowControl(QSerialPort::NoFlowControl);

    connect(target,SIGNAL(readyRead()),this,SLOT(readSerial()));

    if(target->open(QIODevice::ReadWrite)){
        ui->infoLabel->setText("puerto abierto");
    }else{
        ui->infoLabel->setText(target->errorString());
    }

}

void MainWindow::decode_serial( unsigned char* data )
{
    uint8_t sum = data[1],i;

    for(i = 2; i < (data[2] + 3); i++){
        sum ^= data[i];
    }

    if(sum == data[data[2] + 3]){
        if(data[1] == 0x20){
            UpdateFlags(data);
        }else{ sendAck(true); data_check = true;}
    }else{ sendAck(false); data_check = false; }
}



void MainWindow::showDatato( unsigned char* data )
{
    uint32_t wf = 0;
    uint16_t wd = 0;

    switch (data[1]) {
    case 0x40:

        ui->PWM_label->setText(QString::number(data[3]));

        //UpdatePlot(data);

        break;

    case 0x41:

            wf = (((uint32_t)data[5] << 16) | ((uint32_t)data[4] << 8) | data[3]);

            ui->a_max_label->setText(QString::number(wf * 0.72));

            wd = (((uint16_t)data[7] << 8) | data[6]);

            //qDebug() << "data 8 : " << data[8] << " ACK: " << Ack_ << " set : " << set_pos;
            //qDebug() << "No_Ack : " << NoAck_;

            if( data[8] == 3 && Ack_ && set_pos && file && !decode){

                 t += delta_t;

                //qDebug() << "delta_t : " << t;

                if( instru == 1 && t <= 1){

                   // qDebug()<< "new point recta";
                    recta( t );

                } else if(instru == 2 && t <= beta){

                    curva( t );

                }else if(a_pos <= pos){

                    t = 0;
                    px_1 = px;
                    py_1 = py;
                    set_pos = false;
                    qDebug()<< "fin";
                    a_pos += 1;
                    decode = true;
                    Decode(a_pos);

                }

                ui->direccion_label->setText( QString::number(py));
                ui->distancia_label->setText( QString::number(px));


                Send_pos();

                Ack_ = false;
                NoAck_ = false;

            }
            else if( data[8] == 3 && Ack_ && !set_pos && file && !decode && a_pos <= pos){

                a_pos += 1;
                decode = true;
                Decode(a_pos);

            }
            else if( NoAck_ ){

                Send_pos();

                Ack_ = false;
                NoAck_ = false;

            }


        break;
    case 0x42:

        switch (data[3]) {

        case 0:
           // ui->flecha_label->setGeometry(-40,260,141,71);

            if(!data[4]) {
                ui->iniciar_label->setText("iniciar");
                ui->iniciar_label->setStyleSheet("* { background : green }");
            }
            else {
                ui->iniciar_label->setText("parar");
                ui->iniciar_label->setStyleSheet("* { background : red }");
            }

           // ui->reiniciar_label->setStyleSheet("* { background:rgb(255, 255, 0); }");
          //  ui->reiniciar_a_label->setStyleSheet("* { background:rgb(130, 255, 220) }");

            break;
        case 1:
           // ui->flecha_label->setGeometry(-40,310,141,71);

            if(!data[4]) {
                ui->iniciar_label->setText("iniciar");
                ui->iniciar_label->setStyleSheet("* { background : green }");
            }
            else {
                ui->iniciar_label->setText("parar");
                ui->iniciar_label->setStyleSheet("* { background : red }");
            }

           // ui->reiniciar_label->setStyleSheet("* { background:rgb(255, 255, 0); }");
          //  ui->reiniciar_a_label->setStyleSheet("* { background:rgb(130, 255, 220) }");

             break;
        case 2:

         //   ui->flecha_label->setGeometry(-40,360,141,71);

            if(!data[4]) {
                ui->iniciar_label->setText("iniciar");
                ui->iniciar_label->setStyleSheet("* { background : green }");
            }
            else {
                ui->iniciar_label->setText("parar");
                ui->iniciar_label->setStyleSheet("* { background : red }");
            }

            //ui->reiniciar_label->setStyleSheet("* { background:rgb(255, 255, 0); }");
         //   ui->reiniciar_a_label->setStyleSheet("* { background:rgb(130, 255, 220) }");

            break;
        case 3:

         //    ui->flecha_label->setGeometry(150,400,141,71);
             ui->iniciar_label->setStyleSheet("* { background : blue }");

             if(!data[4]) {
                 ui->iniciar_label->setText("iniciar");

             }
             else {
                 ui->iniciar_label->setText("parar");

             }

             //ui->reiniciar_label->setStyleSheet("* { background:rgb(255, 255, 0); }");
        //     ui->reiniciar_a_label->setStyleSheet("* { background:rgb(130, 255, 220) }");

            break;

        case 4:

         //   ui->flecha_label->setGeometry(150,400,141,71);

            if(!data[4]) {
                ui->iniciar_label->setText("iniciar");
                ui->iniciar_label->setStyleSheet("* { background : green }");
            }
            else {
                ui->iniciar_label->setText("parar");
                ui->iniciar_label->setStyleSheet("* { background : red }");
            }

        //    ui->reiniciar_a_label->setStyleSheet("* { background:rgb(130, 255, 220) }");
           // ui->reiniciar_label->setStyleSheet("* { background : blue }");

            break;
        case 5:

       //     ui->flecha_label->setGeometry(150,400,141,71);

            if(!data[4]) {
                ui->iniciar_label->setText("iniciar");
                ui->iniciar_label->setStyleSheet("* { background : green }");
            }
            else {
                ui->iniciar_label->setText("parar");
                ui->iniciar_label->setStyleSheet("* { background : red }");
            }

       //     ui->reiniciar_a_label->setStyleSheet("* { background : blue }");
            //ui->reiniciar_label->setStyleSheet("* { background:rgb(255, 255, 0); }");

            break;
        }

        if(data[5] != change_vel){
             max_vel = 0;
             ui->vel_max_label->setText(QString::number(0));
             change_vel = data[5];
        }
        if(data[6] != change_a){
            max_ac = 0;
            ui->a_max_label->setText(QString::number(0));
            change_a = data[6];
        }

        break;
    case 0x43:

         wf = ( ((uint32_t)data[5] << 16) | ((uint32_t)data[4] << 8) | data[3]);
         /*
        if(wf != 0){
            writeC(QString::number(wf));
        }else{
            writeC(" ");
        }
        */

         ll = (uint16_t) (wf*0.72);

         //ui->direccion_label->setText(QString::number(wf * 0.79));

        break;

       case 0x44:

        wf = (((uint32_t) data[5] << 16) | ((uint32_t)data[4] << 8) | data[3]);
        float dis;

        lr = (uint16_t) (wf*0.72);

        cal_x_y();

        /*
        if(data[6] == 1){
            dis = (wf) * (-0.079);
            ui->distancia_label->setText(QString::number((wf) * (-0.79)));
        }
        else{
            ui->distancia_label->setText(QString::number((wf) * (0.79)));
            dis = (wf) * (0.079);
        }
        */

        y.pop_front();
        y.append(dis);

        /*

        ui->customPlot_v->graph(0)->setData(x,y);
        ui->customPlot_v->replot();
        ui->customPlot_v->update();

        */

        break;
    }
}

void MainWindow::sendAck(bool check)
{
    if(check){
        target->write((char*)&_Ack,6);
    }else{
        target->write((char*)&_No_Ack,6);
    }
}

void MainWindow::UpdateFlags( unsigned char* data )
{
    if(data[3] == 0x1){

        Ack_ = true;
        NoAck_ = false;
        //qDebug()<< "ACK";

    }else{
        Ack_ = false;
        NoAck_ = true;
        target->write((char*)&paquete,(paquete[2]) + 5);
        qDebug()<< "NoACK";
    }
}

void MainWindow::writeC(QString text){
    //ui->controlLabel->setText(text);
}

void MainWindow::MakePlot()
{

    for (int i=0; i<3000; ++i)
    {
      x.append(i); // x goes from -1 to 1
      y.append(0); // let's plot a quadratic function
    }
    for(int i=0; i < 1000; ++i){
        x_a.append(i);
        y_a.append(0);
    }

    /*

    // create graph and assign data to it:
    ui->customPlot_v->addGraph();
    ui->customPlot_v->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->customPlot_v->xAxis->setLabel("muestra (10ms)");
    ui->customPlot_v->yAxis->setLabel("distancia (cm)");
    // set axes ranges, so we see all data:
    ui->customPlot_v->xAxis->setRange(0, 3000);
    ui->customPlot_v->yAxis->setRange(-20, 90);
    ui->customPlot_v->replot();

    ui->customPlot_a->addGraph();
    ui->customPlot_a->graph(0)->setData(x_a, y_a);
    // give the axes some labels:
    //vel *= 16;
    ui->customPlot_a->xAxis->setLabel("muestra (10ms)");
    ui->customPlot_a->yAxis->setLabel("Amperios (mA)");
    // set axes ranges, so we see all data:
    ui->customPlot_a->xAxis->setRange(0, 1000);
    ui->customPlot_a->yAxis->setRange(0, 1500);
    ui->customPlot_a->replot();

    */
}


void MainWindow::UpdatePlot(unsigned char *data)
{
    uint32_t vel = ( ((uint32_t)data[7] << 16) | ((uint32_t) data[6] << 8 ) | data[5] );
    //uint16_t adc = (((uint16_t)data[9] << 8) | data[8]);

   // vel *= 4;

   // if(vel > 0){

    ui->vel_max_label->setText(QString::number(vel * 0.72));

/*
    if( ((vel) - (val_ant)) != 0 ){
        ace_ant =  ((vel) - (val_ant));
    }

    if( max_ac < ace_ant){
        max_ac = ace_ant;
        ui->a_max_label->setText(QString::number(max_ac));
    }

    val_ant = vel;

    }

    adc_float = adc * 0.8;

    y_a.pop_front();
    y_a.append(adc_float);
    ui->customPlot_a->graph(0)->setData(x_a,y_a);
    ui->customPlot_a->replot();
    ui->customPlot_a->update();

    */

}

void MainWindow::ArmarPack(uint8_t *data, uint8_t command, uint8_t packLen)
{

    paquete[0] = START; // inicio
    paquete[1] = command; // comando
    paquete[2] = packLen; // tamaño 1
    for(i = 0; i < packLen; i++){
        paquete[i + 3] = data[i]; // data
    }
    XORData( paquete );
    paquete[packLen + 3] = sum;
    paquete[packLen + 4] = END; // final
}

void MainWindow::XORData(uint8_t* data){
    sum = data[1];
    for (i = 2; i < (data[2] + 3); i++){
        sum ^= data[i];
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == (Qt::Key_Enter - 1) ){

        uint8_t data_send[125];

        if( ui->x_space->text() != "" && ui->y_space->text() != "" ){

            float x = ui->x_space->text().toFloat() , y = ui->y_space->text().toFloat();

            uint16_t ll = sqrt( (x*x) + ((700-y)*(700-y)) );
            uint16_t lr = sqrt( (500-x)*(500-x) + ((700-y)*(700-y)));


            if(ui->x1_space->text() != "" || ui->y1_space->text() != ""){

                px_1 = ui->x_space->text().toInt() , py_1 = ui->y_space->text().toInt();
                px = px_1, py = py_1;
                px_2 = ui->x1_space->text().toInt() , py_2 = ui->y1_space->text().toInt();


                qDebug() << "y_curva";
                Interpolar_curva();

                //Interpolar_recta();


                set_pos = true;

            }else{

                set_pos = false;

            }

            data_send[0] = (uint8_t) ll;
            data_send[1] = (uint8_t)(ll >> 8);
            data_send[2] = (uint8_t) lr;
            data_send[3] = (uint8_t)(lr >> 8);

            qDebug() << QString::number((uint16_t)ll);
            qDebug() << QString::number((uint16_t)lr);


        }

        //dataArray = h.toLocal8Bit();
        //const char *ready = dataArray.data();

        ArmarPack(data_send,0x60,4);
/*
        for(int i = 0; i < (paquete[2] + 5) ; i++ ){
            qDebug() << paquete[i];
        }
*/

        target->write((const char*)paquete,(paquete[2]) + 5);

    }
}

void MainWindow::cal_x_y()
{

    x_pos = ( -(lr*lr) + (w*w) + (ll*ll) )/(2*w);
    y_pos = h - sqrt( (ll*ll) - (x_pos*x_pos) );

    ui->distancia_label->setText(QString::number(x_pos));
    ui->direccion_label->setText(QString::number(y_pos));

}

void MainWindow::Interpolar_recta()
{

    float dt_x = 10, dt_y = 10;

    t = 0;

    if( -px_1 + px_2 != 0 ) dt_x = (float) 1/ (float) (-px_1+px_2);
    if( -py_1 + py_2 != 0 ) dt_y = (float)  1/ (float) (-py_1 + py_2);

    if( abs(dt_x )< abs(dt_y) ) delta_t = abs(dt_x);
    else delta_t = abs(dt_y);

    set_pos = true;

}

void MainWindow::Interpolar_curva()
{
    r = sqrt((I_x*I_x) + (J_y*J_y));

    delta_t = 1/r;

    c_x = I_x + px_1;
    c_y = J_y + py_1;

    if( -(J_y) >= 0 && -(I_x) > 0) alpha = (float) asin( (double) -(J_y)/r);
    if( -(J_y) > 0 && -(I_x) < 0) alpha = (float) - asin( (double) -(J_y)/r) + PI;
    if( -(J_y) < 0 && -(I_x) < 0) alpha = (float) PI + abs(asin( (double) -(J_y)/r));
    if( -(J_y) < 0 && -(I_x) > 0) alpha = (float) (2*PI) - abs(asin( (double) -(J_y)/r));

    float delta_y = (float)py_2 - (float)c_y;
    float delta_x = (float)px_2 - (float)c_x;

    if( delta_y >= 0 && delta_x > 0) beta = (float) asin( (double) delta_y/r );
    if( delta_y > 0 && delta_x < 0) beta = (float) -asin( (double) delta_y/r ) + (PI);
    if( delta_y < 0 && delta_x < 0) beta = (float) (PI) + abs(asin( (double) delta_y/r ));
    if( delta_y < 0 && delta_x > 0) beta = (float) (2*PI) - abs(asin( (double) delta_y/r ));

    t = alpha;

    if(alpha == beta){
        beta = 2*PI;
    }

    set_pos = true;

   // alpha *= 180/PI;
   // beta *= 180/PI;

    qDebug() << "apha: " << alpha << " beta: " << beta << " r : " << r;

}

void MainWindow:: recta( float t )
{

    px = (uint16_t) ((px_1) * (1 - t)) + (px_2 * t);
    py = (uint16_t) ((py_1) * (1 - t)) + (py_2 * t);

    qDebug() << " t: " << t;

    //return ( b_e + ( m_e * ( x - x_e ) ) );
}

void MainWindow::curva( float t )
{
    px = (uint16_t) ( (r * cos( (double) t)) + c_x );
    py = (uint16_t) ( (r * sin( (double) t)) + c_y );

     qDebug() << " x: " << px;
     qDebug() << " y: " << py;
}

void MainWindow::Send_pos()
{

        //qDebug()<< "new point";

        uint16_t ll = sqrt( (px * px ) + ((700 - py)*(700 - py)));
        uint16_t lr = sqrt( (500 - px )*(500 - px)  + ((700 - py)*(700 - py)));

        //qDebug() << QString::number(ll);
        //qDebug() << QString::number(lr);

        data_send[0] = (uint8_t) ll;
        data_send[1] = (uint8_t)(ll >> 8);
        data_send[2] = (uint8_t) lr;
        data_send[3] = (uint8_t)(lr >> 8);

        ArmarPack(data_send,0x60,4);

       target->clear( target -> Output);
       target->write((char*)&paquete,(paquete[2]) + 5);

}

void MainWindow::getData()
{

    const char * temp = dir.toLocal8Bit();
    stream = fopen( temp, "r+");

    if(stream == NULL){
        printf("!!!! No file");
        ui->infoLabel_file->setText("No File");
        exit(1);
    }

    ui->infoLabel_file->setText("File loaded");
    resetAll();
    file = true;

    while (fgets(line, 1024, stream))
    {

        token = strtok(line,"\n"); // primera palabra (dic)


        if( token[0] == 'G' ||  token[0] == 'X' || token[0] == 'Y')
        {

            strcpy(diccionarioP[pos].principal,token);

            a = strlen(token); // largo de la primera palabra
            //token = strtok(line + a + 1,","); // segunda palabra (sin)

            //strcpy(diccionarioP[pos].sinonimo,token);
            qDebug()<< diccionarioP[pos].principal;
            pos += 1;

        }

    }

    printf("\n>>> base de datos cargada... \n\n");

    Decode( 1 );

    fclose(stream);
}

void MainWindow::Decode( int pos )
{
 //
    QString temp = "";

    for(int i = 0; i < (int) strlen(diccionarioP[pos].principal) ; i++){

        //temp.push_front(diccionarioP[pos].principal[i]);

        if( (diccionarioP[pos].principal[i] - '0' <= 9 &&
            diccionarioP[pos].principal[i] - '0' >= 0 ) ||
            diccionarioP[pos].principal[i] == '.' ||
            diccionarioP[pos].principal[i] == '-' ){

            temp.push_back(diccionarioP[pos].principal[i]);

            if( (diccionarioP[pos].principal[i+1] - '0' > 9 ||
                 diccionarioP[pos].principal[i+1] - '0' < 0 ) &&
                 diccionarioP[pos].principal[i+1] != '.' &&
                 diccionarioP[pos].principal[i+1] != '-'){

                res.num = temp.toFloat();
                temp = "";
                qDebug() << "leta:" << res.let << "num:" << res.num ;


                switch (res.let) {
                    case 'G':
                        instru = res.num;
                    break;
                    case 'X':
                        px_2 = (uint16_t) res.num ;
                    break;
                    case 'Y':
                        py_2 = (uint16_t) res.num;
                    break;
                    case 'I':
                        I_x = res.num ;
                    break;
                    case 'J':
                        J_y = res.num;
                    break;
                }

            }

        }else{
            res.let = diccionarioP[pos].principal[i];
        }
    }

    select_action();

    decode = false;

}

void MainWindow::select_action()
{

    switch (instru) {
     case 0:
        goTo();
        break;
    case 1:
        qDebug()<< "new recta";
        Interpolar_recta();
        break;
    case 2:
        qDebug()<< "new curva";
        Interpolar_curva();
        break;
    case 3:
        break;
    }

}

void MainWindow::goTo()
{

   qDebug()<< " GO TO";

    uint8_t data_send[30];

    px = px_1 = px_2;
    py = py_1 = py_2;

    float x = (float) px_2 , y = (float) py_2;

    uint16_t ll = sqrt( (x*x) + ((700-y)*(700-y)) );
    uint16_t lr = sqrt( (500-x)*(500-x) + ((700-y)*(700-y)));

    data_send[0] = (uint8_t) ll;
    data_send[1] = (uint8_t)(ll >> 8);
    data_send[2] = (uint8_t) lr;
    data_send[3] = (uint8_t)(lr >> 8);

    //qDebug() << QString::number((uint16_t)ll);
    //qDebug() << QString::number((uint16_t)lr);

    set_pos = false;

    ArmarPack(data_send,0x60,4);

    target->write((const char*)paquete,(paquete[2]) + 5);

}

void MainWindow::resetAll()
{
    set_pos = false;
    ll= 0, lr =0;
    w = 500;
    h = 700;
    px = px_2 = px_1= 250;
    py = py_2 = py_1 = 387;
    I_x = 0;
    J_y = 0;
    t = 0;
    Ack_ = true;
    file = false;
    decode = true;
    a_pos = 1;
    pos = 0;
    instru = 0;
}

void MainWindow::readSerial()
{
    QByteArray serialData = target -> readAll();
    unsigned char* data = (unsigned char*) serialData.data();

    if(data[0] == 0xff && data[data[2] + 4] == 0xfe){
        decode_serial(data);
    }else{
        target->clear(target->AllDirections);
        qDebug() << QString::number( data[0], 16)  << QString::number( data[0], 16);
    }

    if(data_check){
        //chooseKey( data );
        showDatato( data );
    }


}

void MainWindow::chooseKey( uint8_t *data )
{
    switch (data[4]) {
    case 0xEB:
        switch (data[3]) {
            case 0xCF:
                writeC("UP");
            break;
            case 0xAF:
                writeC("DOWN");
            break;
            case 0x9F:
                writeC("RIGHT");
            break;
            case 0xEF:
                writeC("LEFT");
            break;

        }
        break;
    case 0xCB:
        switch (data[3]) {
            case 0x00:
                writeC("1");
            break;
            case 0x40:
                writeC("2");
            break;
            case 0x20:
                writeC("3");
            break;
            case 0x60:
                writeC("4");
            break;
            case 0x10:
                writeC("5");
            break;
            case 0x50:
                writeC("6");
            break;
            case 0x30:
                writeC("7");
            break;
            case 0x70:
                writeC("8");
            break;
            case 0x08:
                writeC("9");
            break;
            case 0x48:
                writeC("0");
            break;
            case 0x06:
                writeC("prev");
            break;
            case 0x46:
                writeC("next");
            break;
            case 0x22:
                writeC("scan / left");
            break;
            case 0x26:
                writeC("play");
            break;
            case 0x62:
                writeC("scan / left");
            break;
            case 0x4e:
                writeC("PAUSE");
            break;
            case 0x0e:
                writeC("STOP");
            break;
            case 0x38:
                writeC("return");
            break;
            case 0x2c:
                writeC("top Menu");
            break;
            case 0x6c:
                writeC("Menu");
            break;
            case 0x54:
                writeC("I/O");
            break;
            case 0x34:
                writeC("Open/Close");
            break;
            case 0x68:
                writeC("ENTER");
            break;
        }
        break;

    case 0x10:
        switch (data[3]) {
            case 0x54:
                writeC("I/O (tv)");
            break;
            case 0x24:
                writeC("vol +");
            break;
            case 0x64:
                writeC("vol -");
            break;
        }
        break;
    }
}





void MainWindow::on_Home_button_clicked()
{

    qDebug()<< "Home";

    uint8_t data_send[125];

    float x = 250 , y = 387;

    px = px_1 = px_2 = (uint16_t) x;
    py = py_1 = py_2 = (uint16_t) y;

    file = false;

    uint16_t ll = sqrt( (x*x) + ((700-y)*(700-y)) );
    uint16_t lr = sqrt( (500-x)*(500-x) + ((700-y)*(700-y)));

    data_send[0] = (uint8_t) ll;
    data_send[1] = (uint8_t)(ll >> 8);
    data_send[2] = (uint8_t) lr;
    data_send[3] = (uint8_t)(lr >> 8);

    qDebug() << QString::number((uint16_t)ll);
    qDebug() << QString::number((uint16_t)lr);

    set_pos = false;

    //dataArray = h.toLocal8Bit();
    //const char *ready = dataArray.data();

    ArmarPack(data_send,0x60,4);
/*
    for(int i = 0; i < (paquete[2] + 5) ; i++ ){
        qDebug() << paquete[i];
    }
*/
    target->write((const char*)paquete,(paquete[2]) + 5);

}

void MainWindow::on_pushButton_2_clicked()
{

    dir = "D:/SERGIO/Sergio(noCloud)/a empezar de nuevo 2.0/Universidad/embebidos/Codigos G/" + ui->file_load->text().toLocal8Bit();
    getData();

}
