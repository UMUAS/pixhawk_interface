#include <iostream> 
#include <unistd.h> 
#include <fcntl.h>
#include <thread>
#include <termios.h> 
#include <string_view> 
#include <cstring>
#include <mavlink.h> 

mavlink_status_t status;
mavlink_message_t msg;
int chan = MAVLINK_COMM_0;


void configure_termios(struct termios * termios_config, int device_file_descriptor,speed_t input_baud,speed_t output_baud){
        if(tcgetattr(device_file_descriptor, termios_config)< 0){
         std::cout<< "could not get previous configuration for device\n";
         exit(0); 
        }

        cfmakeraw(termios_config); 
        if( cfsetispeed(termios_config, input_baud)< 0 ||  cfsetospeed(termios_config, output_baud)  < 0){
          std::cout<< "failed to set input/output baud rate\n"; 
          exit(0);  
        }
        tcsetattr(device_file_descriptor,TCSANOW, termios_config); 
}

int open_device_file(char * device_path){
        int fd = open(device_path, O_RDWR|O_NDELAY| O_NOCTTY); 
        if(fd < 0){
                std::cout<<"could not open device\n";
               exit(0); 
        }
        return fd; 
}


void read_device(int fd) {
    std::cout<<"[LOG] READ FUNCTION CALLED \n"; 
    std::setlocale(LC_ALL, "");
    uint8_t buffer[1024];
    while (true) {
        read(fd, buffer, sizeof(buffer));
        for (size_t i{}; i < sizeof(buffer); i++){
                if(mavlink_parse_char(chan,buffer[i],&msg,&status)){
                        printf("Received message with ID %d, sequence: %d from component %d of system %d\n", msg.msgid, msg.seq, msg.compid, msg.sysid);
                }
        }
         usleep(1000000);
    }
}

void write_device(int fd) {
    std::string message = "Hello World Test";
    while (true) {
        ssize_t bytes_written = write(fd, message.c_str(), message.size());
        if (bytes_written < 0) {
            std::cout << "Failed to write to device\n";
        }
        usleep(1000000); // Sleep 1 second between writes
    }
}

int main(int argc, char** argv){
        char device_path[] = "/dev/ttyTHS1";
        int fd = open_device_file(device_path); 
        struct termios termios_config; 
        puts("[LOG] SETTING UP RADIO CONFIG...");
        configure_termios(&termios_config,fd, B57600, B57600);
        puts("[LOG] RUNNING PROGRAM...");
        std::thread read_thread(read_device, fd); 
        //std::thread write_thread(write_device, fd); 
        read_thread.join(); 
        //write_thread.join(); 
        close(fd);
}
