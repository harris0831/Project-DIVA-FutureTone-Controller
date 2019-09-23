#include <Wire.h>
#include "Adafruit_MPR121_L.h"
#include "Adafruit_MPR121_C.h"
#include "Adafruit_MPR121_R.h"
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

Adafruit_MPR121_L cap_L = Adafruit_MPR121_L();
Adafruit_MPR121_C cap_C = Adafruit_MPR121_C();
Adafruit_MPR121_R cap_R = Adafruit_MPR121_R();

//LEDテープの設定(LED36個 7番ピンで制御)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(36, 7, NEO_GRB + NEO_KHZ800);

//センサー情報用変数
uint64_t currtouched_L = 0;
uint64_t currtouched_C = 0;
uint64_t currtouched_R = 0;
uint64_t currtouched_ALL = 0;

//タッチセンサー入力位置群
float currtouchpoint_ave_1 = 0.0;
float currtouchpoint_ave_2 = 0.0;
float lasttouchpoint_ave_1 = 0.0;
float lasttouchpoint_ave_2 = 0.0;

int currtouchpoint_ave_1_size = 0;
int currtouchpoint_ave_2_size = 0;

//センサー入力状態判定用変数
uint64_t bitcheck = 0x1000000000;	//処理中ビット位置
uint64_t bitcheck_L = 0x2000000000;	//処理中ビットの左側
uint64_t bitcheck_R = 0x0800000000;	//処理中ビットの右側

//タッチセンサー入力状況
float IPpoint1 = 0.0;	//タッチ位置
float IPpoint2 = 0.0;
int IPpoint1_vector = 0;	//スライド方向 1:右向き -1:左向き
int IPpoint2_vector = 0;

//コントローラーへの出力状況
float Rr_OPpoint = 0;	//位置情報
float Rl_OPpoint = 0;
float Lr_OPpoint = 0;
float Ll_OPpoint = 0;

//LEDテープの発光判定用
uint64_t LED_bitcheck = 0x1000000000;
uint16_t MAX_VAL = 1;	//LEDテープの明るさ設定
unsigned long LED_timerLast;	//LEDテープの虹色発光の開始判定
unsigned long FPS_timer1,FPS_timer2;

//LEDテープ発光パターン
//外向き・内向きWスライド
String CLR1_R = "PPPBBBGGGYYYOOORRR";	//LEDテープ右半分の発光色
String CLR1_L = "RRROOOYYYGGGBBBPPP";	//LEDテープ左半分の発光色

//同方向Wスライド
String CLR2 = "PPPBBBGGGYYYOOORRR";

//シングルスライド
String CLR3 = "PPPPPPBBBBBBGGGGGGYYYYYYOOOOOORRRRRR";

String CLR_BUF;
uint16_t CLR_type = 8;
uint16_t oldCLR_type = 0;
int CLR_Trigger = 0;

void setup(){
	Serial.begin(9600);

//LEDテープ初期化
	#if defined (__AVR_ATtiny85__)
		if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
	#endif
	strip.begin();
	strip.show();

//アナログスティック用のピンモード設定＆初期化
	REG_PORT_DIRSET0 = PORT_PA08;	//pinMode(Lstick_L, OUTPUT);
	REG_PORT_DIRSET0 = PORT_PA09;	//pinMode(Lstick_R, OUTPUT);
	REG_PORT_DIRSET0 = PORT_PA14;	//pinMode(Rstick_L, OUTPUT);
	REG_PORT_DIRSET0 = PORT_PA15;	//pinMode(Rstick_R, OUTPUT);

	REG_PORT_OUTCLR0 = PORT_PA08;	//digitalWrite(Lstick_L, LOW);
	REG_PORT_OUTCLR0 = PORT_PA09;	//digitalWrite(Lstick_R, LOW);
	REG_PORT_OUTCLR0 = PORT_PA14;	//digitalWrite(Rstick_L, LOW);
	REG_PORT_OUTCLR0 = PORT_PA15;	//digitalWrite(Rstick_R, LOW);

//タッチセンサーが接続されているか確認
//Adafruit MPR121 タッチセンサーユニットのアドレス
//デフォルト:0x5A / 3.3Vピン:0x5B / SDAピン:0x5C / SCLピン:0x5D
	Serial.println("Adafruit MPR121 Capacitive Touch sensor test");
	if (!cap_R.begin(0x5A))
	{
		Serial.println("MPR121 L-Side is not found, check wiring?");
		while (1);
	}
	Serial.println("MPR121 L-Side found!");

	if(!cap_C.begin(0x5C))
	{
		Serial.println("MPR121 Center is not found, check wiring?");
		while (1);
	}
	Serial.println("MPR121 Center found!");

	if (!cap_L.begin(0x5B))
	{
		Serial.println("MPR121 R-side is not found, check wiring?");
		while (1);
	}
	Serial.println("MPR121 R-side found!");
}

