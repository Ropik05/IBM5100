#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <PS2Keyboard.h>
#define MODEL ILI9488_18
#define CS    A5    
#define CD    A3
#define RST   A4
#define MOSI  51
#define MISO  50
#define SCK   52
#define LED   A0  
const int DataPin = 8;//дата пин
const int IRQpin =  3;//часы клавиатуры
const int Strings [10]{3,32,64,96,128,160,192,224,256,288};
const String ForPrint [10]{};
int i = 0;
int paddlePosition = 0;
int paddlePrewPosition;
int paddleSize = 30;
int ballX = 120;
int ballY = 160;
int ballXSpeed = 1;//скорости мяча (понг)
int ballYSpeed = -1;
int ballSize = 12;//размер в пикселах
int score = 0;
bool StartMenu = false;
LCDWIKI_SPI my_lcd(MODEL,CS,CD,MISO,MOSI,RST,SCK,LED); //model,cs,dc,miso,mosi,reset,sck,led


#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

char *aspect_name[] = {"PORTRAIT", "LANDSCAPE", "PORTRAIT_REV", "LANDSCAPE_REV"};
char *color_name[] = {"BLUE", "GREEN", "RED", "CYAN","YELLOW","WHITE" ,"MAGENTA"};
uint16_t color_mask[] = { 0x07E0,0x001F, 0xF800, 0x07FF,0xFFE0 ,0xFFFF,0xF81F};
void Show_IBM(){ //возврат IBM в главном меню
  my_lcd.Set_Text_colour(GREEN);
   my_lcd.Set_Text_Size(2);
   my_lcd.Print_String("The IBM Personal Computer DOS",2,Strings[0]);//Dos
   my_lcd.Set_Text_Size(3);
   my_lcd.Print_String("A>",2,Strings[1]);
   }
void show_string(uint8_t *str,int16_t x,int16_t y,uint8_t csize,uint16_t fc, uint16_t bc,boolean mode) // метод построкового вывода
{
    my_lcd.Set_Text_Mode(mode);
    my_lcd.Set_Text_Size(csize);
    my_lcd.Set_Text_colour(fc);
    my_lcd.Set_Text_Back_colour(bc);
    my_lcd.Print_String(str,x,y);
}
PS2Keyboard keyboard; 
String SelMode;
String Print;
bool Card = true;
bool Name = true;
char c;
int Mode = 0;
void FirstMode(String str){Затирка строки и написание заданой строки на ее месте
          my_lcd.Set_Text_colour(BLACK);
          my_lcd.Print_String(SelMode,40,Strings[1]);
          my_lcd.Print_String("A>",2,Strings[1]);
          my_lcd.Set_Text_colour(GREEN);
          my_lcd.Print_String(str,2,Strings[1]);
          delay(1000);
           my_lcd.Set_Text_colour(BLACK);
          my_lcd.Print_String(str,2,Strings[1]);
          SelMode=" ";
}
void setup() {
  Serial.begin(9600);
delay(1000);//задержка для инициализации
  keyboard.begin(DataPin, IRQpin);//загрузка клавы
  pinMode(15, OUTPUT); 
  my_lcd.Init_LCD();
  my_lcd.Set_Rotation(3);//положение экрана
  my_lcd.Set_Text_Mode(1);//скорость текста
  my_lcd.Fill_Screen(BLACK);//заливка дисплея
  my_lcd.Set_Text_Back_colour(BLACK);//фон текста
  my_lcd.Set_Text_colour(WHITE);
  my_lcd.Set_Text_Size(27);
  my_lcd.Print_String("IBM",0,60);//лого IBM
  delay(1000);
   my_lcd.Set_Text_Mode(1);
   my_lcd.Set_Text_colour(BLACK);//затирка лого
   my_lcd.Print_String("IBM",0,60);
      my_lcd.Set_Text_Size(3);
      my_lcd.Set_Text_colour(GREEN);
       my_lcd.Print_String("Mem test",2,Strings[0]);
   for(int i =2; i <1024;i = i*2){ //mem ok test
   my_lcd.Set_Text_Size(3);
    my_lcd.Set_Text_colour(GREEN);
    String data = String(i *4);
    my_lcd.Print_String( data + "OK",2,Strings[1]);
    delay (50);
    my_lcd.Set_Text_colour(BLACK);
    my_lcd.Print_String( data + "OK",2,Strings[1]);
   }
   my_lcd.Print_String("Mem test",2,Strings[0]);//mem test end
}


