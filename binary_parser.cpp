#include <iostream>
#include <memory>
//this utility functions converts a number from a binary buffer, 
//it was inspired by php base_convert

inline uint32_t unpack(unsigned char *buf, int start, int length)
{   
    uint32_t result = 0; 

    int startByte = (int) start / 8;
    int stopByte = (int) (start+length) / 8;
    int lengthBytes = stopByte - startByte + 1;

    int startByteBit = (1 << (8-( start  % 8 ))) - 1;
    int stopByteBit = (start+length) % 8;

    auto cp = buf + startByte;

    for (int i=0; i < lengthBytes; i++) {

         auto mask  =  i==0 ? startByteBit : 0xFF;
         auto shift = (i+1 == lengthBytes) ? stopByteBit : 8;

         result <<= shift ;
         result += ((int) *(cp+i) & mask) >> (8 - shift); 
    }
    return result;
}
void convert_hex_bin(int msg_length, const char * msgp, unsigned char* binmsgp)
{
    for (int i = 0; i < msg_length; i++) {
      auto &ptr = *(binmsgp+i/2);
      if (i%2) ptr <<=4;
      auto ch = toupper(*(msgp + i));
      if (ch >= '0' && ch <= '9') ptr += (ch - '0');
      else if (ch >= 'A' && ch <= 'F') ptr += (ch - 'A' + 10);
    }
}
void extractmsg(unsigned char* binmsgp)
{
    std::cout << "msgType = "      << (int)unpack(binmsgp,0,3) << '\n';
    std::cout << "flag1 = "        << (int)unpack(binmsgp,5,1) << '\n';
    std::cout << "flag2 = "        << (int)unpack(binmsgp,6,1) << '\n';
    std::cout << "flag3 = "        << (int)unpack(binmsgp,7,1) << '\n';
    std::cout << "temp1 = "        << unpack(binmsgp,8,12) * 2 << '\n';
    std::cout << "temp2 = "        << unpack(binmsgp,20,12) * 2 << '\n';
    std::cout << "temp3 = "        << unpack(binmsgp,32,12) * 2 << '\n';
    std::cout << "temp4 = "        << unpack(binmsgp,44,12) * 2 << '\n';
    std::cout << "temp5 = "        << unpack(binmsgp,56,12) * 2 << '\n';
    std::cout << "temp6 = "        << unpack(binmsgp,68,12) * 2 << '\n';
    std::cout << "duration = "     << unpack(binmsgp,80,10) * 100 << '\n';
    std::cout << "voltage = "      << unpack(binmsgp,90,6) * 5 << '\n';
}

int main()
{
    std::string msg {"003de3cf3d145843b43bbf9f" };
    auto buff = std::make_unique<unsigned char[]>(msg.size()/2);
    auto binmsgp = buff.get();
    convert_hex_bin(msg.size(),msg.data(),binmsgp);
    extractmsg(binmsgp);
    std::cout << "finished\n";
}