void loop(){
	FPS_timer1 = millis();
	//タッチセンサーのスキャンと統合
	currtouched_L = cap_L.touched();
	currtouched_C = cap_C.touched();
	currtouched_R = cap_R.touched();

	currtouched_L = currtouched_L<<25;	//両端の入力判定を成立させる為
	currtouched_C = currtouched_C<<13;	//1bit左にシフトさせる
	currtouched_R = currtouched_R<<1;
	currtouched_ALL = currtouched_R | currtouched_C | currtouched_L;

	//タッチセンサーに入力が有る場合
	if(currtouched_ALL)
	{
		for ( int i = 36; i > 0; i--)
		{
			if(currtouched_ALL & bitcheck)
			{
				if(currtouchpoint_ave_1 > 0)
				{
					while(currtouched_ALL & bitcheck)
					{
						currtouchpoint_ave_2 = currtouchpoint_ave_2+(float)i;	//入力位置と数を記録
						currtouchpoint_ave_2_size = currtouchpoint_ave_2_size+1;
						bitcheck = bitcheck >> 1;
						i = i - 1;
					}
				}
				else
				{
					while(currtouched_ALL & bitcheck)
					{
						currtouchpoint_ave_1 = currtouchpoint_ave_1+(float)i;
						currtouchpoint_ave_1_size = currtouchpoint_ave_1_size+1;
						bitcheck = bitcheck >> 1;
						i = i - 1;
					}
				}
				bitcheck = bitcheck << 1;
				i = i + 1;
			}
			bitcheck = bitcheck >> 1;
			if(currtouchpoint_ave_2 > 0)	//入力位置が2か所を超えたら判定終了
			{
				i = 0;
			}
		}
		currtouchpoint_ave_1 = currtouchpoint_ave_1/(float)currtouchpoint_ave_1_size;	//入力位置の中央を求める
		currtouchpoint_ave_2 = currtouchpoint_ave_2/(float)currtouchpoint_ave_2_size;

		if(isnan(currtouchpoint_ave_1))
		{
			currtouchpoint_ave_1 = 0.0;
		}
		if(isnan(currtouchpoint_ave_2))
		{
			currtouchpoint_ave_2 = 0.0;
		}

		if(lasttouchpoint_ave_1 > 0)	//前回入力ある場合
		{
			if((lasttouchpoint_ave_2 > 0) && (currtouchpoint_ave_2 ==0))	//前回２ポイントで入力があって今回１ポイントしか入力が無い場合
			{
				if(abs(lasttouchpoint_ave_1 - currtouchpoint_ave_1) > 5)	//今回入力ポイントが前回の２ポイント目よりだった場合
				{
					lasttouchpoint_ave_1 = lasttouchpoint_ave_2;			//前回の1ポイント目を破棄する
					lasttouchpoint_ave_2 = 0.0;
				}
			}
			else if((lasttouchpoint_ave_2 == 0) && (currtouchpoint_ave_2 > 0))	//前回１ポイントしか入力が無く今回２ポイントで入力があった場合
			{
				if(abs(lasttouchpoint_ave_1 - currtouchpoint_ave_1) > 5)	//前回入力が今回の２ポイント目よりだった場合
				{
					currtouchpoint_ave_1 = currtouchpoint_ave_2;			//今回の１ポイント目を破棄する
					currtouchpoint_ave_2 = 0.0;
				}
			}
			
			if((lasttouchpoint_ave_1 > 0) && (currtouchpoint_ave_1))
			{
				IPpoint1 = lasttouchpoint_ave_1 - currtouchpoint_ave_1;
			}
			if((lasttouchpoint_ave_2 > 0) && (currtouchpoint_ave_2))
			{
				IPpoint2 = lasttouchpoint_ave_2 - currtouchpoint_ave_2;
			}
			
			if(IPpoint1 > 0)
			{
				IPpoint1_vector = 1;
			}
			else if(IPpoint1 < 0)
			{
				IPpoint1_vector = -1;
			}
			
			if(IPpoint2 > 0)
			{
				IPpoint2_vector = 1;
			}
			else if(IPpoint2 < 0)
			{
				IPpoint2_vector = -1;
			}
		}
			//コントローラーに出力
		oldCLR_type = CLR_type;
		if(abs(IPpoint1) > 0)
		{
			if(CLR_Trigger == 0)
			{
				CLR_type = 4;
			}
			if(IPpoint1_vector == 1)
			{
				REG_PORT_OUTCLR0 = PORT_PA08;
				REG_PORT_OUTSET0 = PORT_PA09;
				bitSet(CLR_type,2);
				bitClear(CLR_type,3);
				if(CLR_Trigger < 2)
				{
					CLR_Trigger++;
				}
			}
			else if(IPpoint1_vector == -1)
			{
				REG_PORT_OUTCLR0 = PORT_PA09;
				REG_PORT_OUTSET0 = PORT_PA08;
				bitSet(CLR_type,3);
				bitClear(CLR_type,2);
				if(CLR_Trigger < 2)
				{
					CLR_Trigger++;
				}
			}
		}

		if(abs(IPpoint2) > 0)
		{
			if(IPpoint2_vector == 1)
			{
				REG_PORT_OUTCLR0 = PORT_PA14;
				REG_PORT_OUTSET0 = PORT_PA15;
				bitSet(CLR_type,0);
				bitClear(CLR_type,1);
			}
			else if(IPpoint2_vector == -1)
			{
				REG_PORT_OUTCLR0 = PORT_PA15;
				REG_PORT_OUTSET0 = PORT_PA14;
				bitSet(CLR_type,1);
				bitClear(CLR_type,0);
			}
		}

		if((CLR_Trigger > 1) && ((oldCLR_type == 6) || (oldCLR_type == 9)))
		{
			CLR_type = oldCLR_type;
		}

		//対応するLEDテープを発光させる
		for(uint16_t LED_No = 0; LED_No < strip.numPixels(); LED_No++)
		{
			if(currtouched_ALL & LED_bitcheck)
			{
				strip.setPixelColor(LED_No,127,127,127);
			}
			else
			{
				strip.setPixelColor(LED_No,0,0,0);
			}
			LED_bitcheck = LED_bitcheck>>1;
		}
		strip.show();
		LED_timerLast = millis();
		LED_bitcheck	= 0x1000000000;
		MAX_VAL = 1;
	}
	//タッチセンサーに入力が無い場合
	else
	{
		CLR_Trigger = 0;
		REG_PORT_OUTCLR0 = PORT_PA14;
		REG_PORT_OUTCLR0 = PORT_PA15;
		REG_PORT_OUTCLR0 = PORT_PA08;
		REG_PORT_OUTCLR0 = PORT_PA09;
		if(lasttouchpoint_ave_1)	//前回入力がある場合、LEDテープを消灯させる
		{
			for(uint16_t LED_No = 0; LED_No < strip.numPixels(); LED_No++)
			{
				strip.setPixelColor(LED_No,0,0,0);
			}
			strip.show();
		}
		else
		{
			if(millis() > LED_timerLast+1000)
			{
				switch(CLR_type)
				{
					case 4:
						rainbowCycle_1();
						break;
					case 5:
						rainbowCycle_1();
						break;
					case 6:
						rainbowCycle_5();
						break;
					case 8:
						rainbowCycle_2();
						break;
					case 9:
						rainbowCycle_6();
						break;
					case 10:
						rainbowCycle_2();
						break;
					default:
						rainbowCycle_2();
						break;
				}
			}
		}
	}

//60FPSの判定
	FPS_timer2 = millis();
//	Serial.println(FPS_timer2);
	if( (FPS_timer1+17) > FPS_timer2)
	{
		delay(17-(FPS_timer2 - FPS_timer1));
	}

//入力チェック
//	if(currtouched_ALL > 0)
//	{
//		Serial.print(lasttouchpoint_ave_1);Serial.print(" ");Serial.print(currtouchpoint_ave_1);Serial.print(" ");
//		Serial.print(lasttouchpoint_ave_2);Serial.print(" ");Serial.print(currtouchpoint_ave_2);Serial.print(" ");
//		Serial.print(IPpoint1_vector);Serial.print(" ");Serial.print(IPpoint2_vector);Serial.print(" ");Serial.print(CLR_type);
//		Serial.print(CLR_type);Serial.print(" ");Serial.print(CLR_Trigger);Serial.println("");
//	}

	bitcheck = 0x1000000000;
	IPpoint1 = 0.0;
	IPpoint2 = 0.0;
	IPpoint1_vector = 0;
	IPpoint2_vector = 0;

	//配列のコピーと初期化
	lasttouchpoint_ave_1 = currtouchpoint_ave_1;
	lasttouchpoint_ave_2 = currtouchpoint_ave_2;
	currtouchpoint_ave_1 = 0.0;
	currtouchpoint_ave_2 = 0.0;
	currtouchpoint_ave_1_size = 0;
	currtouchpoint_ave_2_size = 0;
}

