import processing.serial.*;

void setup()
{
  //Sets our frame, 1000 x 1000 px in this case
  size(1000,1000);
  
  //Sets background to black
  background(0);
}

void draw()
{
  //Reads data from file
  String[] data = loadStrings("data.txt");
  
  //Reads every line of the data file
  for(int i = 0; i < data.length; i++){
    
    //Breaks a string into pieces, uses empty space as indicator from where to cut the string
    String[] subData = split(data[i], ' ');
    
    //Removes whitespace characters from the beginning and end of a string
    trim(subData);
    
    /*Conditional, because the program reads the file too fast. Avoids errors related to the array being out
    of bounds. Currently set to 17, because we know that we are receiving 16 data elements(17th being NULL).*/
    if(subData.length == 17){
      
      for(int j = 0; j < 16; j += 2){
        
        //The coordinates for the robot, mark them red
        if(j == 8){
          int x = int(subData[j]);
          int y = int(subData[j + 1]);
          fill(255,0,0);
          ellipse(x,y,7,7);
        }
        //The coordinates for the walls, mark them green
        else{
          //Typecasting the strings into integers
          int x = int(subData[j]);
          int y = int(subData[j + 1]);
        
          //fill functions sets the color of the shape we want to draw.(R,G,B)
          fill(0,255,0);
        
          //ellipse creates an ellipse on the screen, (x-coordinate, y-coordinate, width, length)
          ellipse(x,y,7,7);
        }
      }
    }
  }
}
