#include "VideoStream.h"
#include "SPI.h"
#include "AmebaILI9341.h"
// Include the jpeg decoder library
#include "TJpg_Decoder.h"
#include "AmebaFatFS.h"
#include "inicio1.h"
#include "clasificacion.h"

//////// Librerias para el funcionamiento de el modelo de clasificacion 
#include "WiFi.h"
#include "StreamIO.h"
#include "RTSP.h"
#include "NNImageClassification.h"
#include "VideoStreamOverlay.h"
#include "ClassificationClassList.h"
/////// define pines canales ///////////////
#define IMAGERGB 1
#define CHANNEL 0
#define CHANNELNN 3
#define  LED 9 
///////////// Definicion de tamño  de las imagenes en el modelo
#define NNWIDTH  224
#define NNHEIGHT 224
///////////////// se definen los nombres de las carpetas y los nombres de los archivos


/////////////////////////////// configuracion de la camara ////////////
VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);
VideoSetting configNN(NNWIDTH, NNHEIGHT, 10, VIDEO_RGB, 0);
/////////////// seleccion de tarea//////////////////
NNImageClassification imgclass;
/////////// 

RTSP rtsp;
StreamIO videoStreamer(1, 1);
StreamIO videoStreamerNN(1, 1);
//////////////// CONEXION WIFI
char ssid[] = "Familia Sanabria";    // your network SSID (name)
char pass[] = "M936.Pda";        // your network password
int status = WL_IDLE_STATUS;

IPAddress ip;
int rtsp_portnum;
int button_State = 0;

//////////// CONEXIONES ILI9341
#define TFT_RESET 5
#define TFT_DC    4
#define TFT_CS    SPI_SS
//////////// configuracion tft ili9341

AmebaILI9341 tft = AmebaILI9341(TFT_CS, TFT_DC, TFT_RESET);

#define ILI9341_SPI_FREQUENCY 20000000

//VideoSetting config(VIDEO_VGA, CAM_FPS, VIDEO_JPEG, 1);
int button = 8;
uint32_t img_addr = 0;
uint32_t img_len = 0;

AmebaFatFS fs;
//int button = 8;
//int button_State = 0;
bool Camer_cap;
uint32_t count;
//// funcion interrupcion para  seleccion de la opciones 
void button_Handler(uint32_t id, uint32_t event)
{
    if (button_State == 0) {
        button_State = 1;
    }
}

// funcion graficar imagen 
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    tft.drawBitmap(x, y, w, h, bitmap);

    // Return 1 to decode next block
    return 1;
}

