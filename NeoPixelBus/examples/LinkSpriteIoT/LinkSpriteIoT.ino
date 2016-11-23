#include <LinkSpriteIO.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

String deviceID = "xxxxxxxxxx";
String apikey = "xxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";

LinkSpriteIO linksprite(deviceID,apikey);

static uint16_t PixelCount = 30; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const uint8_t AnimationChannels = 1;

#define colorSaturation 128

NeoPixelBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod> strip(PixelCount, PixelPin);
NeoPixelAnimator animations(AnimationChannels);

uint16_t effectState = 0;

static uint32_t start_time = 0;
static uint32_t end_time = 0;

struct MyAnimationState
{
  RgbColor StartingColor;
  RgbColor EndingColor;
};

MyAnimationState animationState[AnimationChannels];

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);

void SetRandomSeed()
{
    uint32_t seed;
    seed = analogRead(0);
    delay(1);
    for (int shifts = 3; shifts < 31; shifts += 3)
    {
        seed ^= analogRead(0) << shifts;
        delay(1);
    }
    randomSeed(seed);
}


void LED_Group(int first, int last, RgbColor color)
{
  int i;

  for(i=first-1; i<last; i++)
  {
    strip.SetPixelColor(i, color);
  }
  strip.Show();
}

void LED_ChangeColor(int speed, RgbColor color, int num)
{
  RgbColor tmp = strip.GetPixelColor(num);
  
  while(tmp.R != color.R || tmp.G != color.G || tmp.B != color.B)
  {
    if(tmp.R > color.R)
    {
      tmp.R--;
    }
    if(tmp.R < color.R)
    {
      tmp.R++;
    }
    if(tmp.G > color.G)
    {
      tmp.G--;
    }
    if(tmp.G < color.G)
    {
      tmp.G++;
    }
    if(tmp.B > color.B)
    {
      tmp.B--;
    }
    if(tmp.B < color.B)
    {
      tmp.B++;
    }
    strip.SetPixelColor(num, tmp);
    strip.Show();
    delay(speed);
  }
}

void LED_Flow(int ms)
{
  for (int i = 0; i < PixelCount; i++)
  {
    int s = random(255);
    if (s % 4 == 0)
      strip.SetPixelColor(i, red);
    if (s % 4 == 1)
      strip.SetPixelColor(i, green);
    if (s % 4 == 2)
      strip.SetPixelColor(i, blue);
    if (s % 4 == 3)
      strip.SetPixelColor(i, white);
    strip.Show();
    delay(ms);
  }
  for (int a = 0; a < PixelCount; a ++)
  {
    strip.SetPixelColor(a, black);
    strip.Show();
    delay(10);
  }
}

void BlendAnimUpdate(const AnimationParam& param)
{
  RgbColor updatedColor = RgbColor::LinearBlend(
                            animationState[param.index].StartingColor,
                            animationState[param.index].EndingColor,
                            param.progress);
  for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
  {
    strip.SetPixelColor(pixel, updatedColor);
  }
}

void FadeInFadeOutRinseRepeat(float luminance)
{
  if (effectState == 0)
  {
    RgbColor target = HslColor(random(360) / 360.0f, 1.0f, luminance);
    uint16_t time = random(800, 2000);
    animationState[0].StartingColor = strip.GetPixelColor(0);
    animationState[0].EndingColor = target;
    animations.StartAnimation(0, time, BlendAnimUpdate);
  }
  else if (effectState == 1)
  {
    uint16_t time = random(600, 700);
    animationState[0].StartingColor = strip.GetPixelColor(0);
    animationState[0].EndingColor = RgbColor(0);
    animations.StartAnimation(0, time, BlendAnimUpdate);
  }
  effectState = (effectState + 1) % 2;
}

void LED_Breath()
{
  if (animations.IsAnimating())
  {
    animations.UpdateAnimations();
    strip.Show();
  }
  else
  {
    FadeInFadeOutRinseRepeat(0.2f); // 0.0 = black, 0.25 is normal, 0.5 is bright
  }
}


void LED_Blink(int ms)
{
  for (int i = 0; i < PixelCount; i++)
  {
    int s = random(255);
    if (s % 4 == 0)
      strip.SetPixelColor(i, red);
    if (s % 4 == 1)
      strip.SetPixelColor(i, green);
    if (s % 4 == 2)
      strip.SetPixelColor(i, blue);
    if (s % 4 == 3)
      strip.SetPixelColor(i, white);
    strip.Show();
  }
  delay(ms);
  for (int a = 0; a < PixelCount; a ++)
  {
    strip.SetPixelColor(a, black);
    strip.Show();
  }
}

void setup()
{
    Serial.begin(115200);
    linksprite.begin();
    strip.Begin();
    strip.Show();
    start_time = millis();
}


void loop()
{
    static query_val val; 
    end_time = millis();
    if((end_time - start_time) > 8000) 
    {
        start_time = end_time;
        val = linksprite.query("count","mode","time","color","num");
        Serial.println("The get is :");
        Serial.println(val.d1);
        Serial.println(val.d2);
        Serial.println(val.d3);
        Serial.println(val.d4);
        Serial.println(val.d5);
        LED_Group(0,PixelCount,black);
    }
    if(val.d1 != 0)
        PixelCount = val.d1;
    if(val.d2 == 1)
    {
        Serial.println("This is LED_Flow mode!");
        if(val.d3 != 0)
            LED_Flow(val.d3);
        else LED_Flow(100);
    }
    else if(val.d2 == 2)
    {
        Serial.println("This is LED_Breath mode!");
        LED_Breath(); 
    }
    else if(val.d2 == 3)
    {
        Serial.println("This is LED_Blink mode!");
        if(val.d3 != 0)
            LED_Blink(val.d3);
        else  LED_Blink(300); 
    }
    else if(val.d2 == 4)
    {
       Serial.println("This is LED_Auto mode!");
       int st,ed;
       st = val.d5/100;
       ed = val.d5%100;
       Serial.println(st);
       Serial.println(ed);
       if(val.d4 == 0)
          LED_Group(st,ed,red);
       if(val.d4 == 1)
          LED_Group(st,ed,blue);
       if(val.d4 == 2)
          LED_Group(st,ed,green);
       if(val.d4 == 3)
          LED_Group(st,ed,white);
    }
}