int opPrior(char op)//Свич операций
{
switch (op)
{
case '(': return 0;
case '+': return 1;
case '-': return 1;
case '*': return 2;
case '/': return 2;
case '^': return 3;
case '~': return 4;
default: return -1;
}
}

bool isDigit(char ch)
{
return ch >= '0' && ch <= '9';
} // На всякий случай переопределил, мало ли ардуино тупая

String getOp(char op) // Не получилось парсить символы в строку,так что костыль
{
switch (op)
{
case '(': return "(";
case '+': return "+";
case '-': return "-";
case '*': return "*";
case '/': return "/";
case '^': return "^";
case '~': return "~";
default: return "";
}
}

String getNum(String str, int& pos)
{
String temp = "";
while (isDigit(str[pos]) || str[pos] == '.' || str[pos] == ',')
{
  if(str[pos] == ',')
  {
    temp += '.';
    pos++;
  }
  else
    temp += str[pos++];
}
// Функция вызывается в цикле и после вызова счётчик увеличится.
// Вычитаем единицу, чтобы вызывающая функция наткнулась на знак
pos--;
return temp; // парс строки в число
}

// Хз есть ли стэк на ардуино. Написал (спиздил) свой
class StackStr {
private:
int size, top;
String a[31]; // количество чисел иопераций в нотации - 1
public:
StackStr() { top = 0; }
void push(String var) {
a[++top] = var;
}
String pop() {
return a[top--];
}
String peek() {
return a[top];
}
bool empty() { return top == 0; }
int count() { return top; }
};

String toPostFix(String str)
{
String postFix = "";
StackStr operations;
int s = str.length();
for (int i = 0; i < s; i++)
{
char ch = str[i];
if (isDigit(ch))
postFix += getNum(str, i) + " ";
else if (ch == '(')
operations.push(getOp(ch));
else if (ch == ')')
{
// Если наткнулись на закрывающую скобку - вносим все операции междуними
while (operations.count() > 0 && operations.peek()[0] != '(')
postFix += operations.pop();
// удаляем открывающую скобку
operations.pop();
}
else if (opPrior(ch) != -1)
{
if (ch == '-' && (i == 0 || (i > 1 && opPrior(str[i - 1])!= -1 )))
ch = '~'; // Проверка на минус в начале выражения, ака -100+1
// Заносим более приоритетные операции
while (operations.count() > 0 && opPrior(operations.peek()[0]) >= opPrior(ch))
postFix += operations.pop();
// Заносим текущую операцию
operations.push(getOp(ch));
}
}
while (operations.count() > 0)
postFix += operations.pop();

return postFix;
}

float ExOperator(float a, float b, char op)
{
switch (op)
{
case '+': return a + b;
case '-': return a - b;
case '*': return a * b;
case '/': return a / b;
case '^': return pow(a,b);
default: return 0;
}
}

float stof(String a)
{
  char tmp[30];
  a.toCharArray(tmp, 30);
  float x = atof(tmp);
  Serial.print("input = " + a + " char[] = " + tmp + " parsing = " + x + "\n");
  return x;
}

float CalcFunction(String func)
{
StackStr numbers;
String pf = toPostFix(func);
for (int i = 0; i < pf.length(); i++)
{
char ch = pf[i];
if (isDigit(ch))
numbers.push(getNum(pf, i));
else if (opPrior(ch) != -1)
{
if (ch == '~')
{
float last = 0;
// если стек пуст - останется 0 иначе - возьмётся значение из стека
if (numbers.count() > 0)
last = stof(numbers.pop());
numbers.push(String(ExOperator(0, last, '-')));
continue;
}
float first = 0, second = 0;
if (numbers.count() > 0)
second = stof(numbers.pop());
if (numbers.count() > 0)
first = stof(numbers.pop());
numbers.push(String(ExOperator(first, second, ch)));
}
}
return stof(numbers.pop());
}