void setup()
{
  ////// iniciar ccomunicacion serial
    Serial.begin(115200);
      ///////////////////////////// CONEXION LED ILUMINACION 
      pinMode(LED,OUTPUT);
      digitalWrite(LED,HIGH);
      ///configuracion interrupcion 
     pinMode(button, INPUT_IRQ_FALL);
    digitalSetIrqHandler(button, button_Handler);


      //////////////////////////////CONEXION WIFI
   /* while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
       status = WiFi.begin(ssid, pass);

        // wait 2 seconds for connection:
        delay(2000);
    }
    ip = WiFi.localIP();*/

      ////////////////////////// TRANSMISION Y CONFIGURACION DE VIDEO
       config.setBitrate(2 * 1024 * 1024);    // Recommend to use 2Mbps for RTSP streaming to prevent network congestion
    Camera.configVideoChannel(CHANNEL, config);
    Camera.configVideoChannel(CHANNELNN, configNN);
    Camera.videoInit();
    rtsp.configVideo(config);
    rtsp.begin();
    //rtsp_portnum = rtsp.getPort();

    //////////////// configuracion de tarea de clasificacion de imagenes ///////////
     imgclass.configVideo(configNN);
    imgclass.configInputImageColor(IMAGERGB);
    imgclass.setResultCallback(ICPostProcess);
    imgclass.modelSelect(IMAGE_CLASSIFICATION, NA_MODEL, NA_MODEL, NA_MODEL, NA_MODEL, CUSTOMIZED_IMGCLASS);
    imgclass.begin();

  // Configure StreamIO object to stream data from video channel to RTSP
   videoStreamer.registerInput(Camera.getStream(CHANNEL));
    videoStreamer.registerOutput(rtsp);
    if (videoStreamer.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    // Start data stream from video channel
    Camera.channelBegin(CHANNEL);

    // Configure StreamIO object to stream data from RGB video channel to object detection
    videoStreamerNN.registerInput(Camera.getStream(CHANNELNN));
    videoStreamerNN.setStackSize();
    videoStreamerNN.setTaskPriority();
    videoStreamerNN.registerOutput(imgclass);
    if (videoStreamerNN.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    // Start data stream from video channel
    Camera.channelBegin(CHANNELNN);


      ///////////////////////////
    Serial.println("TFT ILI9341 ");

    SPI.setDefaultFrequency(ILI9341_SPI_FREQUENCY);
    pinMode(button, INPUT_IRQ_FALL);
    digitalSetIrqHandler(button, button_Handler);

    //Camera.configVideoChannel(CHANNEL, config);
    //Camera.videoInit();
    //Camera.channelBegin(CHANNEL);

    tft.begin();
    tft.setRotation(1);
    testBitmap(0, 0, inicioWidth,inicioHeight,inicio);
    delay(2000);
    testBitmap(0, 0, clasificacionWidth,clasificacionHeight,clasificacion);
    Rectangle(ILI9341_BLACK,56,135,145,30);
    Rectangle(ILI9341_BLACK,175,310,64,30);
    Rectangle(ILI9341_BLACK,175,310,143,28);
    Rectangle(ILI9341_BLACK,175,310,143,28);
    delay(2000);
    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(4);


    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);
}

void loop()
{
  Camera.getImage(CHANNEL, &img_addr, &img_len);
    TJpgDec.getJpgSize(0, 0, (uint8_t *)img_addr, img_len);
    TJpgDec.drawJpg(0, 0, (uint8_t *)img_addr, img_len);
    /*if (button_State == 1) {
      
        fs.begin();
        File file = fs.open(String(fs.getRootPath()) + String(FILENAME) + String(count) + String(".jpg"));
        file.write((uint8_t *)img_addr, img_len);
        delay(1);
       file.close();
       fs.end();
        count++;
        button_State = 0;
      TJpgDec.getJpgSize(0, 0, (uint8_t *)img_addr, img_len);
    TJpgDec.drawJpg(0, 0, (uint8_t *)img_addr, img_len);
    tft.setCursor(180, 70);
    tft.setForeground(ILI9341_WHITE);
    tft.setFontSize(2);
    tft.println("Resultado");
     tft.setCursor(180, 150);
    tft.setForeground(ILI9341_WHITE);
    tft.setFontSize(2);
    tft.println("Score");
    tft.setCursor(60, 195);
    tft.setForeground(ILI9341_WHITE);
    tft.setFontSize(2);
    tft.println("atras");
    tft.setCursor(65, 150);
    tft.setForeground(ILI9341_WHITE);
    tft.setFontSize(1);
    tft.println("Clasificar");
    clasifi();
    }
    Rectangle(ILI9341_BLACK,56,135,145,30);
    Rectangle(ILI9341_BLACK,175,310,64,30);
    Rectangle(ILI9341_BLACK,175,310,143,28);
    Rectangle(ILI9341_BLACK,175,310,143,28);
     */

    
}
void Rectangle(uint16_t color, int xin, int xfin, int yin, int anch)
{
    
    int x1 = xin;
    int x2 = xfin;
    int y= yin;
    int ancho =anch;   
    //tft.clr();

    for (int i= 0; i< ancho; i++) {
        tft.drawLine(x1,y+i,x2,y+i,color);
    }
}

void testBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const unsigned short *color)
{
    tft.clr();
    tft.drawBitmap(x, y, w, h, color);
    delay(500);
}
void ICPostProcess(void)
{
    int class_id = imgclass.classID();
    if (imgclassItemList[class_id].filter) {    // header file
        float prob = imgclass.score();
        printf("class %d, score: %f, name: %s\r\n", class_id, prob, imgclassItemList[class_id].imgclassName);   
    
        if (button_State == 1) {
          testBitmap(0, 0, clasificacionWidth,clasificacionHeight,clasificacion);
          Rectangle(ILI9341_BLACK,56,135,145,30);
    Rectangle(ILI9341_BLACK,175,310,64,30);
    Rectangle(ILI9341_BLACK,175,310,143,28);
    Rectangle(ILI9341_BLACK,175,310,143,28);
    Rectangle(ILI9341_BLACK,55,135,185,30);
    tft.setForeground(ILI9341_WHITE);
    tft.setFontSize(2);
    tft.println(imgclassItemList[class_id].imgclassName);
    tft.setCursor(180, 150);
    tft.setForeground(ILI9341_WHITE);
    tft.setFontSize(2);
    tft.println(prob);  
      tft.setCursor(63, 153);
      tft.setForeground(ILI9341_GREEN);
      tft.setFontSize(1);
      tft.println("clasificar");
      tft.setCursor(63, 195);
      tft.setForeground(ILI9341_WHITE);
      tft.setFontSize(2);
      tft.println("atras");
     button_State = 0;}  
     delay(1000);
    }
}
void clasifi(void){
   int class_id = imgclass.classID();
    if (imgclassItemList[class_id].filter) {    // header file
        float prob = imgclass.score();
       // printf("class %d, score: %f, name: %s\r\n", class_id, prob, imgclassItemList[class_id].imgclassName);
tft.setCursor(180, 70);
    tft.setForeground(ILI9341_WHITE);
    tft.setFontSize(2);
    tft.println(imgclassItemList[class_id].imgclassName);
    tft.setCursor(180, 150);
    tft.setForeground(ILI9341_WHITE);
    tft.setFontSize(2);
    tft.println(prob);
    }
}
