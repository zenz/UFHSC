import processing.serial.*;

PFont f;
PFont F;
Serial myPort;        // The serial port
int xPos = 40;         // horizontal position of the graph

void setup () {
  // set the window size:  and Font size
  f = createFont("Arial", 12, true);
  F = createFont("Arial", 24, true);
  size(1024, 600);

  // List all the available serial ports
  //println(Serial.list());
  myPort = new Serial(this, "/dev/tty.SLAB_USBtoUART", 9600);
  myPort.bufferUntil('\n');
  // set inital background:
  background(100);
}
void draw () 
{
  String inputString, inString, outString, pumpString, boilerString;
  // everything happens in the serialEvent()
  if (myPort.available()>0) {
    // get the ASCII string:
    inputString = myPort.readStringUntil('\n');
    inString ="";
    if (inputString != null) {
      // trim off any whitespace:
      String[] temp = splitTokens(inputString, ",");
      print("State: ");
      print(inputString);
      if (temp[0] !=null) {
        outString = trim(temp[0]);
      } else {
        outString = "0";
      }
      if (temp[1] !=null) {
        inString = trim(temp[1]);
      } else {
        inString = "0";
      }
      if (temp[2] !=null) {
        pumpString = trim(temp[2]);
      } else {
        pumpString = "0";
      }
      if (temp[3] !=null) {
        boilerString = trim(temp[3]);
      } else {
        boilerString = "0";
      }

      // convert to an int and map to the screen height:
      float inByte = float(inString+(char)9); 
      inByte = map(inByte, 0, 117, 0, height);

      float outByte = float(outString+(char)9);
      outByte = map(outByte, 0, 117, 0, height);

      //println(inByte);

      stroke(175);                       // temperature line
      fill(150);
      line(40, height-40, 40, 0);

      stroke(175);                          // Time line
      line(40, height-40, width, height-40);

      stroke(100, 100, 255);                          // 30 degree line
      line(40, height-194, width, height-194);

      stroke(100, 100, 255);                          // 60 degree line
      line(40, height-344, width, height-344);

      textFont(F);       
      fill(255);

      textAlign(RIGHT);
      text("Temperature Ploting Programme", width-100, 40); 

      textAlign(RIGHT);
      text("By SimonHu@Airfit.CN", width-100, 70); 

      textAlign(RIGHT);
      text("Temp", 70, 40);                         

      textAlign(RIGHT);
      text("Time --->", width-50, 580);    

      fill(0);
      // int j;
      stroke(255);   
      for (int j=width-100; j>width-510; j--) //260
      {
        line(j, 80, j, 107);
      }
      stroke(0, 0, 0);
      textAlign(RIGHT);
      text("S:"+outString +", R:" + inString, width-110, 102); 
      // draw pumpState
      text("PUMP:"+pumpString, width-410, 102);
      // draw boilerState
      text("BOILER:"+boilerString, width-295, 102);


      fill(240);
      textFont(f); 

      textAlign(RIGHT);
      text("(In Degree)", 140, 40); 

      textAlign(RIGHT);                 // 100 degree
      text("100 -", 40, 60);

      textAlign(RIGHT);                // 90 degree
      text("90 -", 40, 110);

      textAlign(RIGHT);                // 80 degree
      text("80 -", 40, 160);

      textAlign(RIGHT);                 // 70 degree
      text("70 -", 40, 210);

      textAlign(RIGHT);                // 60 degree
      text("60 -", 40, 260);

      textAlign(RIGHT);               // 50 degree
      text("50 -", 40, 310);

      textAlign(RIGHT);                 // 40 degree
      text("40 -", 40, 360);

      textAlign(RIGHT);
      text("30 -", 40, 410);

      textAlign(RIGHT);
      text("20 -", 40, 460);

      textAlign(RIGHT);
      text("10 -", 40, 510);

      textAlign(RIGHT);
      text("0 -", 40, 560);

      /*---- scale between 30 degree to 40 degree------*/

      textAlign(RIGHT);
      text("   -", 40, 370);

      textAlign(RIGHT);
      text("   -", 40, 380);

      textAlign(RIGHT);
      text("   -", 40, 390);

      textAlign(RIGHT);
      text("   -", 40, 400);

      // draw the line:
      int shift=40;            // set trace origin
      stroke(0, 255, 0);              // trace colour
      for (int i=0; i<2; i++)
      {
        //line(xPos, height-inByte-1, xPos, height - inByte);
        line(xPos, height-inByte-(shift+3), xPos, height-inByte-shift);
        xPos++;
      }

      // draw out temp
      stroke(255, 0, 0);              // trace colour
      for (int i=0; i<2; i++)
      {
        //line(xPos, height-inByte-1, xPos, height - inByte);
        line(xPos, height-outByte-(shift+3), xPos, height-outByte-shift);
        xPos++;
      }
      if (xPos >= width)         //  go back to begining
      {
        xPos = 40;
        background(100);
      }
    }
  }
}