void rainbowCycle_1()
{
	for(int i=0; i< 36; i++)
	{
		CLR_BUF=CLR3.charAt(i);
		if(CLR_BUF.equals("R")){
			strip.setPixelColor(i,8*MAX_VAL-1,0,0); //赤
		}
		else if(CLR_BUF.equals("B")){
			strip.setPixelColor(i,0,0,8*MAX_VAL-1); //青
		}
		else if(CLR_BUF.equals("G")){
			strip.setPixelColor(i,0,8*MAX_VAL-1,0); //緑
		}
		else if(CLR_BUF.equals("Y")){
			strip.setPixelColor(i,4*MAX_VAL-1,4*MAX_VAL-1,0); //黄
		}
		else if(CLR_BUF.equals("O")){
			strip.setPixelColor(i,6*MAX_VAL-1,2*MAX_VAL-1,0);//橙
		}
		else {
			strip.setPixelColor(i,4*MAX_VAL-1,0,4*MAX_VAL-1); //ピンク
		}
	}
	strip.show();
	if(MAX_VAL < 32){
		MAX_VAL++;
	}
	CLR_BUF = CLR3.substring(0,35);
	CLR3 = CLR3.charAt(35);
	CLR3.concat(CLR_BUF);
}
void rainbowCycle_2()
{
	for(int i=0; i< 36; i++) {
		CLR_BUF=CLR3.charAt(i);
		if(CLR_BUF.equals("R")){
			strip.setPixelColor(i,8*MAX_VAL-1,0,0); //赤
		}
		else if(CLR_BUF.equals("B")){
			strip.setPixelColor(i,0,0,8*MAX_VAL-1); //青
		}
		else if(CLR_BUF.equals("G")){
			strip.setPixelColor(i,0,8*MAX_VAL-1,0); //緑
		}
		else if(CLR_BUF.equals("Y")){
			strip.setPixelColor(i,4*MAX_VAL-1,4*MAX_VAL-1,0); //黄
		}
		else if(CLR_BUF.equals("O")){
			strip.setPixelColor(i,6*MAX_VAL-1,2*MAX_VAL-1,0);//橙
		}
		else {
			strip.setPixelColor(i,4*MAX_VAL-1,0,4*MAX_VAL-1); //ピンク
		}
	}
	strip.show();
	if(MAX_VAL < 32){
		MAX_VAL++;
	}
	CLR_BUF = CLR3.charAt(0);
	CLR3 = CLR3.substring(1,36);
	CLR3.concat(CLR_BUF);
}
void rainbowCycle_5()
{
	for(int i=0; i< 18; i++) {
		CLR_BUF=CLR1_R.charAt(i);
		if(CLR_BUF.equals("R")){
			strip.setPixelColor(i,8*MAX_VAL-1,0,0); //赤
		}
		else if(CLR_BUF.equals("B")){
			strip.setPixelColor(i,0,0,8*MAX_VAL-1); //青
		}
		else if(CLR_BUF.equals("G")){
			strip.setPixelColor(i,0,8*MAX_VAL-1,0); //緑
		}
		else if(CLR_BUF.equals("Y")){
			strip.setPixelColor(i,4*MAX_VAL-1,4*MAX_VAL-1,0); //黄
		}
		else if(CLR_BUF.equals("O")){
			strip.setPixelColor(i,6*MAX_VAL-1,2*MAX_VAL-1,0);//橙
		}
		else {
			strip.setPixelColor(i,4*MAX_VAL-1,0,4*MAX_VAL-1); //ピンク
		}
	}
	for(int i=18; i< 36; i++) {
		CLR_BUF=CLR1_L.charAt(i-18);
		if(CLR_BUF.equals("R")){
			strip.setPixelColor(i,8*MAX_VAL-1,0,0); //赤
		}
		else if(CLR_BUF.equals("B")){
			strip.setPixelColor(i,0,0,8*MAX_VAL-1); //青
		}
		else if(CLR_BUF.equals("G")){
			strip.setPixelColor(i,0,8*MAX_VAL-1,0); //緑
		}
		else if(CLR_BUF.equals("Y")){
			strip.setPixelColor(i,4*MAX_VAL-1,4*MAX_VAL-1,0); //黄
		}
		else if(CLR_BUF.equals("O")){
			strip.setPixelColor(i,6*MAX_VAL-1,2*MAX_VAL-1,0);//橙
		}
		else {
			strip.setPixelColor(i,4*MAX_VAL-1,0,4*MAX_VAL-1); //ピンク
		}
	}
	strip.show();
	if(MAX_VAL < 32){
		MAX_VAL++;
	}
	CLR_BUF = CLR1_R.substring(0,17);
	CLR1_R = CLR1_R.charAt(17);
	CLR1_R.concat(CLR_BUF);
	CLR_BUF = CLR1_L.charAt(0);
	CLR1_L = CLR1_L.substring(1,18);
	CLR1_L.concat(CLR_BUF);
}

