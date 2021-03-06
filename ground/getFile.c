// Transfers a binary file from arduino over I2C

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "../downlink/gpio.h"
#include <time.h>

#define ADDR 0x0F
#define BUSYPIN 4
//#define RSTPIN 22
#define CHUNKSIZE 16
#define BUFSIZE 1024

// commands
#define READ 0
#define PEEK 1
#define CHECKEOF 2

// functions
long timediff(clock_t t1, clock_t t2);

int main(int argc, char **argv){
  //uint8_t buf[CHUNK_SIZE];
  char *outFilename = NULL;
  char i2cFilename[20];
  int I2C,outFile;
  int index;
  int c;
  int end = 0;
  clock_t t1,t2;

  opterr = 0;
  // parse arguments
  while ((c = getopt (argc, argv, "o:")) != -1)
    switch (c)
      {
      case 'o':
        outFilename = optarg;
        break;
      case '?':
        if (optopt == 'o')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }
  // check if input file was specified
  if(outFilename == NULL){
    printf("Output file required\n");
    return 0;
  }
  // print file name for debugging
  printf ("outFilename = %s\n",outFilename);

  // open input file
  outFile = open(outFilename,O_WRONLY|O_CREAT|O_TRUNC);
  if (outFile<0){
    printf("Failed to open output file\n");
    return 0;
  }
  // open i2c interface
  snprintf(i2cFilename,19,"/dev/i2c-%d",1);
  I2C = open(i2cFilename, O_RDWR);
  if (I2C<0){
    printf("Failed to initialize I2C Bus\n");
    // close input file
    close(outFile);
    return 0;
  }

  // get gpio
  gpioExport(BUSYPIN);
  //gpioExport(RSTPIN);
  // set direction
  gpioDirection(BUSYPIN,GPIO_IN);
  //gpioDirection(RSTPIN,GPIO_OUT);
  //gpioSet(RSTPIN,1);
  printf("Got GPIO\n");

  // set device address
  if (ioctl(I2C, I2C_SLAVE, ADDR) < 0){
    // could not set device as slave
    printf("Could not find device\n");
    return 0;
  }

  uint8_t command[3];
  uint8_t data[2];
  uint8_t dataBuf[BUFSIZE];
  uint16_t nBytes = 0;
  uint8_t newFile = 0;
  uint16_t rxBytes;
  long diff;
  // iterate until EOF
  while(!newFile){
   // wait until the data available flag is high
   while(!gpioGet(BUSYPIN)){
     t1 = clock();
     usleep(10);
     t1 = clock();
     diff = timediff(t1,t2);
     if(diff > 100){
       printf("WARNING, EXCESSIVE SERVICE TIME!");
       return -1;
     }
     /*
     // check if EOF
     command[0] = CHECKEOF;
     write(I2C,command,1);
     read(I2C,data,1);
     if(data[0]==1){
       // raise EOF flag
       newFile = 1;
       printf("New File!\n");
       break;
     }
     */
   }
   //if(newFile){
   //  break;
   //}
   // check available data
   command[0]= PEEK;
   if(write(I2C,command,1)!=1){
     printf("Write to Arduino Failed.\n");
     return -1;
   }
   // do nothing for a bit...
   volatile int i = 0;
   for(i=0;i++;i<100){
   }
   read(I2C,data,2);
   nBytes = (data[0] << 8) | data[1];
   // if there is data
   if(nBytes>0){
     //printf("%d bytes available\n",nBytes);
     // determine how many bytes to read
     if(nBytes<CHUNKSIZE){
       rxBytes=nBytes;
     }
     else{
       rxBytes = CHUNKSIZE;
     }
     // write read data command
     command[0] = READ;
     command[1] = rxBytes >> 8;
     command[2] = rxBytes & 0xFF;
     write(I2C,command,3);
     // read data
     nBytes = read(I2C,dataBuf,rxBytes);
     //int i;
     //for(i=0;i<nBytes;i++){
     //  printf("%X,",dataBuf[i]);
     //}
     //printf("\n");
     // write data to file 
     write(outFile,dataBuf,nBytes);
   }
   // check if EOF
   else{
     command[0] = CHECKEOF;
     write(I2C,command,1);
     read(I2C,data,1);
     if(data[0]==1){
       // raise EOF flag
       newFile = 1;
       printf("New File!\n");
     }
   }
   // prevent loop from becoming overly fast
   //usleep(15000);
  }

  // unregister gpios
  gpioUnexport(BUSYPIN);
  //gpioUnexport(RSTPIN);
  // close i2c interface
  close(I2C);
  // close input file
  close(outFile);
}

long timediff(clock_t t1, clock_t t2) {
    long elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}