void loop() { //главный цикл
  my_lcd.Set_Text_Size(3);//размер шрифта постоянно меняеться на 3 ради спокойствия
  if(Mode ==0){ // 1 mode меню
    my_lcd.Set_Text_Mode(3);
       Show_IBM(); //выводим стрку IBM
    SelMode.replace(" ","");
    if (keyboard.available())//если на дата пине есть сигнал то
      {        
       c = keyboard.read();//читаем дата линию
       if(c == PS2_ENTER){ // бинд команд и приложений
        if(SelMode == "help"){
          FirstMode("Commands : help dir");
          Show_IBM();
          }
          else if(SelMode == "dir"){
            FirstMode(" Calc.exe , Pong.exe, ???");
            Show_IBM();
            }
            else if(SelMode == "Calc.exe"){
              FirstMode("Start Calc.exe");
              FirstMode("To exit press TAB");
              Mode =1;
              }
              else if(SelMode == "Pong.exe"){
              FirstMode("Start Pong.exe");
              FirstMode("Left use 4 Right 6");
              FirstMode("To exit press TAB");
               my_lcd.Set_Text_colour(BLACK);
               my_lcd.Set_Text_Size(2);
               my_lcd.Print_String("The IBM Personal Computer DOS",2,Strings[0]);//Dos
               my_lcd.Set_Text_colour(GREEN);
              Mode =2;
              }
              else if(SelMode == "KGU"){
                FirstMode("Made by students:");
                FirstMode(" Jeb129,Ropik");
                FirstMode("Humanum,Favariy");
                FirstMode("Ezhik7454");
                Show_IBM();
              }
          else{
            FirstMode("Not a command Write help");
           Show_IBM();
          }
        }
        else if (c == PS2_DELETE) { //затирание строки
          my_lcd.Set_Text_colour(BLACK);//текст черный
          my_lcd.Print_String(Print,2,Strings[i]);//пишем старую строку поверх новой
          Print.remove(Print.length()-1);//-1 символ с конца
          my_lcd.Set_Text_colour(GREEN);//текст зеленым
          my_lcd.Print_String(Print,2,Strings[i]);//пишем новую строку
       } 
       else{//если не бинд то буква
        digitalWrite(15, HIGH); 
        SelMode = SelMode +c;
        my_lcd.Print_String(SelMode,40,Strings[1]);
        digitalWrite(15, LOW);}
      }
    }
    else if(Mode ==1){ // 2Mode калькулятор
       digitalWrite(15, HIGH);//изменение подсветки диода (работет программа)
      if(i == 10){//если все строки экрана заполнены то заливаем черным и пишем сверху строку заново
        my_lcd.Fill_Screen(BLACK);
         i = 0;
         my_lcd.Print_String(Print,2,Strings[i]);
      } 
      if(Name == true){
       my_lcd.Set_Text_colour(BLACK);
       my_lcd.Set_Text_Size(2);
       my_lcd.Print_String("The IBM Personal Computer DOS",2,Strings[0]);
       Name = false;
   }
        my_lcd.Set_Text_colour(GREEN);
         my_lcd.Set_Text_Size(3);
    if (keyboard.available()) {
       c = keyboard.read();
       if (c == PS2_ENTER || c== '=') {
           my_lcd.Set_Text_colour(BLACK);
            my_lcd.Print_String(Print,2,Strings[i]);
            Print = String(CalcFunction(Print));
              //my_lcd.Print_String("Memory exeption",2,Strings[i]);
            my_lcd.Set_Text_colour(GREEN);
            my_lcd.Print_String(Print,2,Strings[i]);
            ForPrint[i] = Print;
            i++;
            my_lcd.Print_String(Print,2,Strings[i]);
    } 
    else if (c == PS2_TAB) {
      digitalWrite(15, LOW);
      Mode = 0;
      my_lcd.Fill_Screen(BLACK);
      i = 0;
      Name = true;
    }
    else if(c=='+' || c=='-' || c=='*' || c=='/'){ //знаки
      Print = Print + c;
       my_lcd.Print_String(Print,2,Strings[i]);
    }
    else if (c == PS2_DELETE) { //затирание строки
       my_lcd.Set_Text_colour(BLACK);//текст черный
      my_lcd.Print_String(Print,2,Strings[i]);//пишем старую строку поверх новой
      Print.remove(Print.length()-1);//-1 символ с конца
      my_lcd.Set_Text_colour(GREEN);//текст зеленым
      my_lcd.Print_String(Print,2,Strings[i]);//пишем новую строку
    } 
    else {
      //если не бинд то буква добавляем к строке и выводим
      Print = Print + c;
      my_lcd.Print_String(Print,2,Strings[i]);
          }
      }
}
    else if(Mode == 2) // 3mode Понг
    {  digitalWrite(15, HIGH);
            if (keyboard.available()) // проверка отклика от клавиатуры
      {        
       c = keyboard.read();}
       if(StartMenu == false){ // Меню перед игрой
      my_lcd.Set_Text_Size(2);
      my_lcd.Set_Text_colour(GREEN);
      my_lcd.Print_String(" TO play press 5",30,Strings[3]);}
       if (c == '5'){ // если нажали 5 то игра начинаеться
        StartMenu = true;
        my_lcd.Set_Text_colour(BLACK);
        my_lcd.Print_String(" TO play press 5",30,Strings[3]);
       }
      if(StartMenu == true){ // игра
       my_lcd.Fill_Rect(ballX-ballXSpeed, ballY-ballYSpeed,12,12, BLACK);
  my_lcd.Fill_Rect(paddlePosition, 300, paddleSize, 5,WHITE);
  my_lcd.Fill_Rect(ballX, ballY,12,12, WHITE);

  if (ballX + ballXSpeed > 480 || ballX + ballXSpeed < 0) { // если достигли верха экрана то скорость меняеться на обратную
    ballXSpeed = -ballXSpeed;
  }

  if (ballY + ballYSpeed > 320 || ballY + ballYSpeed < 0) { // если достигли края экрана то тоже меняем скорость
    ballYSpeed = -ballYSpeed;
  }

  ballX += ballXSpeed;
  ballY += ballYSpeed;

  if (ballY + ballSize/2 >= 300&& ballY + ballSize/2 <= 310 &&
      ballX + ballSize/2 >= paddlePosition && ballX + ballSize/2 <= paddlePosition + paddleSize) { // есил мяч коснулся панели то меняем скорость по высоте на обратную счет++
    score++;
    ballYSpeed = -ballYSpeed;
  }

  if (ballY + ballSize/2 >= 310) {//если мяч упал ниже уровня панели то GAME OVER
    my_lcd.Fill_Screen(WHITE);
    my_lcd.Set_Text_colour(BLACK);
   // my_lcd.setCursor(60, 120);
    my_lcd.Print_String("Game Over",60,120);
   // my_lcd.setCursor(80, 160);
    my_lcd.Print_String("Score: ",80,160);
    my_lcd.Print_String(String(score),240,160);
    delay(2000);
    my_lcd.Fill_Screen(BLACK);
    ballX = 5;
    ballY = 5;
    score = 0;
    StartMenu = false;
  }
  if (c=='4') {//движение влево 
    paddlePrewPosition = paddlePosition;
      my_lcd.Fill_Rect(paddlePrewPosition, 300, paddleSize, 5,BLACK);
    paddlePosition -= 5;
  }

  if (c=='6') {//движение вправо
    paddlePrewPosition = paddlePosition;
      my_lcd.Fill_Rect(paddlePrewPosition, 300, paddleSize, 5,BLACK);
    paddlePosition += 5;
  }
  delay(10);
      
      }
     if (c == PS2_TAB) { 
     digitalWrite(15, LOW);
          my_lcd.Fill_Screen(BLACK);
          i = 0;
          StartMenu = false;
          Name = true;
          Mode = 0;
    }
     c =' ';
  }
}