void rainbowCycle_6()
{
	for(int i=0; i< 18; i++) {
		CLR_BUF=CLR1_R.charAt(i);
		if(CLR_BUF.equals("R")){
			strip.setPixelColor(i,8*MAX_VAL-1,0,0); //赤
		}
		else if(CLR_BUF.equals("B")){
			strip.setPixelColor(i,0,0,8*MAX_VAL-1); //青
		}
		else if(CLR_BUF.equals("G")){
			strip.setPixelColor(i,0,8*MAX_VAL-1,0); //緑
		}
		else if(CLR_BUF.equals("Y")){
			strip.setPixelColor(i,4*MAX_VAL-1,4*MAX_VAL-1,0); //黄
		}
		else if(CLR_BUF.equals("O")){
			strip.setPixelColor(i,6*MAX_VAL-1,2*MAX_VAL-1,0);//橙
		}
		else {
			strip.setPixelColor(i,4*MAX_VAL-1,0,4*MAX_VAL-1); //ピンク
		}
	}
	for(int i=18; i< 36; i++) {
		CLR_BUF=CLR1_L.charAt(i-18);
		if(CLR_BUF.equals("R")){
			strip.setPixelColor(i,8*MAX_VAL-1,0,0); //赤
		}
		else if(CLR_BUF.equals("B")){
			strip.setPixelColor(i,0,0,8*MAX_VAL-1); //青
		}
		else if(CLR_BUF.equals("G")){
			strip.setPixelColor(i,0,8*MAX_VAL-1,0); //緑
		}
		else if(CLR_BUF.equals("Y")){
			strip.setPixelColor(i,4*MAX_VAL-1,4*MAX_VAL-1,0); //黄
		}
		else if(CLR_BUF.equals("O")){
			strip.setPixelColor(i,6*MAX_VAL-1,2*MAX_VAL-1,0);//橙
		}
		else {
			strip.setPixelColor(i,4*MAX_VAL-1,0,4*MAX_VAL-1); //ピンク
		}
	}
	strip.show();
	if(MAX_VAL < 32){
		MAX_VAL++;
	}
	CLR_BUF = CLR1_R.charAt(0);
	CLR1_R = CLR1_R.substring(1,18);
	CLR1_R.concat(CLR_BUF);
	CLR_BUF = CLR1_L.substring(0,17);
	CLR1_L = CLR1_L.charAt(17);
	CLR1_L.concat(CLR_BUF);
}